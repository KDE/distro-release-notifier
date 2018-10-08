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

#include <NetworkManagerQt/Manager>

#include <QJsonDocument>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
#include <QNetworkReply>
#include <QDate>

#include "config.h"
#include "dbusinterface.h"
#include "debug.h"
#include "notifier.h"
#include "OSRelease.h"

DistroReleaseNotifier::DistroReleaseNotifier(QObject *parent)
    : QObject(parent)
    , m_dbus(new DBusInterface(this))
    , m_checkerProcess(nullptr)
    , m_notifier(new Notifier(this))
    , m_hasChecked(false)
{
    // check after 10 seconds
    auto networkTimer = new QTimer(this);
    networkTimer->setSingleShot(true);
//    networkTimer->setInterval(10 * 1000);
    networkTimer->setInterval(1 * 1000);
    connect(networkTimer, &QTimer::timeout, this, &DistroReleaseNotifier::releaseUpgradeCheck);
    networkTimer->start();

    auto dailyTimer = new QTimer(this);
    dailyTimer->setInterval(24 * 60 * 60 * 1000); // refresh once every day
    connect(dailyTimer, &QTimer::timeout,
            this, &DistroReleaseNotifier::forceCheck);
    dailyTimer->start();

    auto networkNotifier = NetworkManager::notifier();
    connect(networkNotifier, &NetworkManager::Notifier::connectivityChanged,
            [this, networkTimer](NetworkManager::Connectivity connectivity) {
        if (connectivity == NetworkManager::Connectivity::Full) {
            // (re)start the timer. The timer will make sure we collect up
            // multiple signals arriving in quick succession into a single
            // check.
            networkTimer->start();
        }
    });

    connect(m_dbus, &DBusInterface::useDevelChanged,
            this, &DistroReleaseNotifier::forceCheck);
    connect(m_dbus, &DBusInterface::pollingRequested,
            this, &DistroReleaseNotifier::forceCheck);

    connect(m_notifier, &Notifier::activateRequested,
            this, &DistroReleaseNotifier::releaseUpgradeActivated);
}

DistroReleaseNotifier::~DistroReleaseNotifier()
{
}

void DistroReleaseNotifier::releaseUpgradeCheck()
{
    if (m_hasChecked) {
        // Don't check again if we had a successful check again. We don't wanna
        // be spamming the user with the notification. This is reset eventually
        // by a timer to remind the user.
        return;
    }

    const QString checkerFile =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   QStringLiteral("distro-release-notifier/releasechecker"));
    if (checkerFile.isEmpty()) {
        qCWarning(NOTIFIER) << "Couldn't find the releasechecker"
                            << checkerFile
                            << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        return;
    }

    if (m_checkerProcess) {
        // Guard against multiple polls from dbus
        qCDebug(NOTIFIER) << "Check still running";
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

void DistroReleaseNotifier::checkReleaseUpgradeFinished(int exitCode)
{
    m_hasChecked = true;

    auto process = m_checkerProcess;
    m_checkerProcess->deleteLater();
    m_checkerProcess = nullptr;

    const QByteArray checkerOutput = process->readAllStandardOutput();

    // Make sure clearly invalid output doesn't get run through qjson at all.
    if (exitCode != 0 || checkerOutput.isEmpty()) {
        if (exitCode != 32) { // 32 is special exit on no new release
            qCWarning(NOTIFIER()) << "Failed to run releasechecker";
        } else {
            qCDebug(NOTIFIER()) << "No new release found";
        }
        return;
    }

    qCDebug(NOTIFIER) << checkerOutput;
    auto document = QJsonDocument::fromJson(checkerOutput);
    Q_ASSERT(document.isObject());
    auto map = document.toVariant().toMap();
    auto flavor = map.value(QStringLiteral("flavor")).toString();
    auto version = map.value(QStringLiteral("new_dist_version")).toString();

    auto name = NAME_FROM_FLAVOR ? flavor : OSRelease().name;

    //download eol notification
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replyFinished(QNetworkReply*)));

    m_EolRequestRunning = true;
    manager->get(QNetworkRequest(QUrl("http://embra.edinburghlinux.co.uk/~jr/tmp/eol.json")));
    while(m_EolRequestRunning) {
        QCoreApplication::processEvents();
    }
    
    m_notifier->show(name, version, QDate());
}

void DistroReleaseNotifier::replyFinished(QNetworkReply* reply) 
{
    //qCDebug(NOTIFIER) << "Finished";
    //qCDebug(NOTIFIER) << reply->readAll();
    QString versionId = OSRelease().versionId;
    auto document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        return;
    }
    auto map = document.toVariant().toMap();
    auto dateString = map.value(versionId).toString();
    QStringList dateStringPieces = dateString.split("-");
    if (!(dateStringPieces.length() == 3)) {
        return;
    }
    m_eolDate = new QDate(dateStringPieces[0].toInt(), dateStringPieces[1].toInt(), dateStringPieces[2].toInt());
    qCDebug(NOTIFIER) << "EOL" << m_eolDate;
    return;
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

void DistroReleaseNotifier::forceCheck()
{
    m_hasChecked = false;
    releaseUpgradeCheck();
}
