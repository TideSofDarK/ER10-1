#pragma once

#include <array>
#include "AssetTools.hxx"
#include "SDL3/SDL_audio.h"

struct SAudioSpec
{
    SDL_AudioFormat Format;
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

struct SAudioEntry
{
    const SSoundClip* SoundClip{};
    int Current{};
    bool bLoop{};

    [[nodiscard]] inline bool IsPlaying() const { return IsValid() && Current < SoundClip->Length; }
    [[nodiscard]] inline bool IsValid() const { return SoundClip && SoundClip->Ptr != nullptr; }
    int Advance(int Samples, int* Mixed)
    {
        if (IsValid())
        {
            *Mixed = std::min(Samples, SoundClip->Length - Current);
            Current += Samples;
            if (Current >= SoundClip->Length)
            {
                if (bLoop)
                {
                    auto Remaining = Samples - (SoundClip->Length - Current);
                    Current = 0;
                    return Remaining;
                }
                else
                {
                    Current = SoundClip->Length;
                    return 0;
                }
            }
        }
        return 0;
    }
};

struct SAudio
{
protected:
    std::array<SAudioEntry, 8> Queue{};
    SAudioSpec AudioSpec{};
    int Silence{};
    struct SDL_AudioStream* Stream{};
    SSoundClip TestSoundClip{};
    SSoundClip TestMusic{};

    void Clear() const;

public:
    float Volume = 0.30f;

    void Init();
    void Cleanup();

    static void Callback(void* Userdata, struct SDL_AudioStream* Stream, int AdditionalAmount, int TotalAmount);
    void LoadSoundClip(const SAsset& Asset, SSoundClip& SoundClip) const;
    void TestAudio();
    void Play(const SSoundClip& SoundClip);
};
