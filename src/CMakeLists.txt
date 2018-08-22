set(distroreleasenotifier_SRCS
    main.cpp
    distroreleasenotifierwindow.cpp
)

ki18n_wrap_ui(distroreleasenotifier_SRCS distroreleasenotifier.ui)

add_executable(distro-release-notifier ${distroreleasenotifier_SRCS})

target_link_libraries(distro-release-notifier
    KF5::CoreAddons
    KF5::I18n
    KF5::Notifications
    Qt5::Widgets
)

install(TARGETS distro-release-notifier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
