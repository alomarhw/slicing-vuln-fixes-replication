void DocumentVisibilityObserver::setObservedDocument(Document& document)
{
    unregisterObserver();
    registerObserver(document);
}
