import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id: root

    required property QtObject controller

    function containerSeverity(serviceData) {
        if (!serviceData.present || !serviceData.running)
            return "critical"
        if (serviceData.health === "unhealthy")
            return "warning"
        return "healthy"
    }

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

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "logos-node"; color: DesignSystem.textPrimary; font.pixelSize: DesignSystem.sectionTitleSize; font.bold: true }
                    Item { Layout.fillWidth: true }
                    SeverityChip { severity: root.containerSeverity(root.controller.logosService); text: root.controller.logosService.health || "unknown" }
                }

                InfoRow { labelText: "Present"; valueText: root.controller.logosService.present ? "Yes" : "No" }
                InfoRow { labelText: "Running"; valueText: root.controller.logosService.running ? "Yes" : "No" }
                InfoRow { labelText: "Status"; valueText: root.controller.logosService.status || "-" }
                InfoRow { labelText: "Restart"; valueText: root.controller.logosService.restartPolicy || "-" }
                InfoRow { labelText: "Image"; valueText: root.controller.logosService.image || "-"; mono: true }
                InfoRow { labelText: "Ports"; valueText: root.controller.logosService.ports || "-"; mono: true }
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

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "nwaku"; color: DesignSystem.textPrimary; font.pixelSize: DesignSystem.sectionTitleSize; font.bold: true }
                    Item { Layout.fillWidth: true }
                    SeverityChip { severity: root.containerSeverity(root.controller.wakuService); text: root.controller.wakuService.health || "unknown" }
                }

                InfoRow { labelText: "Present"; valueText: root.controller.wakuService.present ? "Yes" : "No" }
                InfoRow { labelText: "Running"; valueText: root.controller.wakuService.running ? "Yes" : "No" }
                InfoRow { labelText: "Status"; valueText: root.controller.wakuService.status || "-" }
                InfoRow { labelText: "Restart"; valueText: root.controller.wakuService.restartPolicy || "-" }
                InfoRow { labelText: "Image"; valueText: root.controller.wakuService.image || "-"; mono: true }
                InfoRow { labelText: "Ports"; valueText: root.controller.wakuService.ports || "-"; mono: true }
            }
        }
    }
}
