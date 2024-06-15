module Engine.Core;

void System::update(float deltaTime)
{
	for (auto& func : m_updateFunctions)
	{
		func(deltaTime);
	}
}

void System::addUpdateFunction(std::function<void(float)> func)
{
	m_updateFunctions.push_back(std::move(func));
}

void Archetype::removeEntity(Entity entity)
{
	std::vector<std::type_index> toRemove;
	for (auto& pair : m_componentArrays)
	{
		pair.second->remove(entity);
		if (pair.second->isEmpty())
		{
			toRemove.push_back(pair.first);
		}
	}

	for (std::type_index index : toRemove)
	{
		m_componentArrays.erase(index);
	}
}

World::World()
	: World{ EngineSettings{} }
{
}

World::World(const EngineSettings& settings)
	: m_jobSystem{ settings.numThreads }
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
	if (!m_archetypes.contains(signature)) {
		m_archetypes[signature] = Archetype();
	}
	return m_archetypes[signature];
}

void World::removeEntity(Entity entity)
{
	auto it = m_entities.find(entity);
	if (it != m_entities.end()) {
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
	for (auto& system : m_systems) {
		system->update(deltaTime);
	}
}