import QtQuick
import ark.controls as ArkControls
import ark.tokens as ArkTokens

ArkControls.Panel {
    id: root
    // Native Panel.Description reads a global token, so use the public slot.
    descriptionItem: description.length ? sharedDescription : null
    property Component sharedDescription: SettingsBody {
        text: root.description
        wrapMode: root.wrapMode
        textFormat: Text.PlainText
    }
    // Preserve native geometry and behavior, with local typography tokens.
    type: ({container: ArkTokens.Panel.primary.container,
        icon: ArkTokens.Panel.primary.icon,
        content: {container: ArkTokens.Panel.primary.content.container,
            label: {fill: "black", typography: Typography.normal},
            description: {fill: "black", typography: Typography.normal}}})
}
