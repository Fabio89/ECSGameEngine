export module AssetLoader.Mesh;
export import Assets.Mesh;
export import Assets.Texture;
import AssetLoader;
import AssetManager;
import Serialization.Json;
import std;

import Assets.Texture;

export class MeshAssetLoader : public AssetLoader<MeshData>
{
public:
    MeshData loadFromFile(const std::filesystem::path& path) override;
};

export class TextureAssetLoader : public AssetLoader<TextureData>
{
public:
    TextureData loadFromFile(const std::filesystem::path& path) override;
};