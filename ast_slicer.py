"""
ast_slicer.py
-------------
A REAL intraprocedural backward static program slicer for C/C++ functions,
built on tree-sitter (tree-sitter-c). Replaces the old line-level heuristic in
slicer.py for the `static` (and, by union, `hybrid`) slicing modes.

The public entry point is `semantic_slice_indices(code)`, which returns the set
of 0-based source line indices (into `code.splitlines()`) that are retained by a
standard backward static slice taken from SySeVR-style slicing criteria
(API/library calls, array accesses, pointer usages, pointer/arithmetic
expressions).

Design goals
============
* Leakage-free: NEVER looks at func_after / the patch. The slice is computed
  purely from the (vulnerable) function under analysis.
* Robust: parse failures, missing function bodies, ERROR nodes, or an empty
  slice all fall back to the whole function (set(range(n_lines))). The function
  must NEVER raise.
* Intraprocedural, single function: dependences are tracked only within the
  outermost function body found in `code`.

The slice is a PROPER SUBSET of the function for typical inputs (so
region_size_ratio < 1.0), while being a conservative super-set of the
fix-relevant statements far more often than the old line heuristic (so
fix_coverage rises).
"""

from __future__ import annotations

from typing import Dict, List, Optional, Set, Tuple

# --- tree-sitter setup (best-effort; never fatal at import time) --------------
_PARSER = None
_TS_OK = False
try:  # pragma: no cover - exercised only where tree-sitter is installed
    import tree_sitter_c as _tsc
    from tree_sitter import Language as _Language, Parser as _Parser

    _LANG = _Language(_tsc.language())
    _PARSER = _Parser(_LANG)
    _TS_OK = True
except Exception:  # pragma: no cover - missing/broken tree-sitter
    _PARSER = None
    _TS_OK = False


# Statement-like node kinds that we treat as schedulable "statements".
_STATEMENT_TYPES = {
    "declaration",
    "expression_statement",
    "return_statement",
    "break_statement",
    "continue_statement",
    "goto_statement",
    "labeled_statement",
    "do_statement",
    # The bodies / headers of these are handled specially, but a bare one that
    # slips through is still useful to retain as a unit.
}

# Library / memory / string functions that strongly indicate a slicing
# criterion (SySeVR "library/API function call" category). ANY call still
# counts as a criterion; these just make the intent explicit / auditable.
_SENSITIVE_CALLS = {
    "memcpy", "memmove", "memset", "strcpy", "strncpy", "strcat", "strncat",
    "sprintf", "snprintf", "vsprintf", "vsnprintf", "gets", "fgets",
    "scanf", "sscanf", "fscanf", "malloc", "calloc", "realloc", "free",
    "alloca", "system", "popen", "exec", "execl", "execlp", "execv",
    "read", "recv", "recvfrom", "fread", "memcmp", "strlen", "strdup",
}

# Predicate-bearing control constructs.
_CONTROL_TYPES = {
    "if_statement",
    "for_statement",
    "while_statement",
    "do_statement",
    "switch_statement",
}


def _node_text(node, code_bytes: bytes) -> str:
    try:
        return code_bytes[node.start_byte:node.end_byte].decode("utf-8", "replace")
    except Exception:
        return ""


def _find_function_body(root):
    """Return the outermost function_definition's body compound_statement, or
    None if no function body can be located."""
    # BFS for the first function_definition; then take its compound_statement.
    stack = [root]
    while stack:
        node = stack.pop()
        if node.type == "function_definition":
            for ch in node.children:
                if ch.type == "compound_statement":
                    return ch, node
        # Continue scanning (handles leading macros / ERROR wrappers).
        stack.extend(reversed(node.children))
    # Fallback: a top-level compound_statement (some snippets are bodies only).
    stack = [root]
    while stack:
        node = stack.pop()
        if node.type == "compound_statement":
            return node, None
        stack.extend(reversed(node.children))
    return None, None


def _collect_identifiers(node, code_bytes: bytes) -> Set[str]:
    """All identifier tokens read anywhere under `node`."""
    out: Set[str] = set()
    stack = [node]
    while stack:
        n = stack.pop()
        if n.type == "identifier":
            txt = _node_text(n, code_bytes)
            if txt:
                out.add(txt)
        else:
            stack.extend(n.children)
    return out


