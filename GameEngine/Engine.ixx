export module Engine.Core;

import std;
import Engine.Job;

export struct EngineSettings
{
	static constexpr int MaxComponentsPerEntity = 64;

	int numThreads{ 4 };

	float targetFps{ 120.f };
};

export using Entity = size_t;

export template<typename T>
struct Component
{
	static const std::type_index TypeID;
};

template<typename T>
const std::type_index Component<T>::TypeID(typeid(T));

export class System
{
public:
	virtual ~System() = default;
	void update(float deltaTime);
	void addUpdateFunction(std::function<void(float)> func);
private:
	std::vector<std::function<void(float)>> m_updateFunctions;
};

using EntitySignature = std::bitset<EngineSettings::MaxComponentsPerEntity>;

// Archetype
class Archetype
{
public:
	bool isEmpty() const { return m_componentArrays.empty(); }

	template<typename T>
	void addComponent(Entity entity, T component);

	template<typename T>
	const T& readComponent(Entity entity) const;

	void removeEntity(Entity entity);

private:
	class ComponentArrayBase
	{
	public:
		virtual ~ComponentArrayBase() = default;
		virtual void remove(Entity entity) = 0;
		virtual bool isEmpty() const = 0;
	};

	template<typename T>
	class ComponentArray : public ComponentArrayBase
	{
	public:
		bool isEmpty() const override { return m_components.empty(); }

		void insert(Entity entity, T component);

		void remove(Entity entity) override;

		const T& get(Entity entity) const;
		T& get(Entity entity);

	private:
		std::vector<T> m_components;
		std::unordered_map<Entity, size_t> m_entityToIndex;
		std::unordered_map<size_t, Entity> indexToEntity;
	};

	std::unordered_map<std::type_index, std::unique_ptr<ComponentArrayBase>> m_componentArrays;
};

export class World
{
public:
	World();

	World(const EngineSettings&);

	Entity createEntity();

	template<typename T, typename... Args>
	void addComponent(Entity entity, Args&&... args);

	template<typename T>
	const T& readComponent(Entity entity) const;

	template<typename T>
	T& editComponent(Entity entity);

	void removeEntity(Entity entity);

	void addSystem(std::unique_ptr<System> system);

	void updateSystems(float deltaTime);

private:
	JobSystem m_jobSystem;
	Entity m_nextEntity = 0;
	std::unordered_map<Entity, EntitySignature> m_entities;
	std::unordered_map<EntitySignature, Archetype> m_archetypes;
	std::vector<std::unique_ptr<System>> m_systems;

	const Archetype& readArchetype(const EntitySignature& signature) const;
	Archetype& editArchetype(const EntitySignature& signature);
	Archetype& editOrCreateArchetype(const EntitySignature& signature);
};


// Definitions

template<typename T>
void Archetype::ComponentArray<T>::insert(Entity entity, T component)
{
	if (m_entityToIndex.contains(entity))
	{
		size_t index = m_entityToIndex[entity];
		m_components[index] = component;
	}
	else
	{
		size_t index = m_components.size();
		m_components.push_back(component);
		m_entityToIndex[entity] = index;
		indexToEntity[index] = entity;
	}
}

template<typename T>
void Archetype::ComponentArray<T>::remove(Entity entity)
{
	size_t index = m_entityToIndex[entity];
	size_t lastIndex = m_components.size() - 1;
	m_components[index] = m_components[lastIndex];
	Entity lastEntity = indexToEntity[lastIndex];
	m_entityToIndex[lastEntity] = index;
	indexToEntity[index] = lastEntity;
	m_components.pop_back();
	m_entityToIndex.erase(entity);
	indexToEntity.erase(lastIndex);
}

template<typename T>
const T& Archetype::ComponentArray<T>::get(Entity entity) const
{
	if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
	{
		return m_components.at(indexIt->second);
	}
	static const T invalid;
	return invalid;
}

template<typename T>
T& Archetype::ComponentArray<T>::get(Entity entity)
{
	return const_cast<T&>(std::as_const(*this).get(entity));
}

template<typename T>
void Archetype::addComponent(Entity entity, T component)
{
	auto& arr = m_componentArrays[Component<T>::TypeID];
	if (!arr)
		arr = std::make_unique<ComponentArray<T>>();

	static_cast<ComponentArray<T>*>(arr.get())->insert(entity, component);
}

template<typename T>
const T& Archetype::readComponent(Entity entity) const
{
	static const auto type = Component<T>::TypeID;
	if (auto it = m_componentArrays.find(type); it != m_componentArrays.end())
	{
		return static_cast<const ComponentArray<T>*>(it->second.get())->get(entity);
	}
	static const T invalid;
	return invalid;
}

template<typename T, typename... Args>
void World::addComponent(Entity entity, Args&&... args)
{
	EntitySignature& signature = m_entities[entity];
	Archetype& oldArchetype = editArchetype(signature);
	oldArchetype.removeEntity(entity);
	if (oldArchetype.isEmpty())
	{
		m_archetypes.erase(signature);
	}

	signature.set(Component<T>::TypeID.hash_code() % EngineSettings::MaxComponentsPerEntity);
	editOrCreateArchetype(signature).addComponent<T>(entity, T(std::forward<Args>(args)...));
}

template<typename T>
const T& World::readComponent(Entity entity) const
{
	if (auto it = m_entities.find(entity); it != m_entities.end())
	{
		return readArchetype(it->second).readComponent<T>(entity);
	}
	static const T invalid;
	return invalid;
}

template<typename T>
T& World::editComponent(Entity entity)
{
	return const_cast<T&>(readComponent<T>(entity));
}
