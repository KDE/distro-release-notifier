/*
    SPDX-FileCopyrightText: 2018-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "upgraderprocess.h"

#include <KLocalizedString>
#include <KOSRelease>
#include <KTitleWidget>

#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QProcess>
#include <QTextEdit>
#include <QVBoxLayout>

#include "debug.h"
#include "upgraderwatcher.h"

void UpgraderProcess::setUseDevel(bool useDevel)
{
    m_useDevel = useDevel;
}

void UpgraderProcess::run()
{
    auto process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](){
        m_waiting = false;
        Q_EMIT notPending();
        deleteLater();
    });

    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::readyReadStandardOutput,
            this, [this, process]() {
        if (!NOTIFIER().isDebugEnabled() && !m_waiting) {
            return;
        }

        const QString newOutput = QString::fromUtf8(process->readAllStandardOutput());
        // route this through format string so newlines are preserved
        qCDebug(NOTIFIER, "do-release-upgrader: %s\n", newOutput.toUtf8().constData());
        if (m_waiting) {
            m_output += newOutput;
        }
    });

    // Monitor dbus for the higher level UIs to appear.
    // If the proc finishes before the dbus magic happens it likely crapped out in early
    // checks which have zero UI backing, meaning we need to display stdout in a dialog.
    auto unexpectedConnection = connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                                        this, &UpgraderProcess::onUnexpectedFinish);
    connect(UpgraderWatcher::self(), &UpgraderWatcher::upgraderRunning,
            this, [this, unexpectedConnection]() {
        m_waiting = false;
        Q_EMIT notPending();
        disconnect(unexpectedConnection);
    });

    // pkexec is being difficult. It will refuse to auth a startDetached service
    // because it won't have a parent and parentless commands are not allowed
    // to auth.
    // Instead hold on to the process.
    // For future reference: another approach is to sh -c and hold
    // do-release-upgrade as a fork of that sh.
    auto args = QStringList({
                                QStringLiteral("-m"), QStringLiteral("desktop"),
                                QStringLiteral("-f"), QStringLiteral("DistUpgradeViewKDE")
                            });
    if (m_useDevel) {
        args << QStringLiteral("--devel-release");
    }

    qCDebug(NOTIFIER) << "Starting do-release-upgrade";
    process->start(QStringLiteral("do-release-upgrade"), args);
}

void UpgraderProcess::onUnexpectedFinish(int code)
{
    // If the process finished within some seconds something is probably broken
    // in the bootstrap. Display the output.
    // Notably the upgrader exit(1) when it detects pending updates on apt and will only
    // inform the user through stdout. It's very crappy UX.

    if (code == 0) {
        qCWarning(NOTIFIER) << "Unexpected early exit but ignoring because it was code 0!";
        return;
    }

    qCDebug(NOTIFIER) << "Probable failure" << code;

    QDialog dialog;
    dialog.setWindowIcon(QIcon::fromTheme(QStringLiteral("system-software-update")));
    dialog.setWindowTitle(i18nc("@title/window upgrade failure dialog", "Upgrade Failed"));
    auto layout = new QVBoxLayout;
    dialog.setLayout(layout);

    auto title = new KTitleWidget(&dialog);
    title->setText(i18nc("@title title widget above process output",
                         "Upgrade failed with the following output:"));
    layout->addWidget(title);

    auto editor = new QTextEdit(&dialog);
    editor->setReadOnly(true);
    editor->setText(m_output.replace(QStringLiteral("ubuntu"), QStringLiteral("neon"), Qt::CaseInsensitive));
    layout->addWidget(editor);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}
