static void removeBlockFromDescendantAndContainerMaps(RenderBlock* block, TrackedDescendantsMap*& descendantMap, TrackedContainerMap*& containerMap)
{
    if (OwnPtr<TrackedRendererListHashSet> descendantSet = descendantMap->take(block)) {
        TrackedRendererListHashSet::iterator end = descendantSet->end();
        for (TrackedRendererListHashSet::iterator descendant = descendantSet->begin(); descendant != end; ++descendant) {
            TrackedContainerMap::iterator it = containerMap->find(*descendant);
            ASSERT(it != containerMap->end());
            if (it == containerMap->end())
                continue;
            HashSet<RenderBlock*>* containerSet = it->value.get();
            ASSERT(containerSet->contains(block));
            containerSet->remove(block);
            if (containerSet->isEmpty())
                containerMap->remove(it);
        }
    }
}
