pragma Singleton
import QtQuick

QtObject {
    // Single source for extension UI typography. Do not alter global Ark tokens.
    readonly property int pixelSize: 36
    readonly property string family: "reMarkable Sans"
    readonly property var normal: ({fontFamily: family, fontSize: pixelSize,
        fontWeight: Font.Normal, letterSpacing: 0, lineHeight: 1.25,
        paragraphSpacing: 0, textCase: Font.MixedCase, textDecoration: false})
    readonly property var strong: Object.assign({}, normal, {fontWeight: Font.Bold})
}
