/*
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

#include "screensaverinhibitor.h"

#include <KLocalizedString>
#include <KNotificationRestrictions>

#include "upgraderwatcher.h"

ScreenSaverInhibitor::ScreenSaverInhibitor(QObject *parent)
    : QObject(parent)
    , m_restriction(nullptr)
{
    auto watcher = UpgraderWatcher::self();
    connect(watcher, &UpgraderWatcher::upgraderRunning, [this]() {
        m_restriction = new KNotificationRestrictions(KNotificationRestrictions::ScreenSaver,
                                                      i18nc("Screensaver inhibition reason", "Upgrading Operating System"));
    });
    connect(watcher, &UpgraderWatcher::upgraderNotRunning, [this]() {
        delete m_restriction;
        m_restriction = nullptr;
    });
}
