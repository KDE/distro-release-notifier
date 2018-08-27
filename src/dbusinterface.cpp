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

#include "dbusinterface.h"

#include "debug.h"
#include "distroreleasenotifieradaptor.h"

DBusInterface::DBusInterface(QObject *parent)
    : QObject(parent)
    , m_useDevel(false)
{
    new DistroReleaseNotifierAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    Q_ASSERT(dbus.registerObject("/", this));
    Q_ASSERT(dbus.registerService("org.kde.DistroReleaseNotifier"));
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
