import QtQuick
import ark.controls as ArkControls

ArkControls.Body {
    typography: type === ArkControls.Body.MediumStrong ? Typography.strong : Typography.normal
}
