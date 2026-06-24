export module Render.Primitives;
import Core;
import Math;
import Render.Model;

export namespace Primitives
{
    MeshData generateCone(float radius, float height, int segments);
}

MeshData Primitives::generateCone(float radius, float height, int segments)
{
    MeshData mesh;
    const float angleStep = 2.0f * Math::pi<float>() / segments;

    // Tip of the cone
    const Vertex tip{.pos = {0.0f, height, 0.0f}, .uv = {0.5f, 1.0f}};
    mesh.vertices.push_back(tip);

    // Base circle
    for (int i = 0; i <= segments; ++i)
    {
        // Include one extra vertex to close the circle
        const float angle = static_cast<float>(i) * angleStep;
        const float x = radius * Math::cos(angle);
        const float z = radius * Math::sin(angle);
        mesh.vertices.push_back({.pos = {x, 0.0f, z}, .uv = {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f}});
    }

    // Optional: Add center of the base for the disk
    mesh.vertices.push_back({.pos = {0.0f, 0.0f, 0.0f}, .uv = {0.5f, 0.5f}});

    // Cone side (connect tip to every segment)
    for (int i = 1; i <= segments; ++i)
    {
        // Vertex 0 is the tip
        mesh.indices.push_back(0); // Tip of the cone
        mesh.indices.push_back(i); // Current base vertex
        mesh.indices.push_back(i + 1); // Next base vertex
    }

    // Optional: Base disk (triangle fan)
    unsigned int baseCenterIndex = segments + 2; // After all base vertices
    for (int i = 1; i <= segments; ++i)
    {
        mesh.indices.push_back(baseCenterIndex); // Center of the base
        mesh.indices.push_back(i + 1); // Current base vertex
        mesh.indices.push_back(i); // Previous vertex
    }

    return mesh;
}
