static ScreenOrientationInfo* orientationsMap(unsigned& length)
{
    DEFINE_STATIC_LOCAL(const AtomicString, portraitPrimary, ("portrait-primary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, portraitSecondary, ("portrait-secondary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, landscapePrimary, ("landscape-primary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, landscapeSecondary, ("landscape-secondary", AtomicString::ConstructFromLiteral));

    static ScreenOrientationInfo orientationMap[] = {
        { portraitPrimary, blink::WebScreenOrientationPortraitPrimary },
        { portraitSecondary, blink::WebScreenOrientationPortraitSecondary },
        { landscapePrimary, blink::WebScreenOrientationLandscapePrimary },
        { landscapeSecondary, blink::WebScreenOrientationLandscapeSecondary }
    };
    length = WTF_ARRAY_LENGTH(orientationMap);
    return orientationMap;
}
