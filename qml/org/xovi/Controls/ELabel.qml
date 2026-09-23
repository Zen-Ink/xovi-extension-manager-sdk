import QtQuick
Text {
    property bool emphasized: false
    color: "black"
    font.pixelSize: Typography.pixelSize
    font.family: Typography.family
    font.weight: emphasized ? Typography.strong.fontWeight : Typography.weight
    wrapMode: Text.Wrap
    verticalAlignment: Text.AlignVCenter
}
