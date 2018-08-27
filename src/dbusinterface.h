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

#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>

class DBusInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.DistroReleaseNotifier")
    Q_PROPERTY(bool UseDevel READ useDevel WRITE setUseDevel NOTIFY useDevelChanged)
public:
    DBusInterface(QObject *parent = nullptr);
    ~DBusInterface();

    bool useDevel() const;
    void setUseDevel(bool use);

    Q_SCRIPTABLE void Poll();

signals:
    void useDevelChanged();
    // Emitted when Poll is called to notify of the reuqest
    void pollingRequested();

private:
    bool m_useDevel;
};

#endif // DBUSINTERFACE_H
