static Ewk_View_Private_Data* _ewk_view_priv_new(Ewk_View_Smart_Data* smartData)
{
    Ewk_View_Private_Data* priv =
        static_cast<Ewk_View_Private_Data*>(calloc(1, sizeof(Ewk_View_Private_Data)));
    AtomicString string;
    WebCore::KURL url;

    if (!priv) {
        CRITICAL("could not allocate Ewk_View_Private_Data");
        return 0;
    }

    WebCore::Page::PageClients pageClients;
    pageClients.chromeClient = new WebCore::ChromeClientEfl(smartData->self);
    pageClients.editorClient = new WebCore::EditorClientEfl(smartData->self);
    pageClients.dragClient = new WebCore::DragClientEfl;
    pageClients.inspectorClient = new WebCore::InspectorClientEfl;
#if ENABLE(DEVICE_ORIENTATION)
    pageClients.deviceMotionClient = new WebCore::DeviceMotionClientEfl;
    pageClients.deviceOrientationClient = new WebCore::DeviceOrientationClientEfl;
#endif
    priv->page = new WebCore::Page(pageClients);
    if (!priv->page) {
        CRITICAL("Could not create WebKit Page");
        goto error_page;
    }

    priv->pageSettings = priv->page->settings();
    if (!priv->pageSettings) {
        CRITICAL("Could not get page settings.");
        goto error_settings;
    }

    priv->viewportArguments.width = WebCore::ViewportArguments::ValueAuto;
    priv->viewportArguments.height = WebCore::ViewportArguments::ValueAuto;
    priv->viewportArguments.initialScale = WebCore::ViewportArguments::ValueAuto;
    priv->viewportArguments.minimumScale = WebCore::ViewportArguments::ValueAuto;
    priv->viewportArguments.maximumScale = WebCore::ViewportArguments::ValueAuto;
    priv->viewportArguments.targetDensityDpi = WebCore::ViewportArguments::ValueAuto;
    priv->viewportArguments.userScalable = true;

    priv->pageSettings->setLoadsImagesAutomatically(true);
    priv->pageSettings->setDefaultTextEncodingName("iso-8859-1");
    priv->pageSettings->setDefaultFixedFontSize(12);
    priv->pageSettings->setDefaultFontSize(16);
    priv->pageSettings->setSerifFontFamily("serif");
    priv->pageSettings->setFixedFontFamily("monotype");
    priv->pageSettings->setSansSerifFontFamily("sans");
    priv->pageSettings->setStandardFontFamily("sans");
    priv->pageSettings->setScriptEnabled(true);
    priv->pageSettings->setPluginsEnabled(true);
    priv->pageSettings->setLocalStorageEnabled(true);
    priv->pageSettings->setOfflineWebApplicationCacheEnabled(true);
    priv->pageSettings->setUsesPageCache(true);
    priv->pageSettings->setUsesEncodingDetector(false);

    url = priv->pageSettings->userStyleSheetLocation();
    priv->settings.userStylesheet = eina_stringshare_add(url.string().utf8().data());

    priv->settings.encodingDefault = eina_stringshare_add
                                          (priv->pageSettings->defaultTextEncodingName().utf8().data());
    priv->settings.encodingCustom = 0;

    string = priv->pageSettings->localStorageDatabasePath();
    priv->settings.localStorageDatabasePath = eina_stringshare_add(string.string().utf8().data());

    priv->settings.fontMinimumSize = priv->pageSettings->minimumFontSize();
    priv->settings.fontMinimumLogicalSize = priv->pageSettings->minimumLogicalFontSize();
    priv->settings.fontDefaultSize = priv->pageSettings->defaultFontSize();
    priv->settings.fontMonospaceSize = priv->pageSettings->defaultFixedFontSize();

    string = priv->pageSettings->standardFontFamily();
    priv->settings.fontStandard = eina_stringshare_add(string.string().utf8().data());
    string = priv->pageSettings->cursiveFontFamily();
    priv->settings.fontCursive = eina_stringshare_add(string.string().utf8().data());
    string = priv->pageSettings->fixedFontFamily();
    priv->settings.fontMonospace = eina_stringshare_add(string.string().utf8().data());
    string = priv->pageSettings->fantasyFontFamily();
    priv->settings.fontFantasy = eina_stringshare_add(string.string().utf8().data());
    string = priv->pageSettings->serifFontFamily();
    priv->settings.fontSerif = eina_stringshare_add(string.string().utf8().data());
    string = priv->pageSettings->sansSerifFontFamily();
    priv->settings.fontSansSerif = eina_stringshare_add(string.string().utf8().data());

    priv->settings.autoLoadImages = priv->pageSettings->loadsImagesAutomatically();
    priv->settings.autoShrinkImages = priv->pageSettings->shrinksStandaloneImagesToFit();
    priv->settings.enableAutoResizeWindow = true;
    priv->settings.enableDeveloperExtras = priv->pageSettings->developerExtrasEnabled();
    priv->settings.enableScripts = priv->pageSettings->isScriptEnabled();
    priv->settings.enablePlugins = priv->pageSettings->arePluginsEnabled();
    priv->settings.enableFrameFlattening = priv->pageSettings->frameFlatteningEnabled();
    priv->settings.scriptsCanOpenWindows = priv->pageSettings->javaScriptCanOpenWindowsAutomatically();
    priv->settings.scriptsCanCloseWindows = priv->pageSettings->allowScriptsToCloseWindows();
    priv->settings.resizableTextareas = priv->pageSettings->textAreasAreResizable();
    priv->settings.privateBrowsing = priv->pageSettings->privateBrowsingEnabled();
    priv->settings.caretBrowsing = priv->pageSettings->caretBrowsingEnabled();
    priv->settings.spatialNavigation = priv->pageSettings->isSpatialNavigationEnabled();
    priv->settings.localStorage = priv->pageSettings->localStorageEnabled();
    priv->settings.offlineAppCache = true; // XXX no function to read setting; this keeps the original setting
    priv->settings.pageCache = priv->pageSettings->usesPageCache();
    priv->settings.encodingDetector = priv->pageSettings->usesEncodingDetector();

    priv->settings.userAgent = ewk_settings_default_user_agent_get();

    priv->settings.zoomRange.minScale = ZOOM_MIN;
    priv->settings.zoomRange.maxScale = ZOOM_MAX;
    priv->settings.zoomRange.userScalable = true;
    priv->settings.devicePixelRatio = DEVICE_PIXEL_RATIO;

    priv->settings.domTimerInterval = priv->pageSettings->defaultMinDOMTimerInterval();

    priv->mainFrame = _ewk_view_core_frame_new(smartData, priv, 0).get();
    if (!priv->mainFrame) {
        CRITICAL("Could not create main frame.");
        goto error_main_frame;
    }

    priv->history = ewk_history_new(static_cast<WebCore::BackForwardListImpl*>(priv->page->backForwardList()));
    if (!priv->history) {
        CRITICAL("Could not create history instance for view.");
        goto error_history;
    }

    return priv;

error_history:
error_main_frame:
error_settings:
    delete priv->page;
error_page:
    free(priv);
    return 0;
}
