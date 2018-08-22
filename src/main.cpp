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
#include "distroreleasenotifierwindow.h"

// KF headers
#include <KAboutData>
#include <KLocalizedString>

// Qt headers
#include <QCoreApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);

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

    DistroReleaseNotifier *mainObject = new DistroReleaseNotifier;
    Q_UNUSED(mainObject);

    return application.exec();
}
