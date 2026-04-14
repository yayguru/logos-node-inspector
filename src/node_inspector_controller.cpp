#include "node_inspector_controller.h"

#include "remote_output_parser.h"
#include "status_classifier.h"

#include <QClipboard>
#include <QDateTime>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSettings>
#include <utility>

namespace {

QString toQString(const std::string& value)
{
    return QString::fromStdString(value);
}

QString formatBytes(const qint64 bytes)
{
    if (bytes < 0) {
        return QStringLiteral("n/a");
    }

    static const char* units[] = { "B", "KiB", "MiB", "GiB", "TiB" };
    double value = static_cast<double>(bytes);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 4) {
        value /= 1024.0;
        ++unitIndex;
    }

    return QLocale().toString(value, 'f', value >= 100.0 ? 0 : 1) + QStringLiteral(" ") + QString::fromLatin1(units[unitIndex]);
}

QString defaultPaneForProfile(const inspector::ConnectionProfile& profile)
{
    return profile.host.empty() ? QStringLiteral("connection") : QStringLiteral("overview");
}

QVariantMap emptyService()
{
    return {
        { QStringLiteral("name"), QStringLiteral("-") },
        { QStringLiteral("present"), false },
        { QStringLiteral("running"), false },
        { QStringLiteral("status"), QStringLiteral("missing") },
        { QStringLiteral("health"), QStringLiteral("unknown") },
        { QStringLiteral("restartPolicy"), QStringLiteral("-") },
        { QStringLiteral("image"), QStringLiteral("-") },
        { QStringLiteral("ports"), QStringLiteral("-") }
    };
}

QVariantMap toVariantMap(const inspector::HostStatus& host)
{
    return {
        { QStringLiteral("hostname"), toQString(host.hostname) },
        { QStringLiteral("kernel"), toQString(host.kernel) },
        { QStringLiteral("uptime"), toQString(host.uptime) },
        { QStringLiteral("hasDisk"), host.hasDisk },
        { QStringLiteral("diskTotalBytes"), static_cast<qint64>(host.diskTotalBytes) },
        { QStringLiteral("diskUsedBytes"), static_cast<qint64>(host.diskUsedBytes) },
        { QStringLiteral("diskAvailableBytes"), static_cast<qint64>(host.diskAvailableBytes) },
        { QStringLiteral("diskUsedPercent"), static_cast<double>(host.diskUsedPercent) },
        { QStringLiteral("hasMemory"), host.hasMemory },
        { QStringLiteral("memoryTotalBytes"), static_cast<qint64>(host.memoryTotalBytes) },
        { QStringLiteral("memoryUsedBytes"), static_cast<qint64>(host.memoryUsedBytes) },
        { QStringLiteral("memoryFreeBytes"), static_cast<qint64>(host.memoryFreeBytes) },
        { QStringLiteral("memoryAvailableBytes"), static_cast<qint64>(host.memoryAvailableBytes) },
        { QStringLiteral("dockerDiskSummary"), toQString(host.dockerDiskSummary) }
    };
}

QVariantMap toVariantMap(const inspector::LogosStatus& logos)
{
    return {
        { QStringLiteral("networkReachable"), logos.networkReachable },
        { QStringLiteral("cryptarchiaReachable"), logos.cryptarchiaReachable },
        { QStringLiteral("networkError"), toQString(logos.networkError) },
        { QStringLiteral("cryptarchiaError"), toQString(logos.cryptarchiaError) },
        { QStringLiteral("peerId"), toQString(logos.peerId) },
        { QStringLiteral("nPeers"), static_cast<int>(logos.nPeers) },
        { QStringLiteral("nConnections"), static_cast<int>(logos.nConnections) },
        { QStringLiteral("nPendingConnections"), static_cast<int>(logos.nPendingConnections) },
        { QStringLiteral("mode"), toQString(logos.mode) },
        { QStringLiteral("height"), static_cast<qint64>(logos.height) },
        { QStringLiteral("slot"), static_cast<qint64>(logos.slot) },
        { QStringLiteral("lib"), toQString(logos.lib) },
        { QStringLiteral("tip"), toQString(logos.tip) }
    };
}

