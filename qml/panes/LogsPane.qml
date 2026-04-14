import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: root

    required property QtObject controller

    spacing: DesignSystem.space16

    TabBar {
        Layout.fillWidth: true
        currentIndex: root.controller.selectedLogService === "nwaku" ? 1 : 0

        background: Rectangle {
            radius: DesignSystem.radiusMedium
            color: DesignSystem.surfaceSoft
            border.width: 1
            border.color: DesignSystem.stroke
        }

        TabButton {
            id: logosLogTab
            text: "logos-node"
            onClicked: root.controller.selectedLogService = "logos-node"

            contentItem: Label {
                text: logosLogTab.text
                color: logosLogTab.checked ? DesignSystem.textPrimary : DesignSystem.textSecondary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: DesignSystem.radiusMedium
                color: logosLogTab.checked ? DesignSystem.surfaceRaised : "transparent"
            }
        }

        TabButton {
            id: wakuLogTab
            text: "nwaku"
            onClicked: root.controller.selectedLogService = "nwaku"

            contentItem: Label {
                text: wakuLogTab.text
                color: wakuLogTab.checked ? DesignSystem.textPrimary : DesignSystem.textSecondary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: DesignSystem.radiusMedium
                color: wakuLogTab.checked ? DesignSystem.surfaceRaised : "transparent"
            }
        }
    }

    Loader {
        Layout.fillWidth: true
        Layout.fillHeight: true
        asynchronous: true

        sourceComponent: Component {
            TextArea {
                readOnly: true
                wrapMode: Text.Wrap
                text: root.controller.activeLogContent
                color: DesignSystem.textPrimary
                font.family: "monospace"
                font.pixelSize: DesignSystem.monoSize
                selectByMouse: true

                background: Rectangle {
                    radius: DesignSystem.radiusLarge
                    color: DesignSystem.surfaceRaised
                    border.width: 1
                    border.color: DesignSystem.stroke
                }
            }
        }
    }
}
