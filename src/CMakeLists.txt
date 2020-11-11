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

qt5_generate_dbus_interface(
    dbusinterface.h
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.DistroReleaseNotifier.xml
    OPTIONS -a
)
qt5_add_dbus_adaptor(distroreleasenotifier_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.DistroReleaseNotifier.xml dbusinterface.h DBusInterface)

add_library(distroreleasenotifier MODULE ${distroreleasenotifier_SRCS})

target_link_libraries(distroreleasenotifier
    KF5::DBusAddons
    KF5::CoreAddons
    KF5::I18n
    KF5::NetworkManagerQt
    KF5::Notifications
    KF5::WidgetsAddons
    Qt5::Network
)

install(TARGETS distroreleasenotifier DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)
kcoreaddons_desktop_to_json(distroreleasenotifier distro-release-notifier.desktop)
