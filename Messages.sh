#! /usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2018 Jonathan Riddell <jr@jriddell.org>

$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/distro-release-notifier.pot
rm -f rc.cpp
