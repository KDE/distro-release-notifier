/*
    SPDX-FileCopyrightText: 2018 Jonathan Riddell <jr@jriddell.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
