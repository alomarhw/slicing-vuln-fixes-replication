void DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments, int timeout)
{
    QByteArray blob;
    {
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream << arguments;
    }

    const auto reply = m_busConnection.interface()->startService(helperID);
    if (!reply.isValid() && !m_busConnection.interface()->isServiceRegistered(helperID)) {
        ActionReply errorReply = ActionReply::DBusErrorReply();
        errorReply.setErrorDescription(tr("DBus Backend error: service start %1 failed: %2").arg(helperID, reply.error().message()));
        emit actionPerformed(action, errorReply);
        return;
    }

    const bool connected = m_busConnection.connect(helperID, QLatin1String("/"), QLatin1String("org.kde.kf5auth"), QLatin1String("remoteSignal"), this, SLOT(remoteSignalReceived(int,QString,QByteArray)));

    if (!connected && m_busConnection.lastError().isValid()) {
        ActionReply errorReply = ActionReply::DBusErrorReply();
        errorReply.setErrorDescription(tr("DBus Backend error: connection to helper failed. %1\n(application: %2 helper: %3)").arg(
                m_busConnection.lastError().message(),
                qApp->applicationName(),
                helperID));
        emit actionPerformed(action, errorReply);
        return;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.kf5auth"), QLatin1String("performAction"));

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID() << blob;
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QDBusPendingCall pendingCall = m_busConnection.asyncCall(message, timeout);

    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, action, watcher]() {
        watcher->deleteLater();

        const QDBusMessage reply = watcher->reply();

        if (reply.type() == QDBusMessage::ErrorMessage) {
            ActionReply r = ActionReply::DBusErrorReply();
            r.setErrorDescription(tr("DBus Backend error: could not contact the helper. "
                                    "Connection error: %1. Message error: %2").arg(reply.errorMessage(), m_busConnection.lastError().message()));
            qCWarning(KAUTH) << reply.errorMessage();

            emit actionPerformed(action, r);
        }
    });
}
