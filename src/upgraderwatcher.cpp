/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    watcher->addWatchedService(QStringLiteral("com.ubuntu.ReleaseUpgrader"));
    watcher->addWatchedService(QStringLiteral("com.ubuntu.ReleaseUpgrader.KDE"));
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
