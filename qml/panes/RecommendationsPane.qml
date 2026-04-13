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
        spacing: DesignSystem.space16

        Repeater {
            model: root.controller.suggestions

            delegate: Frame {
                required property var modelData

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
                            text: modelData.title || "-"
                            color: DesignSystem.textPrimary
                            font.pixelSize: DesignSystem.sectionTitleSize
                            font.bold: true
                        }

                        Item { Layout.fillWidth: true }

                        SeverityChip {
                            severity: modelData.severity
                            text: modelData.severity
                        }
                    }

                    Label {
                        text: modelData.detail || "-"
                        color: DesignSystem.textSecondary
                        wrapMode: Text.Wrap
                    }

                    TextArea {
                        Layout.fillWidth: true
                        readOnly: true
                        wrapMode: Text.WrapAnywhere
                        text: modelData.command || "-"
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
                        id: suggestionCopyButton
                        text: "Copy command"
                        onClicked: root.controller.copyToClipboard(modelData.command || "")

                        background: Rectangle {
                            radius: DesignSystem.radiusMedium
                            color: DesignSystem.surfaceSoft
                            border.width: 1
                            border.color: DesignSystem.stroke
                        }

                        contentItem: Label {
                            text: suggestionCopyButton.text
                            color: DesignSystem.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }
}
