/*
    SPDX-FileCopyrightText: 2018 Jonathan Riddell <jr@jriddell.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DISTRORELEASENOTIFIER_H
#define DISTRORELEASENOTIFIER_H

#include <QObject>

class DBusInterface;
class Notifier;
class QProcess;
class QNetworkReply;
class UpgraderProcess;

class DistroReleaseNotifier : public QObject
{
    Q_OBJECT
public:
    DistroReleaseNotifier(QObject *parent = nullptr);

private Q_SLOTS:
    void checkReleaseUpgradeFinished(int exitCode);
    void releaseUpgradeCheck();
    void releaseUpgradeActivated();
    void forceCheck();
    void replyFinished(QNetworkReply *reply);

private:
    DBusInterface *m_dbus;
    QProcess *m_checkerProcess;
    Notifier *m_notifier;

    // This acts as a safe guard. We listen to network device connections
    // to check on network connections. This can get super annoying for users
    // if we do in fact act on this a lot of times. So, instead this var
    // tracks if we ever had a successful check and if so prevents any further
    // checks from even running.
    bool m_hasChecked;
    QString m_name;
    QString m_version;

    // Upgrader is started but not yet on dbus = pending.
    // This process auto-deleted itself.
    UpgraderProcess *m_pendingUpgrader = nullptr;
};

#endif // DISTRORELEASENOTIFIER_H
