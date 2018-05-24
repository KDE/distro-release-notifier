cmake_minimum_required(VERSION 3.0)

project(distroreleasenotifier)

set(QT_MIN_VERSION "5.9.0")
set(KF_MIN_VERSION "5.35.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Notifications
    KIO
)

add_subdirectory(src)

install( FILES do-release-upgrade releasechecker DESTINATION ${DATA_INSTALL_DIR}/distro-release-notifier
        PERMISSIONS
        OWNER_EXECUTE OWNER_READ OWNER_WRITE
        GROUP_EXECUTE GROUP_READ
        WORLD_EXECUTE WORLD_READ
    )

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
