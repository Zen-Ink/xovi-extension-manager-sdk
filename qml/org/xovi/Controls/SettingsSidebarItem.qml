import QtQuick
import ark.controls as ArkControls
import ark.tokens as ArkTokens

ArkControls.SidebarItem {
    // Clone only the label branch; never mutate xochitl's global theme.
    type: ({
        item: Object.assign({}, ArkTokens.Sidebar.primary.item, {
            content: Object.assign({}, ArkTokens.Sidebar.primary.item.content, {
                label: Object.assign({}, ArkTokens.Sidebar.primary.item.content.label, {
                    idle: Object.assign({}, ArkTokens.Sidebar.primary.item.content.label.idle, {
                        typography: Typography.normal
                    })
                })
            })
        }),
        foldout: Object.assign({}, ArkTokens.Sidebar.primary.foldout, {
            item: Object.assign({}, ArkTokens.Sidebar.primary.foldout.item, {
                content: Object.assign({}, ArkTokens.Sidebar.primary.foldout.item.content, {
                    description: Object.assign({}, ArkTokens.Sidebar.primary.foldout.item.content.description, {
                        idle: Object.assign({}, ArkTokens.Sidebar.primary.foldout.item.content.description.idle, {
                            typography: Typography.normal
                        })
                    })
                })
            })
        })
    })
}
