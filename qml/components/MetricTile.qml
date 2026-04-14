import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Frame {
    id: root

    property string title: ""
    property string value: ""
    property string meta: ""
    property string severity: "healthy"

    Layout.fillWidth: true

    background: Rectangle {
        radius: DesignSystem.radiusLarge
        color: DesignSystem.surfaceRaised
        border.width: 1
        border.color: DesignSystem.stroke
    }

    contentItem: ColumnLayout {
        spacing: DesignSystem.space8

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: root.title
                color: DesignSystem.textSecondary
                font.pixelSize: DesignSystem.bodySize
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 8
                radius: 4
                color: DesignSystem.severityColor(root.severity)
            }
        }

        Label {
            text: root.value
            color: DesignSystem.textPrimary
            font.pixelSize: DesignSystem.valueSize
            font.bold: true
            wrapMode: Text.Wrap
        }

        Label {
            visible: root.meta.length > 0
            text: root.meta
            color: DesignSystem.textMuted
            font.pixelSize: 12
            wrapMode: Text.Wrap
        }
    }
}
