import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    objectName: "paginationFooter"
    property int pageIndex: 0
    property int pageCount: 1
    signal pageRequested(int index)
    implicitHeight: buttons.implicitHeight + 16
    implicitWidth: buttons.implicitWidth + 32
    color: "white"
    onPageCountChanged: if (pageIndex >= pageCount) pageRequested(Math.max(0, pageCount - 1))
    Rectangle { anchors.top: parent.top; width: parent.width; height: 2; color: "black" }
    RowLayout {
        id: buttons
        anchors.fill: parent
        anchors.margins: 8
        spacing: 16
        EButton { objectName: "previousButton"; iconName: "back"; description: qsTr("Previous page"); enabled: root.pageIndex > 0; onClicked: root.pageRequested(root.pageIndex - 1) }
        ELabel { text: (root.pageIndex + 1) + " / " + Math.max(1, root.pageCount); Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
        EButton { objectName: "nextButton"; iconName: "next"; description: qsTr("Next page"); enabled: root.pageIndex + 1 < root.pageCount; onClicked: root.pageRequested(root.pageIndex + 1) }
    }
}
