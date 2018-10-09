/*
 Copyright 2018 Jonathan Riddell <jr@jriddell.org>
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

void Notifier::show(const QString &name, const QString &version, const bool eol, const QDate &eolDate)
{
    // Delayed init. Otherwise we have the KSNI up when no upgrades are available.
    init();

    const QString label = i18nc("KDE neon on 18.04", "%1 on %2", name, version);

    const QString title = i18n("Upgrade available");
    QString text = i18n("%1.", label);
    if (eol && eolDate > QDate::currentDate()) {
        text.append(i18n("\nThis version will stop receiving updates and security fixes in %1 days.", -eolDate.daysTo(QDate::currentDate())));
    } else if (eol) {
        text.append(i18nc("Warning notice with emoji", "\nThis version will no longer receive updates or security fixes from KDE neon.\n%1 Upgrade Now!", "☢"));
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
    auto notification = new KNotification(QLatin1Literal("notification"),
                                          KNotification::Persistent | KNotification::DefaultEvent,
                                          this);
    notification->setIconName(icon);
    notification->setActions(QStringList{QLatin1Literal("Upgrade")});
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
    connect(watcher, &UpgraderWatcher::upgraderRunning, [this]() {
        delete m_notifier;
        m_notifier = nullptr;
    });
}


