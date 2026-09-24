void RenderBlock::simplifiedNormalFlowLayout()
{
    if (childrenInline()) {
        ListHashSet<RootInlineBox*> lineBoxes;
        for (InlineWalker walker(this); !walker.atEnd(); walker.advance()) {
            RenderObject* o = walker.current();
            if (!o->isOutOfFlowPositioned() && (o->isReplaced() || o->isFloating())) {
                o->layoutIfNeeded();
                if (toRenderBox(o)->inlineBoxWrapper()) {
                    RootInlineBox& box = toRenderBox(o)->inlineBoxWrapper()->root();
                    lineBoxes.add(&box);
                }
            } else if (o->isText() || (o->isRenderInline() && !walker.atEndOfInline())) {
                o->clearNeedsLayout();
            }
        }

        GlyphOverflowAndFallbackFontsMap textBoxDataMap;
        for (ListHashSet<RootInlineBox*>::const_iterator it = lineBoxes.begin(); it != lineBoxes.end(); ++it) {
            RootInlineBox* box = *it;
            box->computeOverflow(box->lineTop(), box->lineBottom(), textBoxDataMap);
        }
    } else {
        for (RenderBox* box = firstChildBox(); box; box = box->nextSiblingBox()) {
            if (!box->isOutOfFlowPositioned())
                box->layoutIfNeeded();
        }
    }
}
