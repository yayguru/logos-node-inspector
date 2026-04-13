import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logos.NodeInspector 1.0

RowLayout {
    id: root

    property string labelText: ""
    property string valueText: ""
    property bool mono: false

    Layout.fillWidth: true
    spacing: DesignSystem.space12

    Label {
        Layout.preferredWidth: 160
        text: root.labelText
        color: DesignSystem.textSecondary
        font.pixelSize: DesignSystem.bodySize
        elide: Text.ElideRight
    }

    Label {
        Layout.fillWidth: true
        text: root.valueText
        color: DesignSystem.textPrimary
        font.pixelSize: root.mono ? DesignSystem.monoSize : DesignSystem.bodySize
        font.family: root.mono ? "monospace" : font.family
        wrapMode: Text.WrapAnywhere
    }
}

