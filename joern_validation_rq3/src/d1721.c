void GraphicsContext::setPlatformStrokeStyle(StrokeStyle stroke)
{
    if (paintingDisabled())
        return;

    platformContext()->setStrokeStyle(stroke);
}
