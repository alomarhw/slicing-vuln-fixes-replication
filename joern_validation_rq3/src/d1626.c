void webkit_web_view_set_highlight_text_matches(WebKitWebView* webView, gboolean shouldHighlight)
{
    g_return_if_fail(WEBKIT_IS_WEB_VIEW(webView));

    Frame *frame = core(webView)->mainFrame();
    do {
        frame->editor()->setMarkedTextMatchesAreHighlighted(shouldHighlight);
        frame = frame->tree()->traverseNextWithWrap(false);
    } while (frame);
}
