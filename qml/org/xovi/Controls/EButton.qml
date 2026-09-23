import QtQuick
import QtQuick.Layouts
import ark.controls as ArkControls
import ark.tokens as ArkTokens

ArkControls.Button {
    id: root
    property string iconName: ""
    property string iconBase: "qrc:/xovi/controls/icons/"
    readonly property color contentColor: !enabled ? "#808080" : checked ? "white" : "black"
    iconSource: iconName ? iconBase + iconName + ".svg" : ""
    type: ArkTokens.Button.secondary
    font.pixelSize: Typography.pixelSize
    font.family: Typography.family
    font.weight: Typography.weight
    font.underline: false
    font.strikeout: false
    implicitHeight: 80
    implicitWidth: Math.max(80, row.implicitWidth + 32)
    padding: 12
    opacity: 1
    background: Rectangle {
        color: root.enabled && root.checked ? "black" : "white"
        border.color: root.contentColor
        border.width: 2
        radius: 8
    }
    contentItem: RowLayout {
        id: row
        spacing: 12
        ArkControls.Icon {
            objectName: "buttonIcon"
            color: root.contentColor
            size: 48
            visible: root.iconSource.toString().length > 0
            source: root.iconSource
            Layout.preferredWidth: 48; Layout.preferredHeight: 48
        }
        ELabel {
            objectName: "buttonLabel"
            color: root.contentColor
            visible: root.text.length > 0
            text: root.text
            Layout.fillWidth: true
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }
    }
    // Device Qt is built without Accessibility attached properties.
    property string description: text || iconName
}
