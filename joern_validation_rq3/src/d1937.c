void SVGDocumentExtensions::dispatchSVGLoadEventToOutermostSVGElements()
{
    WillBeHeapVector<RefPtrWillBeMember<SVGSVGElement> > timeContainers;
    timeContainers.appendRange(m_timeContainers.begin(), m_timeContainers.end());

    WillBeHeapVector<RefPtrWillBeMember<SVGSVGElement> >::iterator end = timeContainers.end();
    for (WillBeHeapVector<RefPtrWillBeMember<SVGSVGElement> >::iterator it = timeContainers.begin(); it != end; ++it) {
        SVGSVGElement* outerSVG = (*it).get();
        if (!outerSVG->isOutermostSVGSVGElement())
            continue;

        if (outerSVG->document().wellFormed() || !outerSVG->document().isSVGDocument())
            outerSVG->sendSVGLoadEventIfPossible();
    }
}
