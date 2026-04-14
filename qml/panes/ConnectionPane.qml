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
                spacing: DesignSystem.space8

                Label {
                    text: "Saved Profile"
                    color: DesignSystem.textSecondary
                    font.pixelSize: DesignSystem.bodySize
                }

                Label {
                    text: root.controller.profileSummary
                    color: DesignSystem.textPrimary
                    wrapMode: Text.WrapAnywhere
                }
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
                spacing: DesignSystem.space16

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: DesignSystem.space16
                    rowSpacing: DesignSystem.space12

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Profile name"
                        text: root.controller.profileName
                        onTextChanged: root.controller.profileName = text
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Host"
                        text: root.controller.host
                        onTextChanged: root.controller.host = text
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "SSH user"
                        text: root.controller.user
                        onTextChanged: root.controller.user = text
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Port"
                        text: String(root.controller.port)
                        inputMethodHints: Qt.ImhDigitsOnly
                        onTextChanged: root.controller.port = Number(text || 22)
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Identity file"
                        text: root.controller.identityFile
                        onTextChanged: root.controller.identityFile = text
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "API port"
                        text: String(root.controller.apiPort)
                        inputMethodHints: Qt.ImhDigitsOnly
                        onTextChanged: root.controller.apiPort = Number(text || 8080)
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "logos container"
                        text: root.controller.logosContainerName
                        onTextChanged: root.controller.logosContainerName = text
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "nwaku container"
                        text: root.controller.wakuContainerName
                        onTextChanged: root.controller.wakuContainerName = text
                        color: DesignSystem.textPrimary
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.strokeMuted }
                    }
                }

                CheckBox {
                    id: sudoCheck
                    text: "Use sudo for Docker"
                    checked: root.controller.useSudo
                    onToggled: root.controller.useSudo = checked

                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        radius: 5
                        color: sudoCheck.checked ? DesignSystem.accent : DesignSystem.surfaceSoft
                        border.width: 1
                        border.color: sudoCheck.checked ? DesignSystem.accent : DesignSystem.stroke

                        Rectangle {
                            anchors.centerIn: parent
                            width: 8
                            height: 8
                            radius: 4
                            visible: sudoCheck.checked
                            color: DesignSystem.windowBg
                        }
                    }

                    contentItem: Label {
                        text: sudoCheck.text
                        color: DesignSystem.textPrimary
                        leftPadding: sudoCheck.indicator.width + DesignSystem.space8
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Button {
                        id: saveProfileButton
                        text: "Save profile"
                        onClicked: root.controller.saveProfile()
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.stroke }
                        contentItem: Label { text: saveProfileButton.text; color: DesignSystem.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }

                    Button {
                        id: runRefreshButton
                        text: root.controller.busy ? "Refreshing..." : "Run refresh"
                        enabled: root.controller.connectionConfigured && !root.controller.busy
                        onClicked: root.controller.refresh()
                        background: Rectangle { radius: DesignSystem.radiusMedium; color: DesignSystem.surfaceSoft; border.width: 1; border.color: DesignSystem.stroke }
                        contentItem: Label { text: runRefreshButton.text; color: DesignSystem.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }

                    Item { Layout.fillWidth: true }

                    SeverityChip {
                        severity: root.controller.connectionConfigured ? root.controller.overallStatus : "warning"
                        text: root.controller.connectionConfigured ? "ready" : "empty"
                    }
                }
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
                    text: "SSH Preview"
                    color: DesignSystem.textSecondary
                    font.pixelSize: DesignSystem.bodySize
                }

                TextArea {
                    Layout.fillWidth: true
                    readOnly: true
                    wrapMode: Text.WrapAnywhere
                    text: root.controller.sshPreviewCommand
                    color: DesignSystem.textPrimary
                    selectByMouse: true

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
