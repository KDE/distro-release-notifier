/*
    SPDX-FileCopyrightText: 2018-2020 Harald Sitter <sitter@kde.org>
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
    const bool objectRet = dbus.registerObject(QStringLiteral("/org/kde/DistroReleaseNotifier"), this);
    const bool serviceRet = dbus.registerService(QStringLiteral("org.kde.DistroReleaseNotifier"));
    if (!objectRet || !serviceRet) {
        // If this build isn't qFatal, manually exit on errors.
        // We'd not get here if it was fatal!
        qCritical() << "Failed to register org.kde.DistroReleaseNotifier" << objectRet << serviceRet;
    }
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
