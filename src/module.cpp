/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <KPluginFactory>
#include <KDEDModule>

#include "distroreleasenotifier.h"
#include "screensaverinhibitor.h"

class Module : public KDEDModule
{
    Q_OBJECT
public:
    explicit Module(QObject *parent, const QVariantList &args)
        : KDEDModule(parent)
    {
        Q_UNUSED(args);
    }

private:
    // NB: we are not using kdbusservice because it's largely useless.
    // This service is only ever started via autostart and if not asserting
    // that registering the service is good enough for our purposes.
    DistroReleaseNotifier mainObject;

    // Do not lazy init this on setting up the notification. It makes a 5kb
    // difference if we have the dbusservice under monitoring or not.
    ScreenSaverInhibitor inhibitor;
};

K_PLUGIN_FACTORY_WITH_JSON(DistroReleaseNotifierModuleFactory,
                           "distro-release-notifier.json",
                           registerPlugin<Module>();)

#include "module.moc"
