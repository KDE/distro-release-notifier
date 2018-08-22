configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(distroreleasenotifier_SRCS
    main.cpp
    distroreleasenotifier.cpp
    OSRelease.cpp
)

ecm_qt_declare_logging_category(distroreleasenotifier_SRCS
    HEADER debug.h
    IDENTIFIER NOTIFIER
    CATEGORY_NAME org.kde.distro-release-notifier)

add_executable(distro-release-notifier ${distroreleasenotifier_SRCS})

target_link_libraries(distro-release-notifier
    KF5::CoreAddons
    KF5::I18n
    KF5::Notifications
    Qt5::Widgets
)

install(TARGETS distro-release-notifier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
