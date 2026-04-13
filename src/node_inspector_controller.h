#pragma once

#include "inspector_types.h"
#include "ssh_collector.h"

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

class NodeInspectorController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY profileChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY profileChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY profileChanged)
    Q_PROPERTY(QString user READ user WRITE setUser NOTIFY profileChanged)
    Q_PROPERTY(QString identityFile READ identityFile WRITE setIdentityFile NOTIFY profileChanged)
    Q_PROPERTY(bool useSudo READ useSudo WRITE setUseSudo NOTIFY profileChanged)
    Q_PROPERTY(QString logosContainerName READ logosContainerName WRITE setLogosContainerName NOTIFY profileChanged)
    Q_PROPERTY(QString wakuContainerName READ wakuContainerName WRITE setWakuContainerName NOTIFY profileChanged)
    Q_PROPERTY(int apiPort READ apiPort WRITE setApiPort NOTIFY profileChanged)
    Q_PROPERTY(QString selectedPane READ selectedPane WRITE setSelectedPane NOTIFY selectedPaneChanged)
    Q_PROPERTY(QString selectedLogService READ selectedLogService WRITE setSelectedLogService NOTIFY selectedLogServiceChanged)
    Q_PROPERTY(bool connectionConfigured READ connectionConfigured NOTIFY profileChanged)
    Q_PROPERTY(QString profileSummary READ profileSummary NOTIFY profileChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString sshPreviewCommand READ sshPreviewCommand NOTIFY profileChanged)
    Q_PROPERTY(QString collectorStatus READ collectorStatus NOTIFY snapshotChanged)
    Q_PROPERTY(QString collectorMessage READ collectorMessage NOTIFY snapshotChanged)
    Q_PROPERTY(QString overallStatus READ overallStatus NOTIFY snapshotChanged)
    Q_PROPERTY(QString collectedAt READ collectedAt NOTIFY snapshotChanged)
    Q_PROPERTY(QString lastSuccessfulRefresh READ lastSuccessfulRefresh NOTIFY snapshotChanged)
    Q_PROPERTY(QString diskSummary READ diskSummary NOTIFY snapshotChanged)
    Q_PROPERTY(QString memorySummary READ memorySummary NOTIFY snapshotChanged)
    Q_PROPERTY(QString containersSummary READ containersSummary NOTIFY snapshotChanged)
    Q_PROPERTY(QString healthSummary READ healthSummary NOTIFY snapshotChanged)
    Q_PROPERTY(QString nextActionTitle READ nextActionTitle NOTIFY snapshotChanged)
    Q_PROPERTY(QString nextActionDetail READ nextActionDetail NOTIFY snapshotChanged)
    Q_PROPERTY(QString nextActionCommand READ nextActionCommand NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantMap hostStatus READ hostStatus NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantMap logosStatus READ logosStatus NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantMap logosService READ logosService NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantMap wakuService READ wakuService NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList serviceStatuses READ serviceStatuses NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList logExcerpts READ logExcerpts NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList suggestions READ suggestions NOTIFY snapshotChanged)
    Q_PROPERTY(QString activeLogContent READ activeLogContent NOTIFY snapshotChanged)

public:
    explicit NodeInspectorController(QObject* parent = nullptr);

    QString profileName() const;
    void setProfileName(const QString& value);

    QString host() const;
    void setHost(const QString& value);

    int port() const;
    void setPort(int value);

    QString user() const;
    void setUser(const QString& value);

    QString identityFile() const;
    void setIdentityFile(const QString& value);

    bool useSudo() const;
    void setUseSudo(bool value);

    QString logosContainerName() const;
    void setLogosContainerName(const QString& value);

    QString wakuContainerName() const;
    void setWakuContainerName(const QString& value);

    int apiPort() const;
    void setApiPort(int value);

    QString selectedPane() const;
    void setSelectedPane(const QString& value);

    QString selectedLogService() const;
    void setSelectedLogService(const QString& value);

    bool connectionConfigured() const;
    QString profileSummary() const;

    bool busy() const;
    QString sshPreviewCommand() const;
    QString collectorStatus() const;
    QString collectorMessage() const;
    QString overallStatus() const;
    QString collectedAt() const;
    QString lastSuccessfulRefresh() const;
    QString diskSummary() const;
    QString memorySummary() const;
    QString containersSummary() const;
    QString healthSummary() const;
    QString nextActionTitle() const;
    QString nextActionDetail() const;
    QString nextActionCommand() const;

    QVariantMap hostStatus() const;
    QVariantMap logosStatus() const;
    QVariantMap logosService() const;
    QVariantMap wakuService() const;
    QVariantList serviceStatuses() const;
    QVariantList logExcerpts() const;
    QVariantList suggestions() const;
    QString activeLogContent() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void saveProfile();
    Q_INVOKABLE void loadProfile();
    Q_INVOKABLE void copyToClipboard(const QString& text);

signals:
    void profileChanged();
    void busyChanged();
    void selectedPaneChanged();
    void selectedLogServiceChanged();
    void snapshotChanged();

private:
    void setCollectorError(const QString& message);
    void applySnapshot(inspector::InspectorSnapshot snapshot);
    void syncPaneWithProfile();

    inspector::ConnectionProfile m_profile;
    inspector::InspectorSnapshot m_snapshot;
    SshCollector m_collector;
    QString m_selectedPane;
    QString m_selectedLogService = QStringLiteral("logos-node");
    QString m_lastSuccessfulRefresh;
};
