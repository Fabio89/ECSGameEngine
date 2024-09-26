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
NameComponent deserialize(const Json& data)
{
    NameComponent ret;
    if (auto it = data.find("name"); it != data.end())
    {
        ret.name = *it;
    }
    return ret;
}