QVariantMap toVariantMap(const inspector::ServiceStatus& service)
{
    return {
        { QStringLiteral("name"), toQString(service.name) },
        { QStringLiteral("present"), service.present },
        { QStringLiteral("running"), service.running },
        { QStringLiteral("status"), toQString(service.status) },
        { QStringLiteral("health"), toQString(service.health) },
        { QStringLiteral("restartPolicy"), toQString(service.restartPolicy) },
        { QStringLiteral("image"), toQString(service.image) },
        { QStringLiteral("ports"), toQString(service.ports) }
    };
}

QVariantMap toVariantMap(const inspector::LogExcerpt& log)
{
    return {
        { QStringLiteral("serviceName"), toQString(log.serviceName) },
        { QStringLiteral("content"), toQString(log.content) }
    };
}

QVariantMap toVariantMap(const inspector::Suggestion& suggestion)
{
    return {
        { QStringLiteral("severity"), QString::fromUtf8(inspector::severityToString(suggestion.severity).data()) },
        { QStringLiteral("title"), toQString(suggestion.title) },
        { QStringLiteral("detail"), toQString(suggestion.detail) },
        { QStringLiteral("command"), toQString(suggestion.command) }
    };
}

QVariantList toVariantList(const std::vector<inspector::ServiceStatus>& services)
{
    QVariantList values;
    values.reserve(static_cast<int>(services.size()));
    for (const auto& service : services) {
        values.push_back(toVariantMap(service));
    }
    return values;
}

QVariantList toVariantList(const std::vector<inspector::LogExcerpt>& logs)
{
    QVariantList values;
    values.reserve(static_cast<int>(logs.size()));
    for (const auto& log : logs) {
        values.push_back(toVariantMap(log));
    }
    return values;
}

QVariantList toVariantList(const std::vector<inspector::Suggestion>& suggestions)
{
    QVariantList values;
    values.reserve(static_cast<int>(suggestions.size()));
    for (const auto& suggestion : suggestions) {
        values.push_back(toVariantMap(suggestion));
    }
    return values;
}

QSettings settings()
{
    return QSettings(QStringLiteral("Logos"), QStringLiteral("NodeInspector"));
}

QJsonObject toJsonObject(const inspector::ConnectionProfile& profile)
{
    return QJsonObject {
        { QStringLiteral("name"), QString::fromStdString(profile.name) },
        { QStringLiteral("host"), QString::fromStdString(profile.host) },
        { QStringLiteral("port"), profile.port },
        { QStringLiteral("user"), QString::fromStdString(profile.user) },
        { QStringLiteral("identityFile"), QString::fromStdString(profile.identityFile) },
        { QStringLiteral("useSudo"), profile.useSudo },
        { QStringLiteral("logosContainerName"), QString::fromStdString(profile.logosContainerName) },
        { QStringLiteral("wakuContainerName"), QString::fromStdString(profile.wakuContainerName) },
        { QStringLiteral("apiPort"), profile.apiPort }
    };
}

inspector::ConnectionProfile fromJsonObject(const QJsonObject& object)
{
    inspector::ConnectionProfile profile;
    profile.name = object.value(QStringLiteral("name")).toString(QStringLiteral("Primary VPS")).toStdString();
    profile.host = object.value(QStringLiteral("host")).toString().toStdString();
    profile.port = object.value(QStringLiteral("port")).toInt(22);
    profile.user = object.value(QStringLiteral("user")).toString(QStringLiteral("root")).toStdString();
    profile.identityFile = object.value(QStringLiteral("identityFile")).toString().toStdString();
    profile.useSudo = object.value(QStringLiteral("useSudo")).toBool(false);
    profile.logosContainerName = object.value(QStringLiteral("logosContainerName")).toString(QStringLiteral("logos-node")).toStdString();
    profile.wakuContainerName = object.value(QStringLiteral("wakuContainerName")).toString(QStringLiteral("nwaku")).toStdString();
    profile.apiPort = object.value(QStringLiteral("apiPort")).toInt(8080);
    return profile;
}

} // namespace

