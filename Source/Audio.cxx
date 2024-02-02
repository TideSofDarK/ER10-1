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

void SDLCALL CAudio::Callback(void* Userdata, struct SDL_AudioStream* Stream, int AdditionalAmount, [[maybe_unused]] int TotalAmount)
{
    auto Audio = static_cast<CAudio*>(Userdata);
    if (AdditionalAmount > 0)
    {
        auto* Data = SDL_stack_alloc(uint8_t, AdditionalAmount);
        std::memset(Data, Audio->Silence, AdditionalAmount);
        for (auto& AudioEntry : Audio->Queue)
        {
            if (AudioEntry.IsPlaying())
            {
                int MixedTotal = 0;
                int Remaining = 1;
                while (MixedTotal < AdditionalAmount && Remaining > 0)
                {
                    auto ToMix = AdditionalAmount - MixedTotal;
                    int Current = AudioEntry.Current;
                    int Mixed{};
                    Remaining = AudioEntry.Advance(ToMix, &Mixed);
                    SDL_MixAudioFormat(Data, AudioEntry.SoundClip->Ptr + Current, SDL_AUDIO_S16, Mixed, SDL_MIX_MAXVOLUME * Audio->Volume);
                    MixedTotal += Mixed;
                }
            }
        }
        SDL_PutAudioStreamData(Stream, Data, AdditionalAmount);
        SDL_stack_free(Data);
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

CAudio::CAudio()
    : AudioSpec({ SDL_AUDIO_S16, 2, 44100 })
{
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
    Silence = SDL_GetSilenceValueForFormat(DestSpec.format);

    SDL_AudioDeviceID DeviceID = SDL_GetAudioStreamDevice(Stream);
    char* DeviceName = SDL_GetAudioDeviceName(DeviceID);
    SDL_Log("Opened an AudioStream at %s\n", DeviceName);
    SDL_free(DeviceName);

    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(Stream));

    LoadSoundClip(Asset::Common::Tile_01WAV, TestSoundClip);
    LoadSoundClip(Asset::Common::TestMusicWAV, TestMusic);

    Queue[1].SoundClip = &TestMusic;
    Queue[1].bLoop = true;
    Queue[1].Current = 0;
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
    auto TestRW = SDL_RWFromConstMem(Asset.VoidPtr(), Asset.Length);

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
    Play(TestSoundClip);
}

void CAudio::Play(const SSoundClip& SoundClip)
{
    for (auto& AudioEntry : Queue)
    {
        if (!AudioEntry.IsPlaying())
        {
            AudioEntry.SoundClip = &SoundClip;
            AudioEntry.bLoop = false;
            AudioEntry.Current = 0;
            return;
        }
    }
    Log::Audio<ELogLevel::Critical>("Attempt to play a SoundClip while the queue is full!", "");
}
