  cf2_glyphpath_init( CF2_GlyphPath         glyphpath,
                      CF2_Font              font,
                      CF2_OutlineCallbacks  callbacks,
                      CF2_Fixed             scaleY,
                      /* CF2_Fixed  hShift, */
                      CF2_ArrStack          hStemHintArray,
                      CF2_ArrStack          vStemHintArray,
                      CF2_HintMask          hintMask,
                      CF2_Fixed             hintOriginY,
                      const CF2_Blues       blues,
                      const FT_Vector*      fractionalTranslation )
  {
    FT_ZERO( glyphpath );

    glyphpath->font      = font;
    glyphpath->callbacks = callbacks;

    cf2_arrstack_init( &glyphpath->hintMoves,
                       font->memory,
                       &font->error,
                       sizeof ( CF2_HintMoveRec ) );

    cf2_hintmap_init( &glyphpath->initialHintMap,
                      font,
                      &glyphpath->initialHintMap,
                      &glyphpath->hintMoves,
                      scaleY );
    cf2_hintmap_init( &glyphpath->firstHintMap,
                      font,
                      &glyphpath->initialHintMap,
                      &glyphpath->hintMoves,
                      scaleY );
    cf2_hintmap_init( &glyphpath->hintMap,
                      font,
                      &glyphpath->initialHintMap,
                      &glyphpath->hintMoves,
                      scaleY );

    glyphpath->scaleX = font->innerTransform.a;
    glyphpath->scaleC = font->innerTransform.c;
    glyphpath->scaleY = font->innerTransform.d;

    glyphpath->fractionalTranslation = *fractionalTranslation;

#if 0
    glyphpath->hShift = hShift;       /* for fauxing */
#endif

    glyphpath->hStemHintArray = hStemHintArray;
    glyphpath->vStemHintArray = vStemHintArray;
    glyphpath->hintMask       = hintMask;      /* ptr to current mask */
    glyphpath->hintOriginY    = hintOriginY;
    glyphpath->blues          = blues;
    glyphpath->darken         = font->darkened; /* TODO: should we make copies? */
    glyphpath->xOffset        = font->darkenX;
    glyphpath->yOffset        = font->darkenY;
    glyphpath->miterLimit     = 2 * FT_MAX(
                                     cf2_fixedAbs( glyphpath->xOffset ),
                                     cf2_fixedAbs( glyphpath->yOffset ) );

    /* .1 character space unit */
    glyphpath->snapThreshold = cf2_floatToFixed( 0.1f );

    glyphpath->moveIsPending = TRUE;
    glyphpath->pathIsOpen    = FALSE;
    glyphpath->pathIsClosing = FALSE;
    glyphpath->elemIsQueued  = FALSE;
  }
