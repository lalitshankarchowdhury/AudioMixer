#include "../audio/audio.h"
#include "../log/log.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <sndfile.h>
#include <stdbool.h>

enum internal_audio_subsystem_failure_statuses {
    IAS_CLEANUP_NORMALLY,
    IAS_MAKE_CONTEXT_CURRENT_FAILURE,
    IAS_CREATE_DEVICE_CONTEXT_FAILURE,
    IAS_OPEN_DEFAULT_DEVICE_FAILURE
};

enum internal_audio_track_failure_statuses {
    IAC_CLEANUP_NORMALLY,
    IAC_OPEN_CLIP_FILE_FAILURE,
    IAC_GENERATE_GLOBAL_SOURCE_FAILURE,
    IAC_GENERATE_CLIP_BUFFER_FAILURE,
    IAC_ALLOCATE_MEMORY_TO_TEMP_CLIP_DATA_BUFFER_FAILURE,
    IAC_READ_CLIP_FILE_COMPLETELY_FAILURE,
    IAC_COPY_CLIP_FILE_DATA_TO_CLIP_BUFFER_FAILURE,
    IAC_ATTACH_CLIP_BUFFER_TO_ITS_SOURCE_FAILURE,
};

static ALCdevice* global_device;
static ALCcontext* global_context;

static void
internal_audio_subsystem_cleanup(int failure_status)
{
    switch (failure_status) {
    case IAS_CLEANUP_NORMALLY:
        alcMakeContextCurrent(NULL);

    case IAS_MAKE_CONTEXT_CURRENT_FAILURE:
        alcDestroyContext(global_context);

    case IAS_CREATE_DEVICE_CONTEXT_FAILURE:
        alcCloseDevice(global_device);

    case IAS_OPEN_DEFAULT_DEVICE_FAILURE:
        break;
    }
}

static void
internal_audio_clip_cleanup(int failure_status, Clip* clip)
{
    switch (failure_status) {
    case IAC_CLEANUP_NORMALLY:
    case IAC_ATTACH_CLIP_BUFFER_TO_ITS_SOURCE_FAILURE:
    case IAC_COPY_CLIP_FILE_DATA_TO_CLIP_BUFFER_FAILURE:
    case IAC_READ_CLIP_FILE_COMPLETELY_FAILURE:
    case IAC_ALLOCATE_MEMORY_TO_TEMP_CLIP_DATA_BUFFER_FAILURE:
        sf_close(clip->file);

    case IAC_OPEN_CLIP_FILE_FAILURE:
        alDeleteSources(1, &clip->source);

    case IAC_GENERATE_GLOBAL_SOURCE_FAILURE:
        alDeleteBuffers(1, &clip->buffer);

    case IAC_GENERATE_CLIP_BUFFER_FAILURE:
        break;
    }
}

int audioInitSubsystem()
{
    log_info("Initialize audio subsystem");

    global_device = alcOpenDevice(NULL);

    if (global_device == NULL) {
        log_error("Failed to open default audio device");

        internal_audio_subsystem_cleanup(IAS_OPEN_DEFAULT_DEVICE_FAILURE);

        return AUDIO_FAILURE;
    }

    global_context = alcCreateContext(global_device, NULL);

    if (global_context == NULL) {
        log_error("Failed to create device context");

        internal_audio_subsystem_cleanup(IAS_CREATE_DEVICE_CONTEXT_FAILURE);

        return AUDIO_FAILURE;
    }

    if (alcMakeContextCurrent(global_context) == ALC_FALSE) {
        log_error("Failed to make context current");

        internal_audio_subsystem_cleanup(IAS_MAKE_CONTEXT_CURRENT_FAILURE);

        return AUDIO_FAILURE;
    }

    return AUDIO_SUCCESS;
}

