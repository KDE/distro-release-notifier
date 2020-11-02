/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    // Emitted when Poll is called to notify of the request
    void pollingRequested();

private:
    bool m_useDevel;
};

#endif // DBUSINTERFACE_H
