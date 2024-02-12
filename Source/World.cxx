#include "World.hxx"
#include "AssetTools.hxx"

namespace Asset::Map
{
    EXTERN_ASSET(Floor0)
    EXTERN_ASSET(Floor1)
    EXTERN_ASSET(Floor2)
    EXTERN_ASSET(Floor3)
}

void SWorld::Init()
{
    StartInfo.POV.Coords = { 6, 5 };

    auto LoadLevel = [&](const SAsset& Asset, size_t Index) {
        Serialization::MemoryStream LevelStream(Asset.SignedCharPtr(), Asset.Length);
        Levels[Index].Deserialize(LevelStream);
    };

    LoadLevel(Asset::Map::Floor0, 0);
    LoadLevel(Asset::Map::Floor1, 1);
    LoadLevel(Asset::Map::Floor2, 2);
    LoadLevel(Asset::Map::Floor3, 3);
}

void SWorld::Update(float DeltaTime)
{
    auto Level = GetLevel();

    Level->DoorInfo.Timeline.Advance(DeltaTime);
}
