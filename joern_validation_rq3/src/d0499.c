void Document::webkitExitFullscreen()
{
    
    Document* currentDoc = this;

    if (m_fullScreenElementStack.isEmpty())
        return;
    
    Deque<RefPtr<Document> > descendants;
    for (Frame* descendant = frame() ? frame()->tree()->traverseNext() : 0; descendant; descendant = descendant->tree()->traverseNext()) {
        if (descendant->document()->webkitFullscreenElement())
            descendants.prepend(descendant->document());
    }
        
    for (Deque<RefPtr<Document> >::iterator i = descendants.begin(); i != descendants.end(); ++i) {
        (*i)->clearFullscreenElementStack();
        addDocumentToFullScreenChangeEventQueue(i->get());
    }

    Element* newTop = 0;
    while (currentDoc) {
        currentDoc->popFullscreenElementStack();

        newTop = currentDoc->webkitFullscreenElement();
        if (newTop && (!newTop->inDocument() || newTop->document() != currentDoc))
            continue;

        addDocumentToFullScreenChangeEventQueue(currentDoc);

        if (!newTop && currentDoc->ownerElement()) {
            currentDoc = currentDoc->ownerElement()->document();
            continue;
        }

        currentDoc = 0;
    }


    if (!page())
        return;

    if (!newTop) {
        page()->chrome()->client()->exitFullScreenForElement(m_fullScreenElement.get());
        return;
    }

    page()->chrome()->client()->enterFullScreenForElement(newTop);      
}
