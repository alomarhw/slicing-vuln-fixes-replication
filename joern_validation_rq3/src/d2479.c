static IntPoint extractClickLocation(Event* event)
{
    if (!event->underlyingEvent() || !event->underlyingEvent()->isMouseEvent())
        return IntPoint();
    MouseEvent* mouseEvent = toMouseEvent(event->underlyingEvent());
    if (!mouseEvent->hasPosition())
        return IntPoint();
    return IntPoint(mouseEvent->offsetX(), mouseEvent->offsetY());
}
