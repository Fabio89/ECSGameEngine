export module Editor.EditingContextId;
import Core;

export struct EditingContextId : Id<struct EditingContextIdTag> {};

template<>
struct std::hash<EditingContextId>
{
    constexpr std::size_t operator()(const EditingContextId& id) const noexcept
    {
        return std::hash<Id<EditingContextIdTag>>{}(id);
    }
};