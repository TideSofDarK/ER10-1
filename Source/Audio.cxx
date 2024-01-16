#include "Audio.hxx"

#include <SDL3/SDL_rwops.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <cstdio>
#include "AssetTools.hxx"

namespace Asset::Common
{
    EXTERN_ASSET(Tile_01WAV)
}

namespace Audio
{
    struct SSoundClip
    {
        friend class SAudioStream;

    private:
        SDL_AudioSpec Spec{};
        uint32_t Length{};
        uint8_t* Ptr{};

    public:
        void Load(const SAsset& Asset)
        {
            auto TestRW = SDL_RWFromConstMem(Asset.AsVoidPtr(), Asset.Length);
            SDL_LoadWAV_RW(TestRW, 1, &Spec, &Ptr, &Length);
        }

        void Free() const
        {
            SDL_free(Ptr);
        }

        [[nodiscard]] int Channels() const
        {
            return Spec.channels;
        }
    };

    struct SAudioStream
    {
        const SDL_AudioSpec AudioSpec;
        SDL_AudioStream* Stream{};

        explicit SAudioStream(int Channels) : AudioSpec({ SDL_AUDIO_S16, Channels, 44100 }) {};

        void Open()
        {
            Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &AudioSpec, nullptr, nullptr);
            SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(Stream));
        }

        void Close() const
        {
            SDL_DestroyAudioStream(Stream);
        }

        void Clear() const {
            SDL_ClearAudioStream(Stream);
        }

        void Put(const SSoundClip& SoundClip) const
        {
            SDL_PutAudioStreamData(Stream, SoundClip.Ptr, static_cast<int>(SoundClip.Length));
        }
    };

    SAudioStream MonoStream(1);
    SAudioStream StereoStream(2);

    SSoundClip TestSoundClip;

    void Init()
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0)
        {
            int i, num_devices;
            SDL_AudioDeviceID* devices = SDL_GetAudioOutputDevices(&num_devices);
            if (devices)
            {
                for (i = 0; i < num_devices; ++i)
                {
                    SDL_AudioDeviceID instance_id = devices[i];
                    char* name = SDL_GetAudioDeviceName(instance_id);
                    SDL_Log("AudioDevice %" SDL_PRIu32 ": %s\n", instance_id, name);
                    SDL_free(name);
                }
                SDL_free(devices);
            }
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        }

        TestSoundClip.Load(Asset::Common::Tile_01WAV);

        MonoStream.Open();
        StereoStream.Open();
    }

    void Shutdown()
    {
        TestSoundClip.Free();
        MonoStream.Close();
        StereoStream.Close();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    void TestAudio()
    {
        const auto& Stream = TestSoundClip.Channels() == 1 ? MonoStream : StereoStream;
        std::printf("dodik %d\n", SDL_GetAudioStreamAvailable(Stream.Stream));
        Stream.Clear();
        Stream.Put(TestSoundClip);
    }
}