NodeInspectorController::NodeInspectorController(QObject* parent)
    : QObject(parent)
{
    connect(&m_collector, &SshCollector::finished, this,
        [this](const QString& stdoutText, const QString& stderrText, const int exitCode) {
            auto snapshot = inspector::RemoteOutputParser::parse(
                stdoutText.toStdString(),
                stderrText.toStdString(),
                exitCode
            );
            snapshot.collectedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
            inspector::StatusClassifier::apply(snapshot, m_profile);
            applySnapshot(std::move(snapshot));
        });
    connect(&m_collector, &SshCollector::busyChanged, this, &NodeInspectorController::busyChanged);

    loadProfile();
    if (m_selectedPane.isEmpty()) {
        m_selectedPane = defaultPaneForProfile(m_profile);
    }
}

QString NodeInspectorController::profileName() const { return toQString(m_profile.name); }
void NodeInspectorController::setProfileName(const QString& value) { m_profile.name = value.toStdString(); emit profileChanged(); }
QString NodeInspectorController::host() const { return toQString(m_profile.host); }
void NodeInspectorController::setHost(const QString& value) { m_profile.host = value.trimmed().toStdString(); syncPaneWithProfile(); emit profileChanged(); }
int NodeInspectorController::port() const { return m_profile.port; }
void NodeInspectorController::setPort(const int value) { m_profile.port = value; emit profileChanged(); }
QString NodeInspectorController::user() const { return toQString(m_profile.user); }
void NodeInspectorController::setUser(const QString& value) { m_profile.user = value.trimmed().toStdString(); emit profileChanged(); }
QString NodeInspectorController::identityFile() const { return toQString(m_profile.identityFile); }
void NodeInspectorController::setIdentityFile(const QString& value) { m_profile.identityFile = value.trimmed().toStdString(); emit profileChanged(); }
bool NodeInspectorController::useSudo() const { return m_profile.useSudo; }
void NodeInspectorController::setUseSudo(const bool value) { m_profile.useSudo = value; emit profileChanged(); }
QString NodeInspectorController::logosContainerName() const { return toQString(m_profile.logosContainerName); }
void NodeInspectorController::setLogosContainerName(const QString& value) { m_profile.logosContainerName = value.trimmed().toStdString(); emit profileChanged(); }
QString NodeInspectorController::wakuContainerName() const { return toQString(m_profile.wakuContainerName); }
void NodeInspectorController::setWakuContainerName(const QString& value) { m_profile.wakuContainerName = value.trimmed().toStdString(); emit profileChanged(); }
int NodeInspectorController::apiPort() const { return m_profile.apiPort; }
void NodeInspectorController::setApiPort(const int value) { m_profile.apiPort = value; emit profileChanged(); }
QString NodeInspectorController::selectedPane() const { return m_selectedPane; }
void NodeInspectorController::setSelectedPane(const QString& value)
{
    const auto normalized = value.trimmed().isEmpty() ? defaultPaneForProfile(m_profile) : value.trimmed().toLower();
    if (normalized == m_selectedPane) {
        return;
    }
    m_selectedPane = normalized;
    emit selectedPaneChanged();
}
QString NodeInspectorController::selectedLogService() const { return m_selectedLogService; }
void NodeInspectorController::setSelectedLogService(const QString& value)
{
    const auto normalized = value.trimmed().isEmpty() ? QStringLiteral("logos-node") : value.trimmed();
    if (normalized == m_selectedLogService) {
        return;
    }
    m_selectedLogService = normalized;
    emit selectedLogServiceChanged();
    emit snapshotChanged();
}
bool NodeInspectorController::connectionConfigured() const { return !m_profile.host.empty(); }
QString NodeInspectorController::profileSummary() const
{
    if (!connectionConfigured()) {
        return QStringLiteral("No saved SSH profile");
    }

    return QStringLiteral("%1@%2:%3  |  %4 / %5  |  API %6")
        .arg(user())
        .arg(host())
        .arg(QString::number(port()))
        .arg(logosContainerName())
        .arg(wakuContainerName())
        .arg(QString::number(apiPort()));
}
bool NodeInspectorController::busy() const { return m_collector.busy(); }
QString NodeInspectorController::sshPreviewCommand() const { return m_collector.commandPreview(m_profile); }
QString NodeInspectorController::collectorStatus() const { return toQString(m_snapshot.collectorStatus); }
QString NodeInspectorController::collectorMessage() const { return toQString(m_snapshot.collectorMessage); }
QString NodeInspectorController::overallStatus() const { return QString::fromUtf8(inspector::severityToString(m_snapshot.overallStatus).data()); }
QString NodeInspectorController::collectedAt() const { return toQString(m_snapshot.collectedAt); }
QString NodeInspectorController::lastSuccessfulRefresh() const
{
    return m_lastSuccessfulRefresh.isEmpty() ? QStringLiteral("Never") : m_lastSuccessfulRefresh;
}
QString NodeInspectorController::diskSummary() const
{
    if (!m_snapshot.hostStatus.hasDisk) {
        return QStringLiteral("No disk data");
    }

    return QStringLiteral("%1 used  |  %2 free of %3")
        .arg(QLocale().toString(m_snapshot.hostStatus.diskUsedPercent, 'f', 1) + QStringLiteral("%"))
        .arg(formatBytes(m_snapshot.hostStatus.diskAvailableBytes))
        .arg(formatBytes(m_snapshot.hostStatus.diskTotalBytes));
}
QString NodeInspectorController::memorySummary() const
{
    if (!m_snapshot.hostStatus.hasMemory) {
        return QStringLiteral("No memory data");
    }

    return QStringLiteral("%1 used of %2  |  %3 available")
        .arg(formatBytes(m_snapshot.hostStatus.memoryUsedBytes))
        .arg(formatBytes(m_snapshot.hostStatus.memoryTotalBytes))
        .arg(formatBytes(m_snapshot.hostStatus.memoryAvailableBytes));
}
QString NodeInspectorController::containersSummary() const
{
    int present = 0;
    int running = 0;
    for (const auto& service : m_snapshot.serviceStatuses) {
        if (service.present) {
            ++present;
        }
        if (service.running) {
            ++running;
        }
    }
    return QStringLiteral("%1 running / %2 present").arg(running).arg(present);
}
QString NodeInspectorController::healthSummary() const
{
    if (m_snapshot.collectorStatus == "error") {
        return collectorMessage();
    }

    if (m_snapshot.overallStatus == inspector::Severity::healthy) {
        return QStringLiteral("Read-only checks are healthy and the node is responding.");
    }

    if (m_snapshot.logosStatus.mode == "Bootstrapping" && m_snapshot.logosStatus.height >= 0) {
        return QStringLiteral("Logos is bootstrapping and advancing through chain state while the VPS remains readable.");
    }

    if (!m_snapshot.suggestions.empty()) {
        return toQString(m_snapshot.suggestions.front().detail);
    }

    return QStringLiteral("The inspector found conditions that need operator attention.");
}
QString NodeInspectorController::nextActionTitle() const
{
    for (const auto& suggestion : m_snapshot.suggestions) {
        if (suggestion.severity != inspector::Severity::healthy) {
            return toQString(suggestion.title);
        }
    }
    return QString();
}
QString NodeInspectorController::nextActionDetail() const
{
    for (const auto& suggestion : m_snapshot.suggestions) {
        if (suggestion.severity != inspector::Severity::healthy) {
            return toQString(suggestion.detail);
        }
    }
    return QString();
}
QString NodeInspectorController::nextActionCommand() const
{
    for (const auto& suggestion : m_snapshot.suggestions) {
        if (suggestion.severity != inspector::Severity::healthy) {
            return toQString(suggestion.command);
        }
    }
    return QString();
}
QVariantMap NodeInspectorController::hostStatus() const { return toVariantMap(m_snapshot.hostStatus); }
QVariantMap NodeInspectorController::logosStatus() const { return toVariantMap(m_snapshot.logosStatus); }
QVariantMap NodeInspectorController::logosService() const
{
    if (const auto* service = inspector::findService(m_snapshot, "logos-node")) {
        return toVariantMap(*service);
    }
    return emptyService();
}
QVariantMap NodeInspectorController::wakuService() const
{
    if (const auto* service = inspector::findService(m_snapshot, "nwaku")) {
        return toVariantMap(*service);
    }
    return emptyService();
}
QVariantList NodeInspectorController::serviceStatuses() const { return toVariantList(m_snapshot.serviceStatuses); }
QVariantList NodeInspectorController::logExcerpts() const { return toVariantList(m_snapshot.logExcerpts); }
QVariantList NodeInspectorController::suggestions() const { return toVariantList(m_snapshot.suggestions); }
QString NodeInspectorController::activeLogContent() const
{
    for (const auto& log : m_snapshot.logExcerpts) {
        if (toQString(log.serviceName) == m_selectedLogService) {
            return toQString(log.content);
        }
    }
    return QStringLiteral("No log excerpt collected for %1.").arg(m_selectedLogService);
}

