RuleFeatureSet::extractInvalidationSetFeatures(const CSSSelector& selector, InvalidationSetFeatures& features, PositionType position, CSSSelector::PseudoType pseudo)
{
    bool foundFeatures = false;
    for (const CSSSelector* current = &selector; current; current = current->tagHistory()) {
        if (pseudo != CSSSelector::PseudoNot)
            foundFeatures |= extractInvalidationSetFeature(*current, features);
        if (InvalidationSet* invalidationSet = invalidationSetForSelector(*current, InvalidateDescendants)) {
            if (position == Subject)
                invalidationSet->setInvalidatesSelf();
        } else {
            if (requiresSubtreeInvalidation(*current)) {
                return std::make_pair(&selector, ForceSubtree);
            }
            if (const CSSSelectorList* selectorList = current->selectorList()) {
                ASSERT(supportsInvalidationWithSelectorList(current->pseudoType()));
                const CSSSelector* subSelector = selectorList->first();
                bool allSubSelectorsHaveFeatures = !!subSelector;
                for (; subSelector; subSelector = CSSSelectorList::next(*subSelector)) {
                    auto result = extractInvalidationSetFeatures(*subSelector, features, position, current->pseudoType());
                    if (result.first) {
                        return std::make_pair(&selector, ForceSubtree);
                    }
                    allSubSelectorsHaveFeatures &= result.second == UseFeatures;
                }
                foundFeatures |= allSubSelectorsHaveFeatures;
            }
        }

        if (current->relation() == CSSSelector::SubSelector)
            continue;

        features.treeBoundaryCrossing = current->isShadowSelector();
        features.adjacent = current->isAdjacentSelector();
        if (current->relation() == CSSSelector::DirectAdjacent)
            features.maxDirectAdjacentSelectors = 1;
        return std::make_pair(current->tagHistory(), foundFeatures ? UseFeatures : ForceSubtree);
    }
    return std::make_pair(nullptr,  foundFeatures ? UseFeatures : ForceSubtree);
}
