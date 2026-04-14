import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: root

    NodeInspectorController {
        id: controller
    }

    readonly property bool compactRail: width < 1120
    readonly property var paneItems: [
        { key: "overview", code: "OV", title: "Overview" },
        { key: "logos", code: "LG", title: "Logos" },
        { key: "services", code: "SV", title: "Services" },
        { key: "host", code: "HT", title: "Host" },
        { key: "recommendations", code: "RC", title: "Recommendations" },
        { key: "logs", code: "LO", title: "Logs" },
        { key: "connection", code: "CN", title: "Connection" }
    ]

    function paneTitle(key) {
        for (let i = 0; i < paneItems.length; ++i) {
            if (paneItems[i].key === key)
                return paneItems[i].title
        }
        return "Overview"
    }

    padding: 0

    background: Rectangle {
        color: DesignSystem.windowBg
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: DesignSystem.space20
        spacing: DesignSystem.space20

        Frame {
            Layout.preferredWidth: root.compactRail ? DesignSystem.railCompact : DesignSystem.railExpanded
            Layout.fillHeight: true

            background: Rectangle {
                radius: DesignSystem.radiusLarge
                color: DesignSystem.surface
                border.width: 1
                border.color: DesignSystem.stroke
            }

            contentItem: ColumnLayout {
                spacing: DesignSystem.space12

                Label {
                    visible: !root.compactRail
                    text: "Node Inspector"
                    color: DesignSystem.textPrimary
                    font.pixelSize: DesignSystem.sectionTitleSize
                    font.bold: true
                }

                Label {
                    visible: !root.compactRail
                    text: controller.profileSummary
                    color: DesignSystem.textSecondary
                    wrapMode: Text.Wrap
                    font.pixelSize: 12
                }

                Rectangle {
                    visible: !root.compactRail
                    Layout.fillWidth: true
                    height: 1
                    color: DesignSystem.strokeMuted
                }

                Repeater {
                    model: root.paneItems

                    delegate: NavRailItem {
                        required property var modelData
                        compact: root.compactRail
                        code: modelData.code
                        labelText: modelData.title
                        selected: controller.selectedPane === modelData.key
                        onClicked: controller.selectedPane = modelData.key
                    }
                }

                Item { Layout.fillHeight: true }

                SeverityChip {
                    Layout.alignment: Qt.AlignHCenter
                    severity: controller.overallStatus
                    text: controller.overallStatus
                }

                Label {
                    visible: !root.compactRail
                    Layout.fillWidth: true
                    text: controller.healthSummary
                    color: DesignSystem.textMuted
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true

            background: Rectangle {
                radius: DesignSystem.radiusLarge
                color: DesignSystem.surface
                border.width: 1
                border.color: DesignSystem.stroke
            }

            contentItem: ColumnLayout {
                spacing: DesignSystem.space16

                RowLayout {
                    Layout.fillWidth: true

                    ColumnLayout {
                        spacing: DesignSystem.space4

                        Label {
                            text: root.paneTitle(controller.selectedPane)
                            color: DesignSystem.textPrimary
                            font.pixelSize: DesignSystem.titleSize
                            font.bold: true
                        }

                        Label {
                            text: controller.selectedPane === "connection"
                                ? "Configure one SSH profile and keep the module read-only."
                                : controller.healthSummary
                            color: DesignSystem.textSecondary
                            wrapMode: Text.Wrap
                            Layout.maximumWidth: 820
                        }
                    }

                    Item { Layout.fillWidth: true }

                    ColumnLayout {
                        spacing: DesignSystem.space4
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                        Label {
                            text: "Last successful refresh"
                            color: DesignSystem.textMuted
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignRight
                        }

                        Label {
                            text: controller.lastSuccessfulRefresh
                            color: DesignSystem.textPrimary
                            font.pixelSize: DesignSystem.bodySize
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    Button {
                        id: refreshButton
                        text: controller.busy ? "Refreshing..." : "Refresh"
                        enabled: !controller.busy && controller.connectionConfigured
                        onClicked: controller.refresh()

                        background: Rectangle {
                            radius: DesignSystem.radiusMedium
                            color: refreshButton.down ? DesignSystem.surfaceSoft : DesignSystem.surfaceRaised
                            border.width: 1
                            border.color: DesignSystem.stroke
                        }

                        contentItem: Label {
                            text: refreshButton.text
                            color: DesignSystem.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: DesignSystem.strokeMuted
                }

                Loader {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    asynchronous: true
                    sourceComponent: {
                        switch (controller.selectedPane) {
                        case "logos":
                            return logosPaneComponent
                        case "services":
                            return servicesPaneComponent
                        case "host":
                            return hostPaneComponent
                        case "recommendations":
                            return recommendationsPaneComponent
                        case "logs":
                            return logsPaneComponent
                        case "connection":
                            return connectionPaneComponent
                        default:
                            return overviewPaneComponent
                        }
                    }
                }
            }
        }
    }

    Component { id: overviewPaneComponent; OverviewPane { controller: controller } }
    Component { id: logosPaneComponent; LogosPane { controller: controller } }
    Component { id: servicesPaneComponent; ServicesPane { controller: controller } }
    Component { id: hostPaneComponent; HostPane { controller: controller } }
    Component { id: recommendationsPaneComponent; RecommendationsPane { controller: controller } }
    Component { id: logsPaneComponent; LogsPane { controller: controller } }
    Component { id: connectionPaneComponent; ConnectionPane { controller: controller } }
}
