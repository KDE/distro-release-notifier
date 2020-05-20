/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "dbusinterface.h"

#include <QCoreApplication>

#include "debug.h"
#include "distroreleasenotifieradaptor.h"

DBusInterface::DBusInterface(QObject *parent)
    : QObject(parent)
    , m_useDevel(qEnvironmentVariableIsSet("DEVEL_RELEASE"))
{
    new DistroReleaseNotifierAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    bool objectRet = dbus.registerObject("/", this);
    Q_ASSERT_X(objectRet, Q_FUNC_INFO, "Failed to register / on org.kde.DistroReleaseNotifier");
    bool serviceRet = dbus.registerService("org.kde.DistroReleaseNotifier");
    Q_ASSERT_X(serviceRet, Q_FUNC_INFO, "Failed to register org.kde.DistroReleaseNotifier");
    if (!objectRet || !serviceRet) {
        // If this build isn't qFatal, manually exit on errors.
        // We'd not get here if it was fatal!
        qCWarning(NOTIFIER, "Failed to register org.kde.DistroReleaseNotifier");
        // Exit directly, not through qApp as we haven't even begone execution
        // by the time this ctor runs.
        exit(1);
    }
}

DBusInterface::~DBusInterface()
{
}

bool DBusInterface::useDevel() const
{
    return m_useDevel;
}

void DBusInterface::setUseDevel(bool use)
{
    if (m_useDevel == use) {
        return;
    }
    m_useDevel = use;
    emit useDevelChanged();
}

void DBusInterface::Poll()
{
    emit pollingRequested();
}