int audioLoadClip(Clip* clip, char const* clip_file_name)
{
    log_info("Load audio clip: %s", clip_file_name);

    // Clear old error state
    alGetError();

    alGenBuffers(1, &clip->buffer);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to generate clip buffer");

        internal_audio_clip_cleanup(IAC_GENERATE_CLIP_BUFFER_FAILURE, clip);

        return AUDIO_FAILURE;
    }

    alGenSources(1, &clip->source);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to generate global source");

        internal_audio_clip_cleanup(IAC_GENERATE_GLOBAL_SOURCE_FAILURE, clip);

        return AUDIO_FAILURE;
    }

    SF_INFO clip_file_info;

    clip->file = sf_open(clip_file_name, SFM_READ, &clip_file_info);

    if (clip->file == NULL) {
        log_error("Failed to open clip file");

        internal_audio_clip_cleanup(IAC_OPEN_CLIP_FILE_FAILURE, clip);

        return AUDIO_FAILURE;
    }

    // Set audio clip info
    clip->frames = clip_file_info.frames;
    clip->sample_rate = clip_file_info.samplerate;
    clip->channels = clip_file_info.channels;

    int bit_depth;

    switch (clip_file_info.format & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_U8:
        bit_depth = 8;

        clip->format = (clip->channels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;

        break;

    default:
    case SF_FORMAT_PCM_16:
        bit_depth = 16;

        clip->format = (clip->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

        break;

    case SF_FORMAT_FLOAT:
        bit_depth = 32;

        clip->format = (clip->channels == 1) ? AL_FORMAT_MONO_FLOAT32
                                             : AL_FORMAT_STEREO_FLOAT32;

        break;

    case SF_FORMAT_DOUBLE:
        bit_depth = 64;

        clip->format = (clip->channels == 1) ? AL_FORMAT_MONO_DOUBLE_EXT
                                             : AL_FORMAT_STEREO_DOUBLE_EXT;

        break;
    }

    // Allocate memory of size frames * channels * bytes per-sample
    void* clip_file_data = calloc(clip->frames * clip->channels, bit_depth / 8);

    if (clip_file_data == NULL) {
        log_error("Failed to allocate memory to temporary clip data buffer");

        internal_audio_clip_cleanup(
            IAC_ALLOCATE_MEMORY_TO_TEMP_CLIP_DATA_BUFFER_FAILURE, clip);

        return AUDIO_FAILURE;
    }

    if (sf_readf_double(clip->file, clip_file_data, clip->frames) != clip->frames) {
        log_error("Failed to read clip file completely");

        internal_audio_clip_cleanup(IAC_READ_CLIP_FILE_COMPLETELY_FAILURE, clip);

        return AUDIO_FAILURE;
    }

    alBufferData(clip->buffer,
        clip->format,
        clip_file_data,
        clip->frames * clip->channels * bit_depth / 8,
        clip->sample_rate);

    free(clip_file_data);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to copy clip file data to clip buffer");

        internal_audio_clip_cleanup(IAC_COPY_CLIP_FILE_DATA_TO_CLIP_BUFFER_FAILURE,
            clip);

        return AUDIO_FAILURE;
    }

    log_info("Audio clip loaded: %ld frames, %dHz, %s audio",
        clip->frames,
        clip->sample_rate,
        (clip->channels == 1) ? "Mono" : "Stereo");

    alSourcei(clip->source, AL_BUFFER, clip->buffer);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to attach clip buffer to its source");

        internal_audio_clip_cleanup(IAC_ATTACH_CLIP_BUFFER_TO_ITS_SOURCE_FAILURE,
            clip);

        return AUDIO_FAILURE;
    }

    return AUDIO_SUCCESS;
}

int audioPlayClips(Clip clips[], int num_clips)
{
    for (int i = 0; i < num_clips; ++i) {
        alSourcePlayv(1, &clips[i].source);

        if (alGetError() != AL_NO_ERROR) {
            log_error("Failed to play audio clip");

            return AUDIO_FAILURE;
        }
    }

    return AUDIO_SUCCESS;
}

bool audioIsClipPlaying(Clip* clip)
{
    ALenum source_state;

    alGetSourcei(clip->source, AL_SOURCE_STATE, &source_state);

    if (source_state != AL_PLAYING) {
        return false;
    }

    return true;
}

void audioUnloadClip(Clip* clip)
{
    log_info("Unload audio clip");

    internal_audio_clip_cleanup(IAC_CLEANUP_NORMALLY, clip);
}

void audioQuitSubsystem()
{
    log_info("Quit audio subsystem");

    internal_audio_subsystem_cleanup(IAS_CLEANUP_NORMALLY);
}
