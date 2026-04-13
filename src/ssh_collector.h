#pragma once

#include "inspector_types.h"

#include <QObject>
#include <QProcess>
#include <QStringList>

class SshCollector : public QObject
{
    Q_OBJECT

public:
    explicit SshCollector(QObject* parent = nullptr);

    void collect(const inspector::ConnectionProfile& profile);
    QString commandPreview(const inspector::ConnectionProfile& profile) const;
    bool busy() const;

signals:
    void finished(const QString& stdoutText, const QString& stderrText, int exitCode);
    void busyChanged();

private slots:
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);

private:
    static QString shellQuote(const QString& value);
    QStringList buildArgs(const inspector::ConnectionProfile& profile) const;
    QString buildRemoteScript(const inspector::ConnectionProfile& profile) const;

    QProcess m_process;
    bool m_active = false;
};

