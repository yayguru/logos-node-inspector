import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logos.NodeInspector 1.0

ScrollView {
    id: root

    required property QtObject controller

    clip: true
    contentWidth: availableWidth

    Frame {
        width: parent.width

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
                    text: "Runtime Details"
                    color: DesignSystem.textPrimary
                    font.pixelSize: DesignSystem.sectionTitleSize
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                SeverityChip {
                    severity: root.controller.overallStatus
                    text: root.controller.logosStatus.mode || root.controller.overallStatus
                }
            }

            InfoRow { labelText: "Peer ID"; valueText: root.controller.logosStatus.peerId || "-"; mono: true }
            InfoRow { labelText: "Mode"; valueText: root.controller.logosStatus.mode || "-" }
            InfoRow { labelText: "Height"; valueText: root.controller.logosStatus.height >= 0 ? root.controller.logosStatus.height : "-" }
            InfoRow { labelText: "Slot"; valueText: root.controller.logosStatus.slot >= 0 ? root.controller.logosStatus.slot : "-" }
            InfoRow { labelText: "LIB"; valueText: root.controller.logosStatus.lib || "-"; mono: true }
            InfoRow { labelText: "Tip"; valueText: root.controller.logosStatus.tip || "-"; mono: true }
            InfoRow { labelText: "/network/info"; valueText: root.controller.logosStatus.networkReachable ? "reachable" : (root.controller.logosStatus.networkError || "unreachable") }
            InfoRow { labelText: "/cryptarchia/info"; valueText: root.controller.logosStatus.cryptarchiaReachable ? "reachable" : (root.controller.logosStatus.cryptarchiaError || "unreachable") }
            InfoRow { labelText: "Peers"; valueText: root.controller.logosStatus.nPeers >= 0 ? root.controller.logosStatus.nPeers : "-" }
            InfoRow { labelText: "Connections"; valueText: root.controller.logosStatus.nConnections >= 0 ? root.controller.logosStatus.nConnections : "-" }
            InfoRow { labelText: "Pending"; valueText: root.controller.logosStatus.nPendingConnections >= 0 ? root.controller.logosStatus.nPendingConnections : "-" }
        }
    }
}
