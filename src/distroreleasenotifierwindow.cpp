/*
 Copyright 2018 Jonathan Riddell <jr@jriddell.org>

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

#include "distroreleasenotifierwindow.h"

#include <KNotification>

#include <QProcess>
#include <QStandardPaths>
#include <QDebug>
#include <QTextCodec>
#include <QTimer>

distroReleaseNotifier::distroReleaseNotifier()
    : QObject()
    , m_notification(nullptr)
{
    // check after 10 seconds
    QTimer::singleShot(10 * 1000, this, &distroReleaseNotifier::releaseUpgradeCheck);

    QTimer *regularCheck = new QTimer(this);
    regularCheck->setInterval(24 * 60 * 60 * 1000); //refresh once every day
    connect(regularCheck, &QTimer::timeout, this, &distroReleaseNotifier::releaseUpgradeCheck);
    regularCheck->start();
}

distroReleaseNotifier::~distroReleaseNotifier()
{
}

void distroReleaseNotifier::releaseUpgradeCheck()
{
    QString checkerFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("distro-release-notifier/releasechecker"));
    if (checkerFile.isEmpty()) {
        qWarning() << "Couldn't find the releasechecker" << checkerFile << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        return;
    }
    qDebug() << "Running releasechecker";
    m_checkerProcess = new QProcess(this);
    connect(m_checkerProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &distroReleaseNotifier::checkReleaseUpgradeFinished);
    m_checkerProcess->start(QStringLiteral("/usr/bin/python3"), QStringList() << checkerFile);
}

void distroReleaseNotifier::checkReleaseUpgradeFinished(int exitStatus)
{
    // close old notification first
    if (m_notification != nullptr) {
        m_notification->close();
        m_notification->deleteLater();
    }
    m_notification = nullptr;
    if (exitStatus == 0) {
        QByteArray checkerOutput = m_checkerProcess->readAllStandardOutput();
        qDebug() << checkerOutput;
        m_notification = new KNotification(QLatin1String("notification"), KNotification::Persistent | KNotification::DefaultEvent);
        m_notification->setIconName(QStringLiteral("system-software-update"));
        m_notification->setActions(QStringList{QLatin1String("Upgrade")});
        m_notification->setTitle(i18n("Upgrade available"));
        m_notification->setText(i18n("New version: %1", QTextCodec::codecForMib(106)->toUnicode(checkerOutput)));
        connect(m_notification, &KNotification::action1Activated, this, &distroReleaseNotifier::releaseUpgradeActivated);
        m_notification->sendEvent();
    }

    m_checkerProcess->deleteLater();
    m_checkerProcess = nullptr;
}

void distroReleaseNotifier::releaseUpgradeActivated()
{
    QString releaseUpgradeExe = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("distro-release-notifier/do-release-upgrade"));
    if (releaseUpgradeExe.isEmpty()) {
        qWarning() << "Couldn't find the do-release-upgrade script " << releaseUpgradeExe << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        return;
    }
    QProcess::startDetached(releaseUpgradeExe);
}
