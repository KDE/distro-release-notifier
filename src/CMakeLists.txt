if (${INSTALL_PREVIEW_UPGRADE})
    add_subdirectory(preview)
endif()

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(distroreleasenotifier_SRCS
    main.cpp
    dbusinterface.cpp
    distroreleasenotifier.cpp
    notifier.cpp
    OSRelease.cpp
    upgraderwatcher.cpp
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

add_executable(distro-release-notifier ${distroreleasenotifier_SRCS})

target_link_libraries(distro-release-notifier
    KF5::CoreAddons
    KF5::I18n
    KF5::NetworkManagerQt
    KF5::Notifications
    Qt5::Network
    Qt5::Widgets
)

install(TARGETS distro-release-notifier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
