# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>

install(FILES distro-preview-upgrade
    RENAME ${PREVIEW_UPGRADE_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS
    OWNER_EXECUTE OWNER_READ OWNER_WRITE
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
