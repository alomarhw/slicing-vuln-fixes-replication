void Element::updatePseudoElement(PseudoId pseudoId, StyleChange change)
{
    PseudoElement* element = pseudoElement(pseudoId);
    if (element && (needsStyleRecalc() || shouldRecalcStyle(change, element))) {
        element->recalcStyle(needsStyleRecalc() ? Force : change);

        if (!renderer() || !pseudoElementRendererIsNeeded(renderer()->getCachedPseudoStyle(pseudoId)))
            elementRareData()->setPseudoElement(pseudoId, 0);
    } else if (change >= Inherit || needsStyleRecalc())
        createPseudoElementIfNeeded(pseudoId);
}
