void IRCView::appendServerMessage(const QString& type, const QString& message, bool parseURL)
{
    QString serverColor = Preferences::self()->color(Preferences::ServerMessage).name();
    m_tabNotification = Konversation::tnfControl;

    QString fixed;
    if(Preferences::self()->fixedMOTD() && !m_fontDataBase.isFixedPitch(font().family()))
    {
        if(type == i18n("MOTD"))
            fixed=" face=\"" + KGlobalSettings::fixedFont().family() + "\"";
    }

    QString line;
    QChar::Direction dir;
    QString text(filter(message, serverColor, 0 , true, parseURL, false, &dir));
    bool rtl = (dir == QChar::DirR);

    if(rtl)
    {
        line = RLE;
        line += LRE;
        line += "<font color=\"" + serverColor + "\"" + fixed + "><b>[</b>%2<b>]</b> %1" + PDF + " %3</font>";
    }
    else
    {
        if (!QApplication::isLeftToRight())
            line += LRE;

        line += "<font color=\"" + serverColor + "\"" + fixed + ">%1 <b>[</b>%2<b>]</b> %3</font>";
    }

    line = line.arg(timeStamp(), type, text);

    emit textToLog(QString("%1\t%2").arg(type, message));

    doAppend(line, rtl);
}
