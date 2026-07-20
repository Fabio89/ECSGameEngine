export module Editor.PanelCreateInfo;
export import Editor.EditingContextId;
export import Window;

export struct PanelCreateInfo
{
    EditingContextId contextId;
    WindowHandle window;
};

export template<typename T>
concept ControllerTraits =
requires
{
    typename T::Snapshot;
    typename T::Request;
    typename T::SharedMailbox;
    { T::createMailbox() } -> std::same_as<typename T::SharedMailbox>;
};

export struct NoController
{
    struct Snapshot {};
    struct Request {};
    struct Mailbox {};
    using SharedMailbox = Mailbox;
    static SharedMailbox createMailbox() { return {}; }
};