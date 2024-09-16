/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "screensaverinhibitor.h"

#include <KLocalizedString>
// TODO This class seems to have vanished, see e.g. kdeconnect-kde plugins/screensaver-inhibit
// for how to use the freedesktop interface
//#include <KNotificationRestrictions>

#include "upgraderwatcher.h"

ScreenSaverInhibitor::ScreenSaverInhibitor(QObject *parent)
    : QObject(parent)
{
    /*
    auto watcher = UpgraderWatcher::self();
    connect(watcher, &UpgraderWatcher::upgraderRunning, this, [this]() {
        m_restriction = new KNotificationRestrictions(KNotificationRestrictions::ScreenSaver,
                                                      i18nc("Screensaver inhibition reason", "Upgrading Operating System"));
    });
    connect(watcher, &UpgraderWatcher::upgraderNotRunning, this, [this]() {
        delete m_restriction;
        m_restriction = nullptr;
    });
    */
}
