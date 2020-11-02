/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

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
