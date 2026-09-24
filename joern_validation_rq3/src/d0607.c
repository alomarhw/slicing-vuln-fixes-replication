DeliverToWindowOwner(DeviceIntPtr dev, WindowPtr win,
                     xEvent *events, int count, Mask filter, GrabPtr grab)
{
    /* if nobody ever wants to see this event, skip some work */
    if (filter != CantBeFiltered &&
        !((wOtherEventMasks(win) | win->eventMask) & filter))
        return EVENT_SKIP;

    if (IsInterferingGrab(wClient(win), dev, events))
        return EVENT_SKIP;

    if (!XaceHook(XACE_RECEIVE_ACCESS, wClient(win), win, events, count)) {
        int attempt = TryClientEvents(wClient(win), dev, events,
                                      count, win->eventMask,
                                      filter, grab);

        if (attempt > 0)
            return EVENT_DELIVERED;
        if (attempt < 0)
            return EVENT_REJECTED;
    }

    return EVENT_NOT_DELIVERED;
}
