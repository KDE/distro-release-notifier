set( distroreleasenotifier_SRCS
    main.cpp
    distroreleasenotifierwindow.cpp
)

ki18n_wrap_ui(distroreleasenotifier_SRCS distroreleasenotifier.ui)

add_executable( distroreleasenotifier ${distroreleasenotifier_SRCS} )

target_link_libraries( distroreleasenotifier
    KF5::CoreAddons
    KF5::I18n
    Qt5::Widgets
)

install( TARGETS distroreleasenotifier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS org.example.distroreleasenotifier.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install( FILES org.example.distroreleasenotifier.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
