#include "Audio.hxx"

#include <SDL3/SDL_rwops.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <cstring>
#include "AssetTools.hxx"
#include "Log.hxx"

namespace Asset::Common
{
    EXTERN_ASSET(Tile_01WAV)
    EXTERN_ASSET(TestMusicWAV)
}

void SDLCALL CAudio::Callback(void* Userdata, struct SDL_AudioStream* Stream, int AdditionalAmount, int TotalAmount)
{
    auto Audio = static_cast<CAudio*>(Userdata);
    if (AdditionalAmount > 0)
    {
        auto Offset = Audio->Buffer.size() - AdditionalAmount;
        SDL_PutAudioStreamData(Stream, Audio->Buffer.data(), AdditionalAmount);

        std::memcpy(Audio->Buffer.data(), static_cast<const void*>(Audio->Buffer.data() + AdditionalAmount), Offset);
        std::memset(static_cast<void*>(Audio->Buffer.data() + Offset), 0, AdditionalAmount);
    }
}

void SSoundClip::Free() const
{
    SDL_free(Ptr);
}

void CAudio::Clear() const
{
    SDL_ClearAudioStream(Stream);
}

void CAudio::Put(const SSoundClip* SoundClip) const
{
    SDL_PutAudioStreamData(Stream, SoundClip->Ptr, static_cast<int>(SoundClip->Length));
}

CAudio::CAudio()
    : AudioSpec({ SDL_AUDIO_S16, 2, 44100 })
{
    std::memset(Buffer.data(), 0, Buffer.size());

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        exit(1);
    }

    SDL_AudioSpec DestSpec;
    DestSpec.freq = AudioSpec.Freq;
    DestSpec.format = AudioSpec.Format;
    DestSpec.channels = AudioSpec.Channels;

    Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &DestSpec, &Callback, this);

    SDL_AudioDeviceID DeviceID = SDL_GetAudioStreamDevice(Stream);
    char* DeviceName = SDL_GetAudioDeviceName(DeviceID);
    SDL_Log("Opened an AudioStream at %s\n", DeviceName);
    SDL_free(DeviceName);

    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(Stream));

    LoadSoundClip(Asset::Common::Tile_01WAV, TestSoundClip);
    LoadSoundClip(Asset::Common::TestMusicWAV, TestMusic);
}

CAudio::~CAudio()
{
    TestSoundClip.Free();
    TestMusic.Free();
    SDL_DestroyAudioStream(Stream);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void CAudio::LoadSoundClip(const SAsset& Asset, SSoundClip& SoundClip) const
{
    auto TestRW = SDL_RWFromConstMem(Asset.AsVoidPtr(), Asset.Length);

    SDL_AudioSpec TempSpec;
    uint32_t TempLength{};
    uint8_t* TempPtr{};
    SDL_LoadWAV_RW(TestRW, 1, &TempSpec, &TempPtr, &TempLength);

    SDL_AudioSpec DestSpec;
    DestSpec.freq = AudioSpec.Freq;
    DestSpec.format = AudioSpec.Format;
    DestSpec.channels = AudioSpec.Channels;

    SDL_ConvertAudioSamples(&TempSpec,
        TempPtr,
        static_cast<int>(TempLength),
        &DestSpec,
        &SoundClip.Ptr,
        &SoundClip.Length);

    SDL_free(TempPtr);
}

void CAudio::TestAudio()
{
    SDL_MixAudioFormat(Buffer.data(), TestSoundClip.Ptr, SDL_AUDIO_S16, SDL_min(Buffer.size(), TestSoundClip.Length), SDL_MIX_MAXVOLUME);
}
