#include "ssh_collector.h"

#include <QStringBuilder>

SshCollector::SshCollector(QObject* parent)
    : QObject(parent)
{
    connect(&m_process, &QProcess::finished, this, &SshCollector::onFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &SshCollector::onErrorOccurred);
}

void SshCollector::collect(const inspector::ConnectionProfile& profile)
{
    if (busy()) {
        return;
    }

    m_active = true;
    emit busyChanged();

    m_process.setProgram(QStringLiteral("ssh"));
    m_process.setArguments(buildArgs(profile));
    m_process.setProcessChannelMode(QProcess::SeparateChannels);
    m_process.start();

    if (!m_process.waitForStarted(3000)) {
        onErrorOccurred(QProcess::FailedToStart);
        return;
    }

    m_process.write(buildRemoteScript(profile).toUtf8());
    m_process.closeWriteChannel();
}

QString SshCollector::commandPreview(const inspector::ConnectionProfile& profile) const
{
    QStringList parts;
    parts << QStringLiteral("ssh");
    parts << QStringLiteral("-o") << QStringLiteral("BatchMode=yes");
    parts << QStringLiteral("-o") << QStringLiteral("ConnectTimeout=10");
    parts << QStringLiteral("-o") << QStringLiteral("StrictHostKeyChecking=accept-new");

    if (!profile.identityFile.empty()) {
        parts << QStringLiteral("-i") << shellQuote(QString::fromStdString(profile.identityFile));
    }

    if (profile.port > 0 && profile.port != 22) {
        parts << QStringLiteral("-p") << QString::number(profile.port);
    }

    parts << QString::fromStdString(profile.user + "@" + profile.host);
    parts << QStringLiteral("'bash -s --'");
    return parts.join(' ');
}

bool SshCollector::busy() const
{
    return m_active || m_process.state() != QProcess::NotRunning;
}

void SshCollector::onFinished(const int exitCode, const QProcess::ExitStatus exitStatus)
{
    if (!m_active) {
        return;
    }

    m_active = false;
    emit busyChanged();

    auto stderrText = QString::fromUtf8(m_process.readAllStandardError());
    if (exitStatus != QProcess::NormalExit && stderrText.trimmed().isEmpty()) {
        stderrText = QStringLiteral("ssh process did not exit normally.");
    }

    emit finished(
        QString::fromUtf8(m_process.readAllStandardOutput()),
        stderrText,
        exitCode
    );
}

void SshCollector::onErrorOccurred(const QProcess::ProcessError error)
{
    if (!m_active) {
        return;
    }

    if (error == QProcess::Crashed && m_process.state() != QProcess::NotRunning) {
        return;
    }

    m_active = false;
    emit busyChanged();
    emit finished(QString(), m_process.errorString(), -1);
}

QString SshCollector::shellQuote(const QString& value)
{
    auto escaped = value;
    escaped.replace('\'', QStringLiteral("'\"'\"'"));
    return QStringLiteral("'") + escaped + QStringLiteral("'");
}

QStringList SshCollector::buildArgs(const inspector::ConnectionProfile& profile) const
{
    QStringList args;
    args << QStringLiteral("-o") << QStringLiteral("BatchMode=yes");
    args << QStringLiteral("-o") << QStringLiteral("ConnectTimeout=10");
    args << QStringLiteral("-o") << QStringLiteral("StrictHostKeyChecking=accept-new");

    if (!profile.identityFile.empty()) {
        args << QStringLiteral("-i") << QString::fromStdString(profile.identityFile);
    }

    if (profile.port > 0 && profile.port != 22) {
        args << QStringLiteral("-p") << QString::number(profile.port);
    }

    args << QString::fromStdString(profile.user + "@" + profile.host);
    args << QStringLiteral("bash -s --");
    return args;
}

