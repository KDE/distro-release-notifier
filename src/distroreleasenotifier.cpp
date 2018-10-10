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

#include <QDate>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

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
    networkTimer->setInterval(10 * 1000);
    connect(networkTimer, &QTimer::timeout, this, &DistroReleaseNotifier::releaseUpgradeCheck);
    networkTimer->start();

    auto dailyTimer = new QTimer(this);
    dailyTimer->setInterval(24 * 60 * 60 * 1000); // refresh once every day
    connect(dailyTimer, &QTimer::timeout,
            this, &DistroReleaseNotifier::forceCheck);
    dailyTimer->start();

    auto networkNotifier = NetworkManager::notifier();
    connect(networkNotifier, &NetworkManager::Notifier::connectivityChanged,
            [networkTimer](NetworkManager::Connectivity connectivity) {
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
    m_version = map.value(QStringLiteral("new_dist_version")).toString();
    m_name = NAME_FROM_FLAVOR ? flavor : OSRelease().name;

    // Download eol notification
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &DistroReleaseNotifier::replyFinished);

    auto request = QNetworkRequest(QUrl("https://releases.neon.kde.org/eol.json"));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    manager->get(request);
}

/*
 * Parses the eol.json file which is in a JSON hash of format release_version: eol_date
 * e.g. {"16.04": "2018-10-02"}
 */
void DistroReleaseNotifier::replyFinished(QNetworkReply *reply)
{
    qCDebug(NOTIFIER) << reply->error();
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(NOTIFIER) << reply->errorString();
    }
    const QString versionId = OSRelease().versionId;
    const QByteArray eolOutput = reply->readAll();
    const auto document = QJsonDocument::fromJson(eolOutput);
    if (!document.isObject()) {
        qCWarning(NOTIFIER) << "EOL reply failed to parse as document" << eolOutput;
        m_notifier->show(m_name, m_version, QDate());
        return;
    }
    const auto map = document.toVariant().toMap();
    auto dateString = map.value(versionId).toString();
    if (qEnvironmentVariableIsSet("MOCK_RELEASE")) {
        // If this is a mock we'll construct the date string artifically.
        // Otherwise we'd have to run a server-side generator which is a bit
        // more tricky and detatches the code so if the format changes we may
        // easily forget.
        if (qEnvironmentVariableIsSet("MOCK_EOL")) {
            // already eol
            dateString = QDate::currentDate().addDays(-1).toString("yyyy-MM-dd");
        } else {
            // eol in 3 days
            dateString = QDate::currentDate().addDays(3).toString("yyyy-MM-dd");
        }
    }
    qCDebug(NOTIFIER) << "versionId:" << versionId;
    qCDebug(NOTIFIER) << "dateString" << dateString;
    m_notifier->show(m_name, m_version,
                     QDate::fromString(dateString, Qt::ISODate));
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
