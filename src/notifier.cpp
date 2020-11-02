/*
    SPDX-FileCopyrightText: 2018 Jonathan Riddell <jr@jriddell.org>
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "notifier.h"

#include <KLocalizedString>
#include <KNotification>
#include <KStatusNotifierItem>

#include <QDate>

#include "upgraderwatcher.h"

Notifier::Notifier(QObject *parent)
    : QObject(parent)
    , m_notifier(nullptr)
{
}

void Notifier::show(const QString &name, const QString &version, const QDate &eolDate)
{
    // Delayed init. Otherwise we have the KSNI up when no upgrades are available.
    init();

    const QString label = i18nc("KDE neon on 18.04", "%1 on %2", name, version);

    const QString title = i18n("Upgrade available");
    QString text = i18n("%1.", label);
    if (eolDate.isValid() && eolDate > QDate::currentDate()) {
        text.append(i18np("\nThis version will stop receiving updates and security fixes in 1 day.",
                          "\nThis version will stop receiving updates and security fixes in %1 days.",
                          -eolDate.daysTo(QDate::currentDate())));
    } else if (!eolDate.isNull()) {
        text.append(
            i18nc("Warning notice with emoji",
                  "\nThis version will no longer receive updates or security fixes from KDE neon.\n%1 Upgrade Now!",
                  QStringLiteral("☢")));
    }
    const QString icon = QStringLiteral("system-software-update");

    m_notifier->setIconByName(icon);
    m_notifier->setToolTipIconByName(icon);
    m_notifier->setToolTipTitle(title);
    m_notifier->setToolTipSubTitle(text);
    m_notifier->setStatus(KStatusNotifierItem::Active);
    m_notifier->setCategory(KStatusNotifierItem::SystemServices);
    m_notifier->setStandardActionsEnabled(false);

    // This replaces a potentially pre-existing notification. Notifications
    // are auto-delted, so we need to do no house keeping here. This will
    // automatically replace the previous notification.
    auto notification = new KNotification(QStringLiteral("notification"),
                                          KNotification::Persistent | KNotification::DefaultEvent,
                                          this);
    notification->setIconName(icon);
    notification->setActions(QStringList{QStringLiteral("Upgrade")});
    notification->setTitle(title);
    notification->setText(text);
    connect(notification, &KNotification::action1Activated,
            this, &Notifier::activateRequested);
    notification->sendEvent();
}

void Notifier::init()
{
    if (m_notifier) {
        return;
    }

    m_notifier = new KStatusNotifierItem(this);
    connect(m_notifier, &KStatusNotifierItem::activateRequested,
            this, &Notifier::activateRequested);

    // Watch upgrader running and delete the KSNI while it is up to prevent
    // the user from triggering an upgrade while upgrading.
    auto watcher = UpgraderWatcher::self();
    connect(watcher, &UpgraderWatcher::upgraderRunning, this, [this]() {
        delete m_notifier;
        m_notifier = nullptr;
    });
}


