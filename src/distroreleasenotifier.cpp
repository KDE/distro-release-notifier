/*
 Copyright 2018 Jonathan Riddell <jr@jriddell.org>
 Copyright 2018 Harald Sitter <sitter@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "distroreleasenotifier.h"

#include <KNotification>
#include <KLocalizedString>

#include <QJsonDocument>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#include "config.h"
#include "dbusinterface.h"
#include "debug.h"
#include "OSRelease.h"

DistroReleaseNotifier::DistroReleaseNotifier(QObject *parent)
    : QObject(parent)
    , m_dbus(new DBusInterface(this))
    , m_notification(nullptr)
{
    // check after 10 seconds
    QTimer::singleShot(10 * 1000, this, &DistroReleaseNotifier::releaseUpgradeCheck);

    QTimer *regularCheck = new QTimer(this);
    regularCheck->setInterval(24 * 60 * 60 * 1000); //refresh once every day
    connect(regularCheck, &QTimer::timeout, this, &DistroReleaseNotifier::releaseUpgradeCheck);
    regularCheck->start();

    connect(m_dbus, &DBusInterface::useDevelChanged,
            this, &DistroReleaseNotifier::releaseUpgradeCheck);
    connect(m_dbus, &DBusInterface::pollingRequested,
            this, &DistroReleaseNotifier::releaseUpgradeCheck);
}

DistroReleaseNotifier::~DistroReleaseNotifier()
{
}

void DistroReleaseNotifier::releaseUpgradeCheck()
{
    const QString checkerFile =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   QStringLiteral("distro-release-notifier/releasechecker"));
    if (checkerFile.isEmpty()) {
        qCWarning(NOTIFIER) << "Couldn't find the releasechecker"
                            << checkerFile
                            << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        return;
    }
    qCDebug(NOTIFIER) << "Running releasechecker";
    m_checkerProcess = new QProcess(this);
    m_checkerProcess->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // Force utf-8. In case the system has bogus encoding configured we'll still
    // be able to properly decode.
    env.insert("PYTHONIOENCODING", "utf-8");
    if (m_dbus->useDevel()) {
        env.insert("USE_DEVEL", "1");
    }
    m_checkerProcess->setProcessEnvironment(env);
    connect(m_checkerProcess, QOverload<int>::of(&QProcess::finished),
            this, &DistroReleaseNotifier::checkReleaseUpgradeFinished);
    m_checkerProcess->start(QStringLiteral("/usr/bin/python3"), QStringList() << checkerFile);
}

void DistroReleaseNotifier::checkReleaseUpgradeFinished(int exitStatus)
{
    // close old notification first
    if (m_notification != nullptr) {
        m_notification->close();
        m_notification->deleteLater();
        m_notification = nullptr;
    }

    auto process = m_checkerProcess;
    m_checkerProcess->deleteLater();
    m_checkerProcess = nullptr;

    if (exitStatus != 0) {
        if (exitStatus != 32) { // 32 is special exit on no new release
            qCWarning(NOTIFIER()) << "Failed to run releasechecker";
        } else {
            qCDebug(NOTIFIER()) << "No new release found";
        }
        return;
    }

    const QByteArray checkerOutput = process->readAllStandardOutput();
    qCDebug(NOTIFIER) << checkerOutput;
    auto document = QJsonDocument::fromJson(checkerOutput);
    Q_ASSERT(document.isObject());
    auto map = document.toVariant().toMap();
    auto flavor = map.value(QStringLiteral("flavor")).toString();
    auto newDist = map.value(QStringLiteral("new_dist_version")).toString();

    auto name = NAME_FROM_FLAVOR ? flavor : OSRelease().name;
    const QString label = QString("%1 %2").arg(name, newDist);

    m_notification = new KNotification(QLatin1String("notification"),
                                       KNotification::Persistent | KNotification::DefaultEvent,
                                       this);
    m_notification->setIconName(QStringLiteral("system-software-update"));
    m_notification->setActions(QStringList{QLatin1String("Upgrade")});
    m_notification->setTitle(i18n("Upgrade available"));
    m_notification->setText(i18n("New version: %1", label));
    connect(m_notification, &KNotification::action1Activated,
            this, &DistroReleaseNotifier::releaseUpgradeActivated);
    m_notification->sendEvent();

}

void DistroReleaseNotifier::releaseUpgradeActivated()
{
    // pkexec is being difficult. It will refuse to auth a startDetached service
    // because it won't have a parent and parentless commands are not allowed
    // to auth.
    // Instead hold on to the process.
    // For future reference: another approach is to sh -c and hold
    // do-release-upgrade as a fork of that sh.
    auto process = new QProcess(this);
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(process, QOverload<int>::of(&QProcess::finished),
            this, [process](){ process->deleteLater(); });
    auto args = QStringList({
                                QStringLiteral("-m"), QStringLiteral("desktop"),
                                QStringLiteral("-f"), QStringLiteral("DistUpgradeViewKDE")
                            });
    if (m_dbus->useDevel()) {
        args << "--devel-release";
    }
    process->start(QStringLiteral("do-release-upgrade"), args);
}
