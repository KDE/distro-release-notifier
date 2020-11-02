/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <QObject>

class KStatusNotifierItem;

class Notifier : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    void show(const QString &name, const QString &version, const QDate &eolDate);

signals:
    void activateRequested();

private:
    void init();

    KStatusNotifierItem *m_notifier = nullptr;
};

#endif // NOTIFIER_H
