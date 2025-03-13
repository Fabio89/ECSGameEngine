export module Engine.Component.Name;
import Engine.ComponentRegistry;
import Engine.Config;
import Engine.Core;
import Engine.World;

export struct NameComponent : Component<NameComponent>
{
    std::string name{""};
};

template <>
NameComponent deserialize(const JsonObject& data)
{
    NameComponent ret;
    if (auto it = data.FindMember("name"); it != data.MemberEnd())
    {
        ret.name = it->value.GetString();
    }
    return ret;
}