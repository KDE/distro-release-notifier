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

void Notifier::show(const QString &name, const QString &version, const QDate &eolDate)
{
    // Delayed init. Otherwise we have the KSNI up when no upgrades are available.
    init();

    const QString title = i18n("Upgrade available");
    QString text = i18nc("For example, 'KDE neon 18.04 is available.'", "%1 %2 is available.", name, version);
    if (eolDate.isValid() && eolDate > QDate::currentDate()) {
        text.append(i18np("\nYour device will stop receiving updates and security fixes in 1 day - please upgrade when possible.",
                          "\nYour device will stop receiving updates and security fixes in %1 days - please upgrade when possible.",
                          -eolDate.daysTo(QDate::currentDate())));
    } else if (!eolDate.isNull()) {
        text.append(
            i18n("\nYour device is no longer receiving updates or security fixes - please upgrade now."));
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
    auto upgradeAction = notification->addAction(i18n("Open in File Manager"));
    connect(upgradeAction, &KNotificationAction::activated, this, [this] {
                &Notifier::activateRequested;
    });
    notification->setTitle(title);
    notification->setText(text);
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


