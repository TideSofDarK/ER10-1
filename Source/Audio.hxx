#pragma once

#include "AssetTools.hxx"

struct SAudioSpec
{
    uint16_t Format;
    int Channels;
    int Freq;
};

struct SSoundClip
{
    int Length{};
    uint8_t* Ptr{};

    void Free() const;
};

struct SMusic : SSoundClip
{

};

class CAudio
{
protected:
    std::array<uint8_t, 1024 * 1024> Buffer{};
    SAudioSpec AudioSpec;
    struct SDL_AudioStream* Stream{};
    SSoundClip TestSoundClip{};
    SSoundClip TestMusic{};

    void Clear() const;
    void Put(const SSoundClip* SoundClip) const;

public:
    CAudio();
    ~CAudio();

    static void Callback(void* Userdata, struct SDL_AudioStream* Stream, int AdditionalAmount, int TotalAmount);
    void LoadSoundClip(const SAsset& Asset, SSoundClip& SoundClip) const;
    void TestAudio();
};
