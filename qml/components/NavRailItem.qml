import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
    id: root

    property string code: ""
    property string labelText: ""
    property bool compact: false
    property bool selected: false

    implicitHeight: 46
    Layout.fillWidth: true
    leftPadding: DesignSystem.space12
    rightPadding: DesignSystem.space12
    topPadding: DesignSystem.space8
    bottomPadding: DesignSystem.space8

    background: Rectangle {
        radius: DesignSystem.radiusMedium
        color: root.selected ? DesignSystem.surfaceRaised : "transparent"
        border.width: 1
        border.color: root.selected ? DesignSystem.accent : "transparent"
    }

    contentItem: RowLayout {
        spacing: DesignSystem.space12

        Rectangle {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            radius: 8
            color: root.selected ? DesignSystem.accent : DesignSystem.surfaceSoft
            border.width: 1
            border.color: root.selected ? DesignSystem.accent : DesignSystem.strokeMuted

            Label {
                anchors.centerIn: parent
                text: root.code
                color: root.selected ? DesignSystem.windowBg : DesignSystem.textSecondary
                font.pixelSize: 11
                font.bold: true
            }
        }

        Label {
            Layout.fillWidth: true
            visible: !root.compact
            text: root.labelText
            color: root.selected ? DesignSystem.textPrimary : DesignSystem.textSecondary
            font.pixelSize: DesignSystem.bodySize
            font.bold: root.selected
            elide: Text.ElideRight
        }
    }
}
