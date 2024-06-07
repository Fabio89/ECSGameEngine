import Engine.Core;

#include <chrono>

struct Position : Component<Position> {
	float x{ 0.f }, y{ 0.f };
	Position() = default;
	Position(float x, float y) : x(x), y(y) {}
};

struct Velocity : Component<Velocity> {
	float vx{ 0.f }, vy{ 0.f };
	Velocity() = default;
	Velocity(float vx, float vy) : vx(vx), vy(vy) {}
};

class MovementSystem : public System
{
public:
	void addEntity(Entity entity, World& manager) {
		Position& pos = manager.editComponent<Position>(entity);
		Velocity& vel = manager.editComponent<Velocity>(entity);
		addUpdateFunction([&pos, &vel](float deltaTime) {
			pos.x += vel.vx * deltaTime;
			pos.y += vel.vy * deltaTime;
			});
	}
};

EngineSettings settings
{
.numThreads = 6,
.targetFps = 144.f
};

int main()
{
	World world{ settings };

	// Create entities
	Entity entity1 = world.createEntity();
	world.addComponent<Position>(entity1, 0.0f, 0.0f);
	world.addComponent<Velocity>(entity1, 1.0f, 1.0f);

	// Create systems
	auto movementSystem = std::make_unique<MovementSystem>();
	movementSystem->addEntity(entity1, world);
	world.addSystem(std::move(movementSystem));

	const auto targetFrameDuration = std::chrono::milliseconds{ static_cast<int>(1000 / settings.targetFps) };

	// Game loop
	for (int i = 0; i < 1000; ++i)
	{
		const auto frameStart = std::chrono::high_resolution_clock::now();

		world.updateSystems(1.f / settings.targetFps);

		// Print the position of the entity
		const Position& pos = world.readComponent<Position>(entity1);
		std::cout << "Entity1 Position: (" << pos.x << ", " << pos.y << ")\n";

		const auto frameEnd = std::chrono::high_resolution_clock::now();
		const auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);

		if (frameDuration < targetFrameDuration)
		{
			std::this_thread::sleep_for(targetFrameDuration - frameDuration);
		}
	}

	return 0;
}