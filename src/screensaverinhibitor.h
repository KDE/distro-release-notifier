/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SCREENSAVERINHIBITOR_H
#define SCREENSAVERINHIBITOR_H

#include <QObject>

class KNotificationRestrictions;

class ScreenSaverInhibitor : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSaverInhibitor(QObject *parent = nullptr);

private:
    KNotificationRestrictions *m_restriction = nullptr;
};

#endif // SCREENSAVERINHIBITOR_H
