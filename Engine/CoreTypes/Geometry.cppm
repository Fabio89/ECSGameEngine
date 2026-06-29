export module Geometry;
import Core;
import Math;

export struct Size2D
{
    Int32 width{};
    Int32 height{};
};

export struct Rect
{
    IVec2 position{};
    Size2D size;
};
