import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id: root

    property string severity: "healthy"
    property string text: severity

    radius: 7
    color: Qt.rgba(DesignSystem.severityColor(severity).r, DesignSystem.severityColor(severity).g, DesignSystem.severityColor(severity).b, 0.14)
    border.width: 1
    border.color: Qt.rgba(DesignSystem.severityColor(severity).r, DesignSystem.severityColor(severity).g, DesignSystem.severityColor(severity).b, 0.45)
    implicitHeight: 24
    implicitWidth: label.implicitWidth + 16

    Label {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: DesignSystem.severityColor(root.severity)
        font.pixelSize: 12
        font.bold: true
    }
}
