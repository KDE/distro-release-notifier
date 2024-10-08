# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2018 Jonathan Riddell <jr@jriddell.org>
# SPDX-FileCopyrightText: 2018 Rik Mills <rikmills@kubuntu.org>
# SPDX-FileCopyrightText: 2018-2020 Harald Sitter <sitter@kde.org>

cmake_minimum_required(VERSION 3.0)

project(distroreleasenotifier)

set(QT_MIN_VERSION "6.2.0")
set(KF6_MIN_VERSION "6.2.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    NetworkManagerQt
    Notifications
    WidgetsAddons
    DBusAddons
    KIO
    StatusNotifierItem
)

# update flavor is derived from the installed metapackage(s) by pkg.split('-')[-1].captialize
# it is therefore less accurate and good looking than /etc/os-release
option(NAME_FROM_FLAVOR
    "Get distro name from update flavor [default is from /etc/os-release]"
    OFF)
option(INSTALL_PREVIEW_UPGRADE
    "Whether to install a tiny helper binary to make pre-release upgrades more accessible. Also set PREVIEW_UPGRADE_NAME."
    OFF)

set(PREVIEW_UPGRADE_NAME "distro-preview-upgrade"
    CACHE STRING "Name to install the upgrade preview helper as.")

add_subdirectory(src)

install(FILES releasechecker DESTINATION ${KDE_INSTALL_DATADIR}/distro-release-notifier
        PERMISSIONS
        OWNER_EXECUTE OWNER_READ OWNER_WRITE
        GROUP_EXECUTE GROUP_READ
        WORLD_EXECUTE WORLD_READ
)

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
