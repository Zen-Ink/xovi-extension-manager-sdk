import QtQuick
import ark.controls as ArkControls

ArkControls.Label {
    typography: (type === ArkControls.Label.LargeStrong || type === ArkControls.Label.SmallStrong || type === ArkControls.Label.ExtraLargeStrong) ? Typography.strong : Typography.normal
}
