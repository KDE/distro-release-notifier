/*
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include <QGuiApplication>
#include <QDBusInterface>
#include <QDBusReply>

#include <KLocalizedString>

#include "screensaverinhibitor.h"
#include "upgraderwatcher.h"

ScreenSaverInhibitor::ScreenSaverInhibitor(QObject *parent)
    : QObject(parent)
{
    auto watcher = UpgraderWatcher::self();
    connect(watcher, &UpgraderWatcher::upgraderRunning, this, [this]() {
            QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                              QStringLiteral("/ScreenSaver"),
                                                              QStringLiteral("org.freedesktop.ScreenSaver"),
                                                              QStringLiteral("Inhibit"));
        message << QGuiApplication::desktopFileName();
        message << i18nc("Screensaver inhibition reason", "Upgrading Operating System");
        QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);
        if (reply.isValid()) {
            m_screensaverDisableCookie = reply.value();
        }
    });
    connect(watcher, &UpgraderWatcher::upgraderNotRunning, this, [this]() {
         if (m_screensaverDisableCookie.has_value()) {
            QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                                  QStringLiteral("/ScreenSaver"),
                                                                  QStringLiteral("org.freedesktop.ScreenSaver"),
                                                                  QStringLiteral("UnInhibit"));
            (m_screensaverDisableCookie.value());
            m_screensaverDisableCookie = {};
            QDBusConnection::sessionBus().send(message);
        }
    });
}

    /*
    auto watcher = UpgraderWatcher::self();
    connect(watcher, &UpgraderWatcher::upgraderRunning, this, [this]() {
        m_restriction = new KNotificationRestrictions(KNotificationRestrictions::ScreenSaver,
                                                      i18nc("Screensaver inhibition reason", "Upgrading Operating System"));

    connect(watcher, &UpgraderWatcher::upgraderNotRunning, this, [this]() {
        delete m_restriction;
        m_restriction = nullptr;
    });
    */