def _root_identifier(node, code_bytes: bytes) -> Optional[str]:
    """Approximate the base variable of an lvalue expression by its leftmost /
    root identifier (handles a.b, a->b, a[i], *a, (a))."""
    n = node
    guard = 0
    while n is not None and guard < 64:
        guard += 1
        t = n.type
        if t == "identifier":
            return _node_text(n, code_bytes)
        if t in ("field_expression",):
            # base is the 'argument' child (the part before . / ->)
            base = n.child_by_field_name("argument")
            n = base if base is not None else (n.children[0] if n.children else None)
            continue
        if t in ("subscript_expression",):
            base = n.child_by_field_name("argument")
            n = base if base is not None else (n.children[0] if n.children else None)
            continue
        if t in ("pointer_expression", "parenthesized_expression",
                 "unary_expression", "cast_expression"):
            # descend into the first meaningful child
            base = None
            for ch in n.children:
                if ch.is_named:
                    base = ch
                    break
            n = base
            continue
        # Unknown wrapper: descend to first named child if any.
        nxt = None
        for ch in n.children:
            if ch.is_named:
                nxt = ch
                break
        if nxt is None or nxt is n:
            return None
        n = nxt
    return None


def _stmt_def_use(node, code_bytes: bytes) -> Tuple[Set[str], Set[str]]:
    """Compute (DEF, USE) identifier sets for a statement subtree.

    DEF = identifiers assigned/declared/updated in this statement.
    USE = identifiers read in this statement, minus pure-def targets.
    """
    defs: Set[str] = set()
    # Track the lvalue subtrees so we can keep their index reads in USE
    # (e.g. a[i] = x  => DEF {a}, USE {i, x}; a[i] read of i must remain).
    lvalue_pure_def_nodes = []  # nodes whose root identifier is a pure def

    stack = [node]
    while stack:
        n = stack.pop()
        t = n.type

        if t == "assignment_expression":
            lhs = n.child_by_field_name("left")
            if lhs is not None:
                base = _root_identifier(lhs, code_bytes)
                if base:
                    defs.add(base)
                # Only a plain identifier lhs is a *pure* def; for a[i]/p->f the
                # inner reads (i) still count as uses, so don't mark pure.
                if lhs.type == "identifier":
                    lvalue_pure_def_nodes.append(lhs)
            # Descend into both sides for further defs/uses.
            stack.extend(n.children)
            continue

        if t == "init_declarator":
            decl = n.child_by_field_name("declarator")
            if decl is not None:
                base = _root_identifier(decl, code_bytes)
                if base:
                    defs.add(base)
                    if decl.type == "identifier":
                        lvalue_pure_def_nodes.append(decl)
            stack.extend(n.children)
            continue

        if t == "declaration":
            # plain `int x;`, `int *p;`, `char buf[N];` (no initializer) —
            # the declared name is a def. init_declarator children (with an
            # initializer) are handled by their own branch above on descent.
            for ch in n.children:
                if ch.type == "identifier":
                    txt = _node_text(ch, code_bytes)
                    if txt:
                        defs.add(txt)
                        lvalue_pure_def_nodes.append(ch)
                elif ch.type in ("pointer_declarator", "array_declarator"):
                    base = _root_identifier(ch, code_bytes)
                    if base:
                        defs.add(base)
            stack.extend(n.children)
            continue

        if t == "update_expression":
            # i++ / --i : both a def and a use of i.
            for ch in n.children:
                if ch.is_named:
                    base = _root_identifier(ch, code_bytes)
                    if base:
                        defs.add(base)
            stack.extend(n.children)
            continue

        stack.extend(n.children)

    uses = _collect_identifiers(node, code_bytes)
    # Remove identifiers that are *only* pure-def targets (plain-identifier
    # lvalues). Identifiers that also appear as reads stay in `uses`.
    pure_def_names: Set[str] = set()
    for ln in lvalue_pure_def_nodes:
        txt = _node_text(ln, code_bytes)
        if txt:
            pure_def_names.add(txt)
    # A name stays a use if it is read somewhere other than its pure-def slot.
    # Conservative + simple: drop pure-def-only names from USE; if the name is
    # also used on an rhs it will still be picked up because `uses` already has
    # it — so only subtract names that do not appear in any non-def context.
    # We approximate by subtracting pure_def_names then re-adding any that are
    # genuinely read (update_expression / compound-assign already keep them
    # because their base was added to defs but also remains a textual read).
    uses = (uses - pure_def_names) | (defs & _read_again_names(node, code_bytes, pure_def_names))
    return defs, uses


