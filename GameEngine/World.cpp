module Engine.World;

import Engine.Component.Model;
import Engine.Config;
import Engine.Render.Core;
import std;
import <glm/glm.hpp>;

void World::createObjectsFromConfig()
{
    const auto& cfg = Config::getEngineConfig();
    std::map<std::string, MeshId> meshes;
    std::map<std::string, TextureId> textures;

    for (const auto& meshJson : cfg["meshes"])
    {
        MeshData meshData;
        if (meshJson.contains("path"))
        {
            meshData = ModelUtils::loadModel(std::string{meshJson["path"]}.c_str());
        }
        else
        {
            for (const auto& verticesJson : meshJson["vertices"])
            {
                Vertex& vertex = meshData.vertices.emplace_back();
                std::array<float, 3> pos = verticesJson["position"];
                vertex.pos = {pos[0], pos[1], pos[2]};

                std::array<float, 2> uv = verticesJson["uv"];
                vertex.texCoordinates = {uv[0], uv[1]};
            }
            for (const auto& indicesJson : meshJson["indices"])
            {
                meshData.indices.emplace_back(indicesJson);
            }
        }

        MeshId mesh = m_renderObjectManager.createMesh(meshData);
        meshes.emplace(meshJson["name"], mesh);
    }

    for (const auto& texJson : cfg["textures"])
    {
        if (texJson.contains("path"))
        {
            const TextureId texture = m_renderObjectManager.createTexture(std::string{texJson["path"]}.c_str());
            textures.emplace(texJson["name"], texture);
        }
    }

    ModelSystem modelSystem;
    for (const auto& modelJson : cfg["models"])
    {
        std::string meshName = modelJson["mesh"];
        std::string textureName = modelJson["texture"];

        glm::vec3 location{};
        if (modelJson.contains("position"))
        {
            std::array<float, 3> pos = modelJson["position"];
            location = {pos[0], pos[1], pos[2]};
        }

        glm::vec3 rotation{};
        if (modelJson.contains("rotation"))
        {
            std::array<float, 3> rot = modelJson["rotation"];
            rotation = {rot[0], rot[1], rot[2]};
        }

        float scale{1.f};
        if (modelJson.contains("scale"))
        {
            scale = modelJson["scale"];
        }

        m_renderObjectManager.createRenderObject(meshes[meshName], textures[textureName], location, rotation, scale);
    }
}

World::World(const ApplicationSettings& settings, ApplicationState& globalState)
    : m_jobSystem{settings.numThreads}
    , m_renderObjectManager{globalState.application.m_renderObjectManager}
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
