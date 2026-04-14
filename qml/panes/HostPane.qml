import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id: root

    required property QtObject controller

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: DesignSystem.space16

        Frame {
            Layout.fillWidth: true

            background: Rectangle {
                radius: DesignSystem.radiusLarge
                color: DesignSystem.surfaceRaised
                border.width: 1
                border.color: DesignSystem.stroke
            }

            contentItem: ColumnLayout {
                spacing: DesignSystem.space12

                Label {
                    text: "Host Summary"
                    color: DesignSystem.textPrimary
                    font.pixelSize: DesignSystem.sectionTitleSize
                    font.bold: true
                }

                InfoRow { labelText: "Hostname"; valueText: root.controller.hostStatus.hostname || "-" }
                InfoRow { labelText: "Kernel"; valueText: root.controller.hostStatus.kernel || "-" }
                InfoRow { labelText: "Uptime"; valueText: root.controller.hostStatus.uptime || "-" }
                InfoRow { labelText: "Disk"; valueText: root.controller.diskSummary }
                InfoRow { labelText: "Memory"; valueText: root.controller.memorySummary }
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
                spacing: DesignSystem.space12

                Label {
                    text: "Docker Disk Summary"
                    color: DesignSystem.textPrimary
                    font.pixelSize: DesignSystem.sectionTitleSize
                    font.bold: true
                }

                TextArea {
                    Layout.fillWidth: true
                    readOnly: true
                    wrapMode: Text.Wrap
                    text: root.controller.hostStatus.dockerDiskSummary || "No Docker disk summary collected."
                    color: DesignSystem.textPrimary

                    background: Rectangle {
                        radius: DesignSystem.radiusMedium
                        color: DesignSystem.surfaceSoft
                        border.width: 1
                        border.color: DesignSystem.strokeMuted
                    }
                }
            }
        }
    }
}