QString SshCollector::buildRemoteScript(const inspector::ConnectionProfile& profile) const
{
    const auto dockerCommand = profile.useSudo ? QStringLiteral("sudo -n docker") : QStringLiteral("docker");
    const auto logosContainer = shellQuote(QString::fromStdString(profile.logosContainerName));
    const auto wakuContainer = shellQuote(QString::fromStdString(profile.wakuContainerName));
    const auto apiPort = QString::number(profile.apiPort);

    QStringList lines;
    lines
        << QStringLiteral("set -u")
        << QStringLiteral("section() { printf '__NODE_INSPECTOR_SECTION__:%s\\n' \"$1\"; }")
        << QStringLiteral("section_end() { printf '__NODE_INSPECTOR_END_SECTION__:%s\\n' \"$1\"; }")
        << QStringLiteral("")
        << QStringLiteral("section HOST_BASICS")
        << QStringLiteral("hostname 2>/dev/null || true")
        << QStringLiteral("uname -srmo 2>/dev/null || true")
        << QStringLiteral("uptime -p 2>/dev/null || uptime 2>/dev/null || true")
        << QStringLiteral("section_end HOST_BASICS")
        << QStringLiteral("")
        << QStringLiteral("section ROOT_DF")
        << QStringLiteral("df -B1 / 2>/dev/null || true")
        << QStringLiteral("section_end ROOT_DF")
        << QStringLiteral("")
        << QStringLiteral("section MEMORY")
        << QStringLiteral("free -b 2>/dev/null || true")
        << QStringLiteral("section_end MEMORY")
        << QStringLiteral("")
        << QStringLiteral("section DOCKER_SYSTEM_DF")
        << QString("%1 system df 2>/dev/null || true").arg(dockerCommand)
        << QStringLiteral("section_end DOCKER_SYSTEM_DF")
        << QStringLiteral("")
        << QStringLiteral("section DOCKER_PS")
        << QString("%1 ps --format '{{.Names}}|{{.Image}}|{{.Status}}|{{.Ports}}' 2>/dev/null || true").arg(dockerCommand)
        << QStringLiteral("section_end DOCKER_PS")
        << QStringLiteral("")
        << QStringLiteral("section SERVICE_LOGOS_NODE")
        << QString("if %1 inspect %2 >/dev/null 2>&1; then").arg(dockerCommand, logosContainer)
        << QString("  printf 'name=%s\\n' %1").arg(logosContainer)
        << QString("  %1 inspect --format 'running={{.State.Running}}").arg(dockerCommand)
        << QStringLiteral("health={{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}")
        << QStringLiteral("restart={{.HostConfig.RestartPolicy.Name}}")
        << QStringLiteral("image={{.Config.Image}}")
        << QStringLiteral("ports={{json .NetworkSettings.Ports}}")
        << (QStringLiteral("status={{.State.Status}}' ") + logosContainer)
        << QStringLiteral("else")
        << QStringLiteral("  printf '__MISSING__\\n'")
        << QStringLiteral("fi")
        << QStringLiteral("section_end SERVICE_LOGOS_NODE")
        << QStringLiteral("")
        << QStringLiteral("section SERVICE_NWAKU")
        << QString("if %1 inspect %2 >/dev/null 2>&1; then").arg(dockerCommand, wakuContainer)
        << QString("  printf 'name=%s\\n' %1").arg(wakuContainer)
        << QString("  %1 inspect --format 'running={{.State.Running}}").arg(dockerCommand)
        << QStringLiteral("health={{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}")
        << QStringLiteral("restart={{.HostConfig.RestartPolicy.Name}}")
        << QStringLiteral("image={{.Config.Image}}")
        << QStringLiteral("ports={{json .NetworkSettings.Ports}}")
        << (QStringLiteral("status={{.State.Status}}' ") + wakuContainer)
        << QStringLiteral("else")
        << QStringLiteral("  printf '__MISSING__\\n'")
        << QStringLiteral("fi")
        << QStringLiteral("section_end SERVICE_NWAKU")
        << QStringLiteral("")
        << QStringLiteral("section NETWORK_INFO")
        << QString("curl -fsS --max-time 8 http://127.0.0.1:%1/network/info 2>/dev/null || printf '__ERROR__:curl_failed\\n'").arg(apiPort)
        << QStringLiteral("section_end NETWORK_INFO")
        << QStringLiteral("")
        << QStringLiteral("section CRYPTARCHIA_INFO")
        << QString("curl -fsS --max-time 8 http://127.0.0.1:%1/cryptarchia/info 2>/dev/null || printf '__ERROR__:curl_failed\\n'").arg(apiPort)
        << QStringLiteral("section_end CRYPTARCHIA_INFO")
        << QStringLiteral("")
        << QStringLiteral("section LOGOS_NODE_LOGS")
        << QString("%1 logs --tail 120 %2 2>&1 || printf '__ERROR__:docker_logs_failed\\n'").arg(dockerCommand, logosContainer)
        << QStringLiteral("section_end LOGOS_NODE_LOGS")
        << QStringLiteral("")
        << QStringLiteral("section NWAKU_LOGS")
        << QString("%1 logs --tail 120 %2 2>&1 || printf '__ERROR__:docker_logs_failed\\n'").arg(dockerCommand, wakuContainer)
        << QStringLiteral("section_end NWAKU_LOGS")
        << QStringLiteral("");

    return lines.join('\n');
}