def _read_again_names(node, code_bytes: bytes, candidates: Set[str]) -> Set[str]:
    """For names in `candidates`, return those that appear as a read (i.e. on
    the RHS of an assignment, inside a compound assignment, or in an update).
    Used to keep self-referential defs (x = x + 1; x += y; x++) as uses."""
    if not candidates:
        return set()
    out: Set[str] = set()
    stack = [node]
    while stack:
        n = stack.pop()
        t = n.type
        if t == "assignment_expression":
            op = None
            # tree-sitter-c exposes the operator as an anonymous child
            for ch in n.children:
                if not ch.is_named and ch.type not in ("(", ")"):
                    op = ch.type
                    break
            rhs = n.child_by_field_name("right")
            lhs = n.child_by_field_name("left")
            if rhs is not None:
                for name in _collect_identifiers(rhs, code_bytes) & candidates:
                    out.add(name)
            # compound assignment (+=, -=, ...) reads the lhs too
            if op and op != "=" and lhs is not None:
                for name in _collect_identifiers(lhs, code_bytes) & candidates:
                    out.add(name)
        elif t == "update_expression":
            for name in _collect_identifiers(n, code_bytes) & candidates:
                out.add(name)
        stack.extend(n.children)
    return out


def _contains_type(node, types: Set[str], code_bytes: bytes) -> bool:
    stack = [node]
    while stack:
        n = stack.pop()
        if n.type in types:
            return True
        stack.extend(n.children)
    return False


def _contains_sensitive_or_any_call(node, code_bytes: bytes) -> bool:
    stack = [node]
    while stack:
        n = stack.pop()
        if n.type == "call_expression":
            return True
        stack.extend(n.children)
    return False


def _contains_pointer_arith(node, code_bytes: bytes) -> bool:
    """binary_expression with + - * / where at least one operand is an
    identifier-bearing expression (pointer/arithmetic on variables)."""
    stack = [node]
    while stack:
        n = stack.pop()
        if n.type == "binary_expression":
            op = None
            for ch in n.children:
                if not ch.is_named:
                    op = ch.type
                    break
            if op in ("+", "-", "*", "/", "%", "<<", ">>"):
                if _collect_identifiers(n, code_bytes):
                    return True
        stack.extend(n.children)
    return False


def _is_criterion(stmt, code_bytes: bytes) -> bool:
    """SySeVR-style criterion test for a statement subtree."""
    # 1) any call expression (library/API call)
    if _contains_sensitive_or_any_call(stmt, code_bytes):
        return True
    # 2) array access
    if _contains_type(stmt, {"subscript_expression"}, code_bytes):
        return True
    # 3) pointer usage: dereference / field via -> / pointer_expression
    if _contains_type(stmt, {"pointer_expression"}, code_bytes):
        return True
    # field_expression with -> operator
    if _has_arrow_field(stmt, code_bytes):
        return True
    # 4) pointer / arithmetic on identifiers
    if _contains_pointer_arith(stmt, code_bytes):
        return True
    return False


def _has_arrow_field(node, code_bytes: bytes) -> bool:
    stack = [node]
    while stack:
        n = stack.pop()
        if n.type == "field_expression":
            for ch in n.children:
                if not ch.is_named and ch.type == "->":
                    return True
        stack.extend(n.children)
    return False


