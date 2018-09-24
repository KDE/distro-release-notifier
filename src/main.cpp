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

// application header
#include "distroreleasenotifier.h"
#include "screensaverinhibitor.h"

// KF headers
#include <KAboutData>
#include <KLocalizedString>

// Qt headers
#include <QApplication>
#include <QCommandLineParser>
#include <QSessionManager>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    // SNI is considered a window. We don't want to close when the SNI
    // gets dropped. We run all the time to check for releases regularly.
    application.setQuitOnLastWindowClosed(false);

    KLocalizedString::setApplicationDomain("distro-release-notifier");

    KAboutData aboutData( QStringLiteral("distro-release-notifier"),
                          i18n("distro-release-notifier"),
                          QStringLiteral("0.1"),
                          i18n("Checks for new Ubuntu releases and notifies"),
                          KAboutLicense::GPL,
                          i18n("Copyright 2018 Jonathan Riddell <jr@jriddell.org>"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(application);
    aboutData.processCommandLine(&parser);

    // Disable session management for the notifier. We do not ever want to get
    // restored. The notifier gets autostarted, or not at all.
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&application, &QGuiApplication::commitDataRequest,
                     disableSessionManagement);
    QObject::connect(&application, &QGuiApplication::saveStateRequest,
                     disableSessionManagement);

    if (application.isSessionRestored()) {
        // Do not ever restore it from a previous session. Session restoration
        // wasn't always disabled, so make sure restoration attempts are discard
        // for sessions which have older notifiers in their restoration list
        // still.
        return 0;
    }

    // NB: we are not using kdbusservice because it's largely useless.
    // This service is only ever started via autostart and if not asserting
    // that registering the service is good enough for our purposes.
    DistroReleaseNotifier mainObject;

    // Do not lazy init this on setting up the notification. It makes a 5kb
    // difference if we have the dbusservice under monitoring or not.
    ScreenSaverInhibitor inhibitor;

    return application.exec();
}
