TabContentsWrapper* Browser::AddSelectedTabWithURL(
    const GURL& url,
    PageTransition::Type transition) {
  browser::NavigateParams params(this, url, transition);
  params.disposition = NEW_FOREGROUND_TAB;
  browser::Navigate(&params);
  return params.target_contents;
}
