export module UI.IWidget;

export class IWidget
{
public:
    virtual ~IWidget() = default;
    virtual void draw() = 0;
};