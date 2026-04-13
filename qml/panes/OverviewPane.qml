import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logos.NodeInspector 1.0

ScrollView {
    id: root

    required property QtObject controller

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: DesignSystem.space20

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: DesignSystem.space16
            rowSpacing: DesignSystem.space16

            MetricTile {
                title: "Node"
                value: root.controller.connectionConfigured ? root.controller.profileName : "Not configured"
                meta: root.controller.profileSummary
                severity: root.controller.overallStatus
            }

            MetricTile {
                title: "Sync Mode"
                value: root.controller.logosStatus.mode || "Unknown"
                meta: "Height " + (root.controller.logosStatus.height >= 0 ? root.controller.logosStatus.height : "n/a")
                severity: root.controller.logosStatus.mode === "Bootstrapping" ? "warning" : root.controller.overallStatus
            }

            MetricTile {
                title: "Peers"
                value: root.controller.logosStatus.nPeers >= 0 ? root.controller.logosStatus.nPeers : "n/a"
                meta: "Connections " + (root.controller.logosStatus.nConnections >= 0 ? root.controller.logosStatus.nConnections : "n/a")
                severity: Number(root.controller.logosStatus.nPeers) > 0 ? "healthy" : "critical"
            }

            MetricTile {
                title: "Disk"
                value: root.controller.diskSummary
                meta: root.controller.hostStatus.hostname || "-"
                severity: Number(root.controller.hostStatus.diskUsedPercent) >= 95 ? "critical" : Number(root.controller.hostStatus.diskUsedPercent) >= 85 ? "warning" : "healthy"
            }

            MetricTile {
                title: "Containers"
                value: root.controller.containersSummary
                meta: "logos-node + nwaku"
                severity: root.controller.overallStatus
            }

            MetricTile {
                title: "Last Refresh"
                value: root.controller.lastSuccessfulRefresh
                meta: root.controller.collectorStatus
                severity: root.controller.collectorStatus === "error" ? "critical" : root.controller.overallStatus
            }
        }

        Frame {
            Layout.fillWidth: true

            background: Rectangle {
                radius: DesignSystem.radiusLarge
                color: DesignSystem.surfaceRaised
                border.width: 1
                border.color: DesignSystem.stroke
            }

            contentItem: ColumnLayout {
                spacing: DesignSystem.space8

                Label {
                    text: "Health Summary"
                    color: DesignSystem.textSecondary
                    font.pixelSize: DesignSystem.bodySize
                }

                Label {
                    text: root.controller.healthSummary
                    color: DesignSystem.textPrimary
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
        }

        Frame {
            visible: root.controller.nextActionTitle.length > 0
            Layout.fillWidth: true

            background: Rectangle {
                radius: DesignSystem.radiusLarge
                color: DesignSystem.surfaceRaised
                border.width: 1
                border.color: DesignSystem.stroke
            }

            contentItem: ColumnLayout {
                spacing: DesignSystem.space12

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "Next Action"
                        color: DesignSystem.textSecondary
                        font.pixelSize: DesignSystem.bodySize
                    }

                    Item { Layout.fillWidth: true }

                    SeverityChip {
                        severity: root.controller.overallStatus
                        text: root.controller.overallStatus
                    }
                }

                Label {
                    text: root.controller.nextActionTitle
                    color: DesignSystem.textPrimary
                    font.pixelSize: DesignSystem.sectionTitleSize
                    font.bold: true
                }

                Label {
                    text: root.controller.nextActionDetail
                    color: DesignSystem.textSecondary
                    wrapMode: Text.Wrap
                }

                TextArea {
                    Layout.fillWidth: true
                    readOnly: true
                    wrapMode: Text.WrapAnywhere
                    text: root.controller.nextActionCommand
                    color: DesignSystem.textPrimary
                    selectByMouse: true

                    background: Rectangle {
                        radius: DesignSystem.radiusMedium
                        color: DesignSystem.surfaceSoft
                        border.width: 1
                        border.color: DesignSystem.strokeMuted
                    }
                }

                Button {
                    id: nextActionButton
                    text: "Copy command"
                    onClicked: root.controller.copyToClipboard(root.controller.nextActionCommand)

                    background: Rectangle {
                        radius: DesignSystem.radiusMedium
                        color: DesignSystem.surfaceSoft
                        border.width: 1
                        border.color: DesignSystem.stroke
                    }

                    contentItem: Label {
                        text: nextActionButton.text
                        color: DesignSystem.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
