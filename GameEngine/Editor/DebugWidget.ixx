export module Engine.DebugWidget;
import Engine.ApplicationState;
import Engine.World;

export class DebugWidget
{
public:
    virtual ~DebugWidget() = default;
    virtual void draw(World& world) = 0;
};

export template<typename T>
void drawDebugWidget(T& item)
{
    
}
