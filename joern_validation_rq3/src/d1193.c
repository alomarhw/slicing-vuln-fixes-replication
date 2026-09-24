void RenderBox::computePositionedLogicalWidthUsing(Length logicalWidth, const RenderBoxModelObject* containerBlock, TextDirection containerDirection,
                                                   int containerLogicalWidth, int bordersPlusPadding,
                                                   Length logicalLeft, Length logicalRight, Length marginLogicalLeft, Length marginLogicalRight,
                                                   int& logicalWidthValue, int& marginLogicalLeftValue, int& marginLogicalRightValue, int& logicalLeftPos)
{
    ASSERT(!(logicalLeft.isAuto() && logicalRight.isAuto()));

    int logicalLeftValue = 0;

    bool logicalWidthIsAuto = logicalWidth.isIntrinsicOrAuto();
    bool logicalLeftIsAuto = logicalLeft.isAuto();
    bool logicalRightIsAuto = logicalRight.isAuto();

    if (!logicalLeftIsAuto && !logicalWidthIsAuto && !logicalRightIsAuto) {
        /*-----------------------------------------------------------------------*\
         * If none of the three is 'auto': If both 'margin-left' and 'margin-
         * right' are 'auto', solve the equation under the extra constraint that
         * the two margins get equal values, unless this would make them negative,
         * in which case when direction of the containing block is 'ltr' ('rtl'),
         * set 'margin-left' ('margin-right') to zero and solve for 'margin-right'
         * ('margin-left'). If one of 'margin-left' or 'margin-right' is 'auto',
         * solve the equation for that value. If the values are over-constrained,
         * ignore the value for 'left' (in case the 'direction' property of the
         * containing block is 'rtl') or 'right' (in case 'direction' is 'ltr')
         * and solve for that value.
        \*-----------------------------------------------------------------------*/

        logicalLeftValue = logicalLeft.calcValue(containerLogicalWidth);
        logicalWidthValue = computeContentBoxLogicalWidth(logicalWidth.calcValue(containerLogicalWidth));

        const int availableSpace = containerLogicalWidth - (logicalLeftValue + logicalWidthValue + logicalRight.calcValue(containerLogicalWidth) + bordersPlusPadding);

        if (marginLogicalLeft.isAuto() && marginLogicalRight.isAuto()) {
            if (availableSpace >= 0) {
                marginLogicalLeftValue = availableSpace / 2; // split the difference
                marginLogicalRightValue = availableSpace - marginLogicalLeftValue; // account for odd valued differences
            } else {
                if (containerDirection == LTR) {
                    marginLogicalLeftValue = 0;
                    marginLogicalRightValue = availableSpace; // will be negative
                } else {
                    marginLogicalLeftValue = availableSpace; // will be negative
                    marginLogicalRightValue = 0;
                }
            }
        } else if (marginLogicalLeft.isAuto()) {
            marginLogicalRightValue = marginLogicalRight.calcValue(containerLogicalWidth);
            marginLogicalLeftValue = availableSpace - marginLogicalRightValue;
        } else if (marginLogicalRight.isAuto()) {
            marginLogicalLeftValue = marginLogicalLeft.calcValue(containerLogicalWidth);
            marginLogicalRightValue = availableSpace - marginLogicalLeftValue;
        } else {
            marginLogicalLeftValue = marginLogicalLeft.calcValue(containerLogicalWidth);
            marginLogicalRightValue = marginLogicalRight.calcValue(containerLogicalWidth);

            if (containerDirection == RTL)
                logicalLeftValue = (availableSpace + logicalLeftValue) - marginLogicalLeftValue - marginLogicalRightValue;
        }
    } else {
        /*--------------------------------------------------------------------*\
         * Otherwise, set 'auto' values for 'margin-left' and 'margin-right'
         * to 0, and pick the one of the following six rules that applies.
         *
         * 1. 'left' and 'width' are 'auto' and 'right' is not 'auto', then the
         *    width is shrink-to-fit. Then solve for 'left'
         *
         *              OMIT RULE 2 AS IT SHOULD NEVER BE HIT
         * ------------------------------------------------------------------
         * 2. 'left' and 'right' are 'auto' and 'width' is not 'auto', then if
         *    the 'direction' property of the containing block is 'ltr' set
         *    'left' to the static position, otherwise set 'right' to the
         *    static position. Then solve for 'left' (if 'direction is 'rtl')
         *    or 'right' (if 'direction' is 'ltr').
         * ------------------------------------------------------------------
         *
         * 3. 'width' and 'right' are 'auto' and 'left' is not 'auto', then the
         *    width is shrink-to-fit . Then solve for 'right'
         * 4. 'left' is 'auto', 'width' and 'right' are not 'auto', then solve
         *    for 'left'
         * 5. 'width' is 'auto', 'left' and 'right' are not 'auto', then solve
         *    for 'width'
         * 6. 'right' is 'auto', 'left' and 'width' are not 'auto', then solve
         *    for 'right'
         *
         * Calculation of the shrink-to-fit width is similar to calculating the
         * width of a table cell using the automatic table layout algorithm.
         * Roughly: calculate the preferred width by formatting the content
         * without breaking lines other than where explicit line breaks occur,
         * and also calculate the preferred minimum width, e.g., by trying all
         * possible line breaks. CSS 2.1 does not define the exact algorithm.
         * Thirdly, calculate the available width: this is found by solving
         * for 'width' after setting 'left' (in case 1) or 'right' (in case 3)
         * to 0.
         *
         * Then the shrink-to-fit width is:
         * min(max(preferred minimum width, available width), preferred width).
        \*--------------------------------------------------------------------*/

        marginLogicalLeftValue = marginLogicalLeft.calcMinValue(containerLogicalWidth);
        marginLogicalRightValue = marginLogicalRight.calcMinValue(containerLogicalWidth);

        const int availableSpace = containerLogicalWidth - (marginLogicalLeftValue + marginLogicalRightValue + bordersPlusPadding);

        if (logicalLeftIsAuto && logicalWidthIsAuto && !logicalRightIsAuto) {
            int logicalRightValue = logicalRight.calcValue(containerLogicalWidth);

            int preferredWidth = maxPreferredLogicalWidth() - bordersPlusPadding;
            int preferredMinWidth = minPreferredLogicalWidth() - bordersPlusPadding;
            int availableWidth = availableSpace - logicalRightValue;
            logicalWidthValue = min(max(preferredMinWidth, availableWidth), preferredWidth);
            logicalLeftValue = availableSpace - (logicalWidthValue + logicalRightValue);
        } else if (!logicalLeftIsAuto && logicalWidthIsAuto && logicalRightIsAuto) {
            logicalLeftValue = logicalLeft.calcValue(containerLogicalWidth);

            int preferredWidth = maxPreferredLogicalWidth() - bordersPlusPadding;
            int preferredMinWidth = minPreferredLogicalWidth() - bordersPlusPadding;
            int availableWidth = availableSpace - logicalLeftValue;
            logicalWidthValue = min(max(preferredMinWidth, availableWidth), preferredWidth);
        } else if (logicalLeftIsAuto && !logicalWidthIsAuto && !logicalRightIsAuto) {
            logicalWidthValue = computeContentBoxLogicalWidth(logicalWidth.calcValue(containerLogicalWidth));
            logicalLeftValue = availableSpace - (logicalWidthValue + logicalRight.calcValue(containerLogicalWidth));
        } else if (!logicalLeftIsAuto && logicalWidthIsAuto && !logicalRightIsAuto) {
            logicalLeftValue = logicalLeft.calcValue(containerLogicalWidth);
            logicalWidthValue = availableSpace - (logicalLeftValue + logicalRight.calcValue(containerLogicalWidth));
        } else if (!logicalLeftIsAuto && !logicalWidthIsAuto && logicalRightIsAuto) {
            logicalLeftValue = logicalLeft.calcValue(containerLogicalWidth);
            logicalWidthValue = computeContentBoxLogicalWidth(logicalWidth.calcValue(containerLogicalWidth));
        }
    }


    if (containerBlock->isRenderInline() && !containerBlock->style()->isLeftToRightDirection()) {
        const RenderInline* flow = toRenderInline(containerBlock);
        InlineFlowBox* firstLine = flow->firstLineBox();
        InlineFlowBox* lastLine = flow->lastLineBox();
        if (firstLine && lastLine && firstLine != lastLine) {
            logicalLeftPos = logicalLeftValue + marginLogicalLeftValue + lastLine->borderLogicalLeft() + (lastLine->logicalLeft() - firstLine->logicalLeft());
            return;
        }
    }

    logicalLeftPos = logicalLeftValue + marginLogicalLeftValue;
    computeLogicalLeftPositionedOffset(logicalLeftPos, this, logicalWidthValue, containerBlock, containerLogicalWidth);
}