def _enclosing_predicates(stmt_node, predicate_index):
    """Walk ancestors of stmt_node; for every control construct that encloses
    it, return the predicate statement-id(s) recorded in predicate_index.

    predicate_index maps a control node's (start_byte, end_byte) span -> predicate
    stmt id. We key on the byte span rather than id(node): tree-sitter's Python
    bindings mint a fresh wrapper object on every `.parent`/`.children` access,
    so id(node) is not stable across separate traversals of "the same" node and
    a lookup keyed on it silently always misses.
    """
    out: List[int] = []
    p = stmt_node.parent
    guard = 0
    while p is not None and guard < 256:
        guard += 1
        if p.type in _CONTROL_TYPES:
            sid = predicate_index.get((p.start_byte, p.end_byte))
            if sid is not None:
                out.append(sid)
        p = p.parent
    return out


class _Stmt:
    __slots__ = ("sid", "node", "start_byte", "defs", "uses",
                 "start_line", "end_line", "is_predicate")

    def __init__(self, sid, node, defs, uses, is_predicate=False):
        self.sid = sid
        self.node = node
        self.start_byte = node.start_byte
        self.defs = defs
        self.uses = uses
        self.start_line = node.start_point[0]
        self.end_line = node.end_point[0]
        self.is_predicate = is_predicate


def _gather_statements(body_node, code_bytes: bytes):
    """Walk the function body and produce the list of statement units plus a
    map from control-construct node -> predicate statement id.

    Returns (stmts, predicate_index).

    We treat:
      * each child statement of a compound_statement as a statement;
      * the *condition* expression of if/while/switch and the header parts of
        for / do-while as predicate "statements".
    """
    stmts: List[_Stmt] = []
    predicate_index: Dict[int, int] = {}

    def add_stmt(node, is_predicate=False):
        defs, uses = _stmt_def_use(node, code_bytes)
        s = _Stmt(len(stmts), node, defs, uses, is_predicate)
        stmts.append(s)
        return s

    # Recursive descent that registers statements and predicates.
    def visit(node):
        t = node.type

        if t == "compound_statement":
            for ch in node.children:
                if ch.type in ("{", "}"):
                    continue
                visit(ch)
            return

        if t == "if_statement":
            cond = node.child_by_field_name("condition")
            if cond is not None:
                ps = add_stmt(cond, is_predicate=True)
                predicate_index[(node.start_byte, node.end_byte)] = ps.sid
            cons = node.child_by_field_name("consequence")
            if cons is not None:
                visit(cons)
            alt = node.child_by_field_name("alternative")
            if alt is not None:
                visit(alt)
            return

        if t in ("while_statement", "switch_statement"):
            cond = node.child_by_field_name("condition")
            if cond is not None:
                ps = add_stmt(cond, is_predicate=True)
                predicate_index[(node.start_byte, node.end_byte)] = ps.sid
            body = node.child_by_field_name("body")
            if body is not None:
                visit(body)
            return

        if t == "do_statement":
            body = node.child_by_field_name("body")
            cond = node.child_by_field_name("condition")
            # Register the condition as the predicate first so its sid maps.
            if cond is not None:
                ps = add_stmt(cond, is_predicate=True)
                predicate_index[(node.start_byte, node.end_byte)] = ps.sid
            if body is not None:
                visit(body)
            return

        if t == "for_statement":
            # for ( init ; cond ; update ) body  — treat the whole header as a
            # predicate unit (init defs + cond/update uses all matter), plus the
            # init clause as its own statement so its defs participate.
            ps = add_stmt(node, is_predicate=True)  # header captured via fields
            predicate_index[(node.start_byte, node.end_byte)] = ps.sid
            # init declaration is a def site; register it too if present.
            init = node.child_by_field_name("initializer")
            if init is not None:
                add_stmt(init)
            body = node.child_by_field_name("body")
            if body is not None:
                visit(body)
            return

        if t == "labeled_statement":
            # descend into the labeled body
            for ch in node.children:
                if ch.is_named and ch.type != "statement_identifier":
                    visit(ch)
            return

        # Leaf-ish statement kinds: register as a unit.
        add_stmt(node)
        return

    visit(body_node)
    return stmts, predicate_index


