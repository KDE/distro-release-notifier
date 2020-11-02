/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UPGRADERPROCESS_H
#define UPGRADERPROCESS_H

#include <QObject>

/**
 * Runs the upgrader. Possibly displays UI if the upgrader craps out unexpectedly during
 * startup.
 */
class UpgraderProcess : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    void setUseDevel(bool useDevel);
    void run();

signals:
    // Either the process finished or it registered on dbus.
    // Notifier should consider this launch concluded.
    void notPending();

private slots:
    void onUnexpectedFinish(int code);

private:
    bool m_useDevel = false;
    bool m_waiting = true; // only true while we wait for the proc to fail
    QString m_output;
};

#endif // UPGRADERPROCESS_H
