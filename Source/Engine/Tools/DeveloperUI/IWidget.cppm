export module DevUI.IDebugWidget;

export class IWidget
{
public:
    virtual ~IWidget() = default;
    virtual void draw() = 0;
};