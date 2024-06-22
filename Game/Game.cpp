import Engine.Config;
import Engine.Core;
import Engine.Render;
import Engine.World;

import std;

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

void gameInit(World& world)
{
	// // Create entities
	// Entity entity1 = world.createEntity();
	// world.addComponent<Position>(entity1, 0.0f, 0.0f);
	// world.addComponent<Velocity>(entity1, 1.0f, 1.0f);
	//
	// // Create systems
	// auto movementSystem = std::make_unique<MovementSystem>();
	// movementSystem->addEntity(entity1, world);
	// world.addSystem(std::move(movementSystem));

	world.createObjectsFromConfig();
}


void gameShutdown()
{
	std::cout << "[Game] Shutdown complete.\n";
}

int main()
{
	const ApplicationSettings& settings = Config::getApplicationSettings();
	ApplicationState globalState;

	World world{ settings, globalState };
	const LoopSettings loopSettings{ .targetFps = settings.targetFps };

	std::thread renderThread = runRenderThread(loopSettings, globalState);

	while(!globalState.initialized)
	{
		//std::cout << "Waiting for render thread...\n";
	}
	
	gameInit(world);

	auto gameTick = [&world](float deltaTime)
	{
		world.updateSystems(deltaTime);
	};

	auto shouldRun = [&globalState] { return !globalState.closing; };

	performLoop(loopSettings, gameTick, shouldRun);

	gameShutdown();
	renderThread.join();

	return 0;
}