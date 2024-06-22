module Engine.World;

import Engine.Guid;
import Engine.Component.Model;
import Engine.Config;
import Engine.Render.Application;
import Engine.Render.Core;
import std;
import <glm/glm.hpp>;

void World::createObjectsFromConfig()
{
    const auto& cfg = Config::getEngineConfig();

    for (const Json& meshJson : *cfg.find("meshes"))
    {
        auto mesh = std::make_unique<MeshAsset>(meshJson);
        m_loadedAssets.emplace_back(std::move(mesh));
    }

    for (const auto& texJson : *cfg.find("textures"))
    {
        auto texture = std::make_unique<TextureAsset>(texJson);
        m_loadedAssets.emplace_back(std::move(texture));
    }

    for (const auto& modelJson : *cfg.find("models"))
    {
        Guid meshGuid{*modelJson.find("mesh")};
        Guid textureGuid{*modelJson.find("texture")};

        glm::vec3 location{};
        if (modelJson.contains("position"))
        {
            std::array<float, 3> pos = *modelJson.find("position");
            location = {pos[0], pos[1], pos[2]};
        }

        glm::vec3 rotation{};
        if (auto it = modelJson.find("rotation"); it != modelJson.end())
        {
            rotation = *it;
        }

        float scale{1.f};
        if (auto it = modelJson.find("scale"); it != modelJson.end())
        {
            scale = *it;
        }

        auto meshAssetIt = std::ranges::find_if( m_loadedAssets, [&](auto&& texture) {return texture->getGuid() == meshGuid;});
        auto textureAssetIt = std::ranges::find_if(m_loadedAssets, [&](auto&& texture) {return texture->getGuid() == textureGuid;});
        if (meshAssetIt == m_loadedAssets.end() || textureAssetIt == m_loadedAssets.end())
            throw std::runtime_error("Tried to load model that references an unloaded asset!");

        RenderMessages::AddObject command
        {
            .mesh = static_cast<const MeshAsset*>(meshAssetIt->get()),
            .texture = static_cast<const TextureAsset*>(textureAssetIt->get()),
            .location = location,
            .rotation = rotation,
            .scale = scale
        };
        m_applicationState.get().application->requestAddRenderObject(std::move(command));
    }
}

World::World(const ApplicationSettings& settings, ApplicationState& globalState)
    : m_jobSystem{settings.numThreads}
      , m_applicationState{globalState}
{
}

Entity World::createEntity()
{
    Entity entity = m_nextEntity++;
    m_entities.try_emplace(entity);
    return entity;
}

const Archetype& World::readArchetype(const EntitySignature& signature) const
{
    if (auto it = m_archetypes.find(signature); it != m_archetypes.end())
    {
        return it->second;
    }
    static const Archetype invalid;
    return invalid;
}

Archetype& World::editArchetype(const EntitySignature& signature)
{
    return const_cast<Archetype&>(std::as_const(*this).readArchetype(signature));
}

Archetype& World::editOrCreateArchetype(const EntitySignature& signature)
{
    if (!m_archetypes.contains(signature))
    {
        m_archetypes[signature] = Archetype();
    }
    return m_archetypes[signature];
}

void World::removeEntity(Entity entity)
{
    auto it = m_entities.find(entity);
    if (it != m_entities.end())
    {
        editArchetype(it->second).removeEntity(entity);
        m_entities.erase(it);
    }
}

void World::addSystem(std::unique_ptr<System> system)
{
    m_systems.emplace_back(std::move(system));
}

void World::updateSystems(float deltaTime)
{
    for (auto& system : m_systems)
    {
        system->update(deltaTime);
    }
}

ArchetypeChangedObserverHandle World::observeOnComponentAdded(ArchetypeChangedCallback observer)
{
    ArchetypeChangedObserverHandle handle = generateArchetypeObserverHandle();
    m_archetypeChangeObservers.insert_or_assign(handle, std::move(observer));
    return handle;
}

void World::unobserveOnComponentAdded(ArchetypeChangedObserverHandle observerHandle)
{
    m_archetypeChangeObservers.erase(observerHandle);
}
