bool RenderBox::repaintLayerRectsForImage(WrappedImagePtr image, const FillLayer* layers, bool drawingBackground)
{
    IntRect rendererRect;
    RenderBox* layerRenderer = 0;

    for (const FillLayer* curLayer = layers; curLayer; curLayer = curLayer->next()) {
        if (curLayer->image() && image == curLayer->image()->data() && curLayer->image()->canRender(style()->effectiveZoom())) {
            if (!layerRenderer) {
                bool drawingRootBackground = drawingBackground && (isRoot() || (isBody() && !document()->documentElement()->renderer()->hasBackground()));
                if (drawingRootBackground) {
                    layerRenderer = view();

                    int rw;
                    int rh;

                    if (FrameView* frameView = toRenderView(layerRenderer)->frameView()) {
                        rw = frameView->contentsWidth();
                        rh = frameView->contentsHeight();
                    } else {
                        rw = layerRenderer->width();
                        rh = layerRenderer->height();
                    }
                    rendererRect = IntRect(-layerRenderer->marginLeft(),
                        -layerRenderer->marginTop(),
                        max(layerRenderer->width() + layerRenderer->marginLeft() + layerRenderer->marginRight() + layerRenderer->borderLeft() + layerRenderer->borderRight(), rw),
                        max(layerRenderer->height() + layerRenderer->marginTop() + layerRenderer->marginBottom() + layerRenderer->borderTop() + layerRenderer->borderBottom(), rh));
                } else {
                    layerRenderer = this;
                    rendererRect = borderBoxRect();
                }
            }

            BackgroundImageGeometry geometry;
            layerRenderer->calculateBackgroundImageGeometry(curLayer, rendererRect, geometry);
            layerRenderer->repaintRectangle(geometry.destRect());
            if (geometry.destRect() == rendererRect)
                return true;
        }
    }
    return false;
}
