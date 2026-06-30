export module Editor.PanelCreateInfo;
export import Editor.EditingContextId;
export import Window;

export struct PanelCreateInfo
{
    EditingContextId contextId;
    WindowHandle window;
};