def _function_parameters(func_node, code_bytes: bytes) -> Set[str]:
    """Parameter identifier names — treated as defs at function entry."""
    if func_node is None:
        return set()
    params: Set[str] = set()
    decl = func_node.child_by_field_name("declarator")
    if decl is None:
        return params
    # Find the parameter_list anywhere under the declarator.
    stack = [decl]
    plist = None
    while stack:
        n = stack.pop()
        if n.type == "parameter_list":
            plist = n
            break
        stack.extend(n.children)
    if plist is None:
        return params
    for pd in plist.children:
        if pd.type == "parameter_declaration":
            d = pd.child_by_field_name("declarator")
            if d is not None:
                base = _root_identifier(d, code_bytes)
                if base:
                    params.add(base)
            else:
                # e.g. plain identifier params
                for ch in pd.children:
                    if ch.type == "identifier":
                        txt = _node_text(ch, code_bytes)
                        if txt:
                            params.add(txt)
    return params


def semantic_slice_indices(code: str) -> Set[int]:
    """Return the 0-based source line indices kept by an intraprocedural
    backward static slice of `code`, sliced from SySeVR-style criteria.

    Robust: on parse failure / empty slice / missing function body, return
    set(range(n_lines)) (the whole function). Never raises.
    """
    lines = code.splitlines()
    n = len(lines)
    if n == 0:
        return set()
    whole = set(range(n))

    if not _TS_OK or _PARSER is None:
        return whole

    try:
        code_bytes = code.encode("utf-8", "replace")
        tree = _PARSER.parse(code_bytes)
        root = tree.root_node

        body, func_node = _find_function_body(root)
        if body is None:
            return whole

        stmts, predicate_index = _gather_statements(body, code_bytes)
        if not stmts:
            return whole

        param_defs = _function_parameters(func_node, code_bytes)

        # --- choose criterion statements ---
        # Predicates can also be criteria (e.g. while (i < len) is arithmetic);
        # _is_criterion is applied uniformly. If none found, use the last stmt.
        criteria = [s for s in stmts if _is_criterion(s.node, code_bytes)]
        if not criteria:
            criteria = [stmts[-1]]

        # Index defs by variable name (for reaching-def lookups). Conservative:
        # any statement that defs v is a candidate definer of a use of v.
        defs_by_var: Dict[str, List[_Stmt]] = {}
        for s in stmts:
            for v in s.defs:
                defs_by_var.setdefault(v, []).append(s)

        # --- backward worklist slice ---
        slice_ids: Set[int] = set()
        worklist: List[_Stmt] = []
        for c in criteria:
            if c.sid not in slice_ids:
                slice_ids.add(c.sid)
                worklist.append(c)

        by_id = {s.sid: s for s in stmts}

        while worklist:
            s = worklist.pop()

            # Data dependence: for each used variable, pull in defining
            # statements that appear textually before/at s.
            for v in s.uses:
                if v in param_defs:
                    # parameter def is at function entry — implicitly available,
                    # nothing to add (no statement to slice for it).
                    pass
                definers = defs_by_var.get(v)
                if not definers:
                    continue
                # Conservative reaching defs: include all defs of v at or before
                # s (by start byte). If none qualifies before s (e.g. forward
                # use), include all defs of v.
                before = [d for d in definers if d.start_byte <= s.start_byte]
                chosen = before if before else definers
                for d in chosen:
                    if d.sid not in slice_ids:
                        slice_ids.add(d.sid)
                        worklist.append(d)

            # Control dependence: enclosing predicate statements.
            for pid in _enclosing_predicates(s.node, predicate_index):
                if pid not in slice_ids:
                    slice_ids.add(pid)
                    pstmt = by_id.get(pid)
                    if pstmt is not None:
                        worklist.append(pstmt)

        # --- map sliced statements to line indices ---
        kept: Set[int] = set()
        for sid in slice_ids:
            s = by_id.get(sid)
            if s is None:
                continue
            lo = s.start_line
            hi = s.end_line
            if lo < 0:
                lo = 0
            for li in range(lo, hi + 1):
                if 0 <= li < n:
                    kept.add(li)

        kept &= whole
        if not kept:
            return whole
        return kept
    except Exception:
        return whole
