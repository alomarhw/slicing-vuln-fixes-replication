void Polkit1Backend::preAuthAction(const QString& action, QWidget* parent)
{
    kDebug();
    if (!parent) {
        kDebug() << "Parent widget does not exist, skipping";
        return;
    }

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.Polkit1AuthAgent"))) {
        if (qApp == 0 || QApplication::type() == QApplication::Tty) {
            kDebug() << "Not streaming parent as we are on a TTY application";
        }

        qulonglong wId = parent->effectiveWinId();

        QDBusMessage methodCall =
                QDBusMessage::createMethodCall(QLatin1String("org.kde.Polkit1AuthAgent"), QLatin1String("/org/kde/Polkit1AuthAgent"), QLatin1String("org.kde.Polkit1AuthAgent"),
                                               QLatin1String("setWIdForAction"));

        methodCall << action;
        methodCall << wId;

        QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(methodCall);
        call.waitForFinished();

        if (call.isError()) {
            kWarning() << "ERROR while streaming the parent!!" << call.error();
        }
    } else {
        kDebug() << "KDE polkit agent appears too old or not registered on the bus";
    }
}
