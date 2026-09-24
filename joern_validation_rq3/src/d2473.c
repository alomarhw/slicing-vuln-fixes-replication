bool WebPage::blockZoom(const Platform::IntPoint& documentTargetPoint)
{
    if (!d->m_mainFrame->view() || !d->isUserScalable())
        return false;

    Node* node = d->bestNodeForZoomUnderPoint(documentTargetPoint);
    if (!node)
        return false;

    IntRect nodeRect = d->rectForNode(node);
    IntRect blockRect;
    bool endOfBlockZoomMode = d->compareNodesForBlockZoom(d->m_currentBlockZoomAdjustedNode.get(), node);
    const double oldScale = d->m_transformationMatrix->m11();
    double newScale = 0;
    const double margin = endOfBlockZoomMode ? 0 : blockZoomMargin * 2 * oldScale;
    bool isFirstZoom = false;

    if (endOfBlockZoomMode) {
        IntRect rect = d->blockZoomRectForNode(node);
        blockRect = IntRect(0, rect.y(), d->transformedContentsSize().width(), d->transformedContentsSize().height() - rect.y());
        d->m_shouldReflowBlock = false;
    } else {
        Node* tempBlockZoomAdjustedNode = d->m_currentBlockZoomAdjustedNode.get();
        blockRect = d->blockZoomRectForNode(node);

        if (!node->hasTagName(HTMLNames::imgTag)) {
            IntRect tRect = d->mapFromTransformed(blockRect);
            int blockArea = tRect.width() * tRect.height();
            int pageArea = d->contentsSize().width() * d->contentsSize().height();
            double blockToPageRatio = static_cast<double>(pageArea - blockArea) / pageArea;
            if (blockToPageRatio < minimumExpandingRatio) {
                d->m_currentBlockZoomAdjustedNode = tempBlockZoomAdjustedNode;
                return false;
            }
        }

        if (blockRect.isEmpty() || !blockRect.width() || !blockRect.height())
            return false;

        if (!d->m_currentBlockZoomNode.get())
            isFirstZoom = true;

        d->m_currentBlockZoomNode = node;
        d->m_shouldReflowBlock = true;
    }

    newScale = std::min(d->newScaleForBlockZoomRect(blockRect, oldScale, margin), d->maxBlockZoomScale());
    newScale = std::max(newScale, minimumScale());

#if ENABLE(VIEWPORT_REFLOW)
    if (d->m_currentBlockZoomNode && d->m_shouldReflowBlock && settings()->textReflowMode() != WebSettings::TextReflowDisabled) {
        RenderObject* renderer = d->m_currentBlockZoomNode->renderer();
        if (renderer && renderer->isText()) {
            double newFontSize = renderer->style()->fontSize() * newScale;
            if (newFontSize < d->m_webSettings->defaultFontSize()) {
                newScale = std::min(static_cast<double>(d->m_webSettings->defaultFontSize()) / renderer->style()->fontSize(), d->maxBlockZoomScale());
                newScale = std::max(newScale, minimumScale());
            }
            blockRect.setWidth(oldScale * static_cast<double>(d->transformedActualVisibleSize().width()) / newScale);
            newScale = std::min(d->newScaleForBlockZoomRect(blockRect, oldScale, margin), d->maxBlockZoomScale());
            newScale = std::max(newScale, minimumScale()); // Still, it's not allowed to be smaller than minimum scale.
        }
    }
#endif

    double newBlockHeight = d->mapFromTransformed(blockRect).height();
    double newBlockWidth = d->mapFromTransformed(blockRect).width();
    double scaledViewportWidth = static_cast<double>(d->actualVisibleSize().width()) * oldScale / newScale;
    double scaledViewportHeight = static_cast<double>(d->actualVisibleSize().height()) * oldScale / newScale;
    double dx = std::max(0.0, (scaledViewportWidth - newBlockWidth) / 2.0);
    double dy = std::max(0.0, (scaledViewportHeight - newBlockHeight) / 2.0);

    RenderObject* renderer = d->m_currentBlockZoomAdjustedNode->renderer();
    FloatPoint anchor;
    FloatPoint topLeftPoint(d->mapFromTransformed(blockRect).location());
    if (renderer && renderer->isText()) {
        ETextAlign textAlign = renderer->style()->textAlign();
        switch (textAlign) {
        case CENTER:
        case WEBKIT_CENTER:
            anchor = FloatPoint(nodeRect.x() + (nodeRect.width() - scaledViewportWidth) / 2, topLeftPoint.y());
            break;
        case LEFT:
        case WEBKIT_LEFT:
            anchor = topLeftPoint;
            break;
        case RIGHT:
        case WEBKIT_RIGHT:
            anchor = FloatPoint(nodeRect.x() + nodeRect.width() - scaledViewportWidth, topLeftPoint.y());
            break;
        case TAAUTO:
        case JUSTIFY:
        default:
            if (renderer->style()->isLeftToRightDirection())
                anchor = topLeftPoint;
            else
                anchor = FloatPoint(nodeRect.x() + nodeRect.width() - scaledViewportWidth, topLeftPoint.y());
            break;
        }
    } else
        anchor = renderer->style()->isLeftToRightDirection() ? topLeftPoint : FloatPoint(nodeRect.x() + nodeRect.width() - scaledViewportWidth, topLeftPoint.y());

    if (newBlockHeight <= scaledViewportHeight) {
        d->m_finalBlockPoint = FloatPoint(anchor.x() - dx, anchor.y() - dy);
    } else {
        d->m_finalBlockPoint = FloatPoint(anchor.x() - dx, anchor.y() - 3);
    }

#if ENABLE(VIEWPORT_REFLOW)
    if (settings()->textReflowMode() != WebSettings::TextReflowDisabled) {
        d->m_finalBlockPoint = FloatPoint(anchor.x() - dx, anchor.y() - 3);
        d->m_finalBlockPointReflowOffset = FloatPoint(-dx, -3);
    }
#endif

    FloatRect br(anchor, FloatSize(scaledViewportWidth, scaledViewportHeight));
    if (!br.contains(IntPoint(documentTargetPoint))) {
        d->m_finalBlockPointReflowOffset.move(0, (documentTargetPoint.y() - scaledViewportHeight / 2) - d->m_finalBlockPoint.y());
        d->m_finalBlockPoint = FloatPoint(d->m_finalBlockPoint.x(), documentTargetPoint.y() - scaledViewportHeight / 2);
    }

    if (d->m_finalBlockPoint.x() < 0) {
        d->m_finalBlockPoint.setX(0);
        d->m_finalBlockPointReflowOffset.setX(0);
    } else if (d->m_finalBlockPoint.x() + scaledViewportWidth > d->contentsSize().width()) {
        d->m_finalBlockPoint.setX(d->contentsSize().width() - scaledViewportWidth);
        d->m_finalBlockPointReflowOffset.setX(0);
    }

    if (d->m_finalBlockPoint.y() < 0) {
        d->m_finalBlockPoint.setY(0);
        d->m_finalBlockPointReflowOffset.setY(0);
    } else if (d->m_finalBlockPoint.y() + scaledViewportHeight > d->contentsSize().height()) {
        d->m_finalBlockPoint.setY(d->contentsSize().height() - scaledViewportHeight);
        d->m_finalBlockPointReflowOffset.setY(0);
    }

    if (!endOfBlockZoomMode && abs(newScale - oldScale) / oldScale < minimumExpandingRatio) {
        const double minimumDisplacement = minimumExpandingRatio * webkitThreadViewportAccessor()->documentViewportSize().width();
        if (oldScale == d->minimumScale() || (distanceBetweenPoints(d->scrollPosition(), roundUntransformedPoint(d->m_finalBlockPoint)) < minimumDisplacement && abs(newScale - oldScale) / oldScale < 0.10)) {
            if (isFirstZoom) {
                d->resetBlockZoom();
                return false;
            }
            blockZoom(documentTargetPoint);
            return true;
        }
    }

    d->m_blockZoomFinalScale = newScale;

    d->m_userPerformedManualZoom = true;
    d->m_userPerformedManualScroll = true;
    d->m_client->animateBlockZoom(d->m_blockZoomFinalScale, d->m_finalBlockPoint);

    return true;
}
