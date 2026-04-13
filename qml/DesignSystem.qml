pragma Singleton

import QtQuick

QtObject {
    readonly property color windowBg: "#171a1f"
    readonly property color surface: "#1e2229"
    readonly property color surfaceRaised: "#242a33"
    readonly property color surfaceSoft: "#20262e"
    readonly property color stroke: "#343b46"
    readonly property color strokeMuted: "#2a313b"
    readonly property color textPrimary: "#e4e7ec"
    readonly property color textSecondary: "#9aa3af"
    readonly property color textMuted: "#7f8894"
    readonly property color accent: "#7f90a9"
    readonly property color healthy: "#6f8f86"
    readonly property color warning: "#b08a57"
    readonly property color critical: "#a8635f"

    readonly property int railExpanded: 232
    readonly property int railCompact: 72
    readonly property int radiusSmall: 10
    readonly property int radiusMedium: 14
    readonly property int radiusLarge: 18

    readonly property int space4: 4
    readonly property int space8: 8
    readonly property int space12: 12
    readonly property int space16: 16
    readonly property int space20: 20
    readonly property int space24: 24
    readonly property int space32: 32

    readonly property int titleSize: 24
    readonly property int sectionTitleSize: 18
    readonly property int bodySize: 13
    readonly property int valueSize: 22
    readonly property int monoSize: 12

    function severityColor(severity) {
        if (severity === "critical")
            return critical
        if (severity === "warning")
            return warning
        return healthy
    }
}

