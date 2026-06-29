export module Editor.EntityEditingMode;
import Core;

export enum class EntityEditingMode : UInt8
{
    Translate = 0,
    Rotate = 1,
    Scale = 2,
    None = 3
};