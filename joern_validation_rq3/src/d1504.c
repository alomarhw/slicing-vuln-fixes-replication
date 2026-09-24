FormController& Document::formController()
{
    if (!m_formController) {
        m_formController = FormController::create();
        if (m_frame && m_frame->loader().currentItem() && m_frame->loader().currentItem()->isCurrentDocument(this))
            m_frame->loader().currentItem()->setDocumentState(m_formController->formElementsState());
    }
    return *m_formController;
}