void NodeInspectorController::refresh()
{
    if (busy()) {
        return;
    }

    if (m_profile.host.empty()) {
        setCollectorError(QStringLiteral("A host is required before running a read-only refresh."));
        return;
    }

    m_snapshot.collectorStatus = "running";
    m_snapshot.collectorMessage = "Collecting read-only snapshot...";
    m_snapshot.collectedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
    emit snapshotChanged();
    m_collector.collect(m_profile);
}

void NodeInspectorController::saveProfile()
{
    auto store = settings();
    store.setValue(
        QStringLiteral("connectionProfile"),
        QString::fromUtf8(QJsonDocument(toJsonObject(m_profile)).toJson(QJsonDocument::Compact))
    );
}

void NodeInspectorController::loadProfile()
{
    auto store = settings();
    const auto raw = store.value(QStringLiteral("connectionProfile")).toString();
    if (!raw.trimmed().isEmpty()) {
        const auto document = QJsonDocument::fromJson(raw.toUtf8());
        if (document.isObject()) {
            m_profile = fromJsonObject(document.object());
        }
    }
    syncPaneWithProfile();
    emit profileChanged();
}

void NodeInspectorController::copyToClipboard(const QString& text)
{
    if (auto* clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(text);
    }
}

void NodeInspectorController::setCollectorError(const QString& message)
{
    inspector::InspectorSnapshot snapshot;
    snapshot.collectedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
    snapshot.collectorStatus = "error";
    snapshot.collectorMessage = message.toStdString();
    snapshot.overallStatus = inspector::Severity::critical;
    snapshot.suggestions.push_back(inspector::Suggestion {
        .severity = inspector::Severity::critical,
        .title = "Fill in the connection profile",
        .detail = "Add a host and SSH identity before refreshing.",
        .command = QStringLiteral("ssh -i /path/to/key root@your-vps").toStdString()
    });
    applySnapshot(std::move(snapshot));
}

void NodeInspectorController::applySnapshot(inspector::InspectorSnapshot snapshot)
{
    if (snapshot.collectorStatus != "error" && !snapshot.collectedAt.empty()) {
        m_lastSuccessfulRefresh = toQString(snapshot.collectedAt);
    }
    m_snapshot = std::move(snapshot);
    emit snapshotChanged();
}

void NodeInspectorController::syncPaneWithProfile()
{
    if (!connectionConfigured()) {
        setSelectedPane(QStringLiteral("connection"));
        return;
    }

    if (m_selectedPane.isEmpty() || m_selectedPane == QStringLiteral("connection")) {
        setSelectedPane(QStringLiteral("overview"));
    }
}
