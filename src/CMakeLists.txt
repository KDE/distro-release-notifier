# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2018-2020 Jonathan Riddell <jr@jriddell.org>
# SPDX-FileCopyrightText: 2018-2020 Harald Sitter <sitter@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"distro-release-notifier\")

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

if (${INSTALL_PREVIEW_UPGRADE})
    add_subdirectory(preview)
endif()

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(distroreleasenotifier_SRCS
    module.cpp
    dbusinterface.cpp
    distroreleasenotifier.cpp
    notifier.cpp
    screensaverinhibitor.cpp
    upgraderwatcher.cpp
    upgraderprocess.cpp
)

ecm_qt_declare_logging_category(distroreleasenotifier_SRCS
    HEADER debug.h
    IDENTIFIER NOTIFIER
    CATEGORY_NAME org.kde.distro-release-notifier)

kcoreaddons_add_plugin(distroreleasenotifier SOURCES ${distroreleasenotifier_SRCS} INSTALL_NAMESPACE "kf6/kded")

target_link_libraries(distroreleasenotifier
    KF6::DBusAddons
    KF6::CoreAddons
    KF6::I18n
    KF6::NetworkManagerQt
    KF6::Notifications
    KF6::WidgetsAddons
    Qt6::Network
)
