/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UPGRADERWATCHER_H
#define UPGRADERWATCHER_H

#include <QObject>

class UpgraderWatcher : public QObject
{
    Q_OBJECT
public:
    static UpgraderWatcher *self();

signals:
    void upgraderRunning();
    void upgraderNotRunning();

private:
    explicit UpgraderWatcher(QObject *parent = nullptr);
};

#endif // UPGRADERWATCHER_H
