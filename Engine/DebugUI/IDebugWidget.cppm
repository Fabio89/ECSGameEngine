export module DebugUI.IDebugWidget;

export class IDebugWidget
{
public:
    virtual ~IDebugWidget() = default;
    virtual void draw() = 0;
};