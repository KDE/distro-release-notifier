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

#include "upgraderwatcher.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>

#include "debug.h"

UpgraderWatcher *UpgraderWatcher::self()
{
    static UpgraderWatcher watcher;
    return &watcher;
}

UpgraderWatcher::UpgraderWatcher(QObject *parent)
    : QObject(parent)
{
    auto watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::systemBus());
    watcher->addWatchedService("com.ubuntu.ReleaseUpgrader");
    watcher->addWatchedService("com.ubuntu.ReleaseUpgrader.KDE");
    connect(watcher, &QDBusServiceWatcher::serviceRegistered,
            this, &UpgraderWatcher::upgraderRunning);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &UpgraderWatcher::upgraderNotRunning);
    if (!NOTIFIER().isDebugEnabled()) {
        return;
    }
    connect(watcher, &QDBusServiceWatcher::serviceRegistered,
            [](const QString &service) {
        qCDebug(NOTIFIER) << "Service registered" << service;
    });
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered,
            [](const QString &service) {
        qCDebug(NOTIFIER) << "Service unregistered" << service;
    });
}
