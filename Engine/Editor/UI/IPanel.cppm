export module UI.IPanel;

export class IPanel
{
public:
    virtual ~IPanel() = default;
    virtual void draw() = 0;
};