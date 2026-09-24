// Joern reference backward slice for the RQ1 slicer validation.
//
// Seeds: the same SySeVR-style sink categories as ast_slicer.py -- every non-operator
// call, array subscripts, pointer dereferences, -> field accesses, and pointer/arith
// operators. From the seeds, the slice is the fixed point over Joern's flow-sensitive
// reaching-definition (DDG) edges and control-dependence (CDG) edges, inside the method.
// Parameter and method nodes are dropped (our slicer never adds the signature line for
// a parameter use). Output: one JSON line per method: {"file","method","lines":[...]}.

import scala.collection.mutable

val SinkOps = Set(
  "<operator>.indirectIndexAccess", "<operator>.indexAccess", "<operator>.indirection",
  "<operator>.indirectFieldAccess", "<operator>.pointerShift",
  "<operator>.addition", "<operator>.subtraction", "<operator>.multiplication",
  "<operator>.assignmentPlus", "<operator>.assignmentMinus",
  "<operator>.postIncrement", "<operator>.preIncrement",
  "<operator>.postDecrement", "<operator>.preDecrement")

def isSink(c: Call): Boolean = !c.name.startsWith("<operator>") || SinkOps.contains(c.name)

@main def exec(cpgFile: String, outFile: String) = {
  importCpg(cpgFile)
  val out = new java.io.PrintWriter(outFile)
  cpg.method.isExternal(false).l.foreach { m =>
    val file = m.file.name.headOption.getOrElse("")
    if (file.matches(""".*d\d+\.c""") && m.name != "<global>") {
      val seeds: List[CfgNode] = m.call.filter(isSink).l
      val visited = mutable.Set[CfgNode]()
      val work = mutable.Stack[CfgNode]()
      seeds.foreach(work.push)
      while (work.nonEmpty) {
        val n = work.pop()
        if (!visited.contains(n)) {
          visited += n
          val sub: List[CfgNode] = n :: n.ast.isCfgNode.l
          sub.foreach { x =>
            x._reachingDefIn.collectAll[CfgNode].foreach(d => if (!visited.contains(d)) work.push(d))
            x.controlledBy.foreach(c => if (!visited.contains(c)) work.push(c))
          }
        }
      }
      val lines = visited.toList
        .filterNot(n => n.isInstanceOf[MethodParameterIn] || n.isInstanceOf[Method] || n.isInstanceOf[MethodReturn])
        .flatMap(_.lineNumber.map(_.toInt)).distinct.sorted
      out.println(s"""{"file":"${file.replace("\\", "/")}","method":"${m.name.replace("\"", "")}","seeds":${seeds.size},"lines":[${lines.mkString(",")}]}""")
    }
  }
  out.close()
}
