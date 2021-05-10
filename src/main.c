#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "include/log.h"
#include "include/AL/al.h"
#include "include/AL/alc.h"
#include "include/AL/efx.h"
#include "include/AL/efx-presets.h"
#include "include/sndfile.h"

void init();
void load_audio();
void play_audio ();
void quit(int failure_status);

ALCdevice* device;
ALCcontext* context;
ALuint buffer[1];
ALuint source[1];
ALuint effect[1];
ALuint auxiliary_effect_slot[1];
enum failure_statuses {
    NORMAL,
    DEFAULT_DEVICE_OPEN_FAILURE,
    CONTEXT_CREATION_FAILURE,
    MAKE_CONTEXT_CURRENT_FAILURE,
    SOURCE_BUFFER_GENERATION_FAILURE,
    AUDIO_SOURCE_GENERATION_FAILURE,
    AUDIO_EFFECT_GENERATION_FAILURE,
    AUXILIARY_EFFECT_SLOT_GENERATION_FAILURE,
    BIND_BUFFER_TO_SOURCE_FAILURE,
    AUDIO_FILE_OPEN_FAILURE,
    ALLOCATE_MEMORY_TO_DATA_BUFFER_FAILURE,
    COPY_DATA_BUFFER_TO_SOURCE_BUFFER_FAILURE
};
SNDFILE* file;
short* data;

int main() {
    init();

    load_audio();

    play_audio();

    quit(NORMAL);
}

void init() {
    log_info("Initialize audio subsystem");

    device = alcOpenDevice(NULL);

    if (device == NULL) {
        log_error("Failed to open default audio device");

        quit(DEFAULT_DEVICE_OPEN_FAILURE);
    }

    ALCint attr_list[2] = { ALC_FREQUENCY, 44100 };

    context = alcCreateContext(device, attr_list);

    if (context == NULL) {
        log_error("Failed to create context");

        quit(CONTEXT_CREATION_FAILURE);
    }

    if (alcMakeContextCurrent(context) == ALC_FALSE) {
        log_error("Failed to make context current");

        quit(MAKE_CONTEXT_CURRENT_FAILURE);
    }

    // Clear error code
    alGetError();

    alGenBuffers(1, buffer);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to generate source buffer");

        quit(SOURCE_BUFFER_GENERATION_FAILURE);
    }

    alGenSources(1, source);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to generate audio source");

        quit(AUDIO_SOURCE_GENERATION_FAILURE);
    }

    alGenEffects(1, effect);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to generate effect");

        quit(AUDIO_EFFECT_GENERATION_FAILURE);
    }

    // Set reverb sound effect
    alEffecti(effect[0], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

    EFXEAXREVERBPROPERTIES reverb_preset = EFX_REVERB_PRESET_DRUGGED;

    // Set reverb properties
    alEffectf(effect[0], AL_EAXREVERB_DENSITY, reverb_preset.flDensity);
    alEffectf(effect[0], AL_EAXREVERB_DIFFUSION, reverb_preset.flDiffusion);
    alEffectf(effect[0], AL_EAXREVERB_GAIN, reverb_preset.flGain);
    alEffectf(effect[0], AL_EAXREVERB_GAINHF, reverb_preset.flGainHF);
    alEffectf(effect[0], AL_EAXREVERB_GAINLF, reverb_preset.flGainLF);
    alEffectf(effect[0], AL_EAXREVERB_DECAY_TIME, reverb_preset.flDecayTime);
    alEffectf(effect[0], AL_EAXREVERB_DECAY_HFRATIO, reverb_preset.flDecayHFRatio);
    alEffectf(effect[0], AL_EAXREVERB_DECAY_LFRATIO, reverb_preset.flDecayLFRatio);
    alEffectf(effect[0], AL_EAXREVERB_REFLECTIONS_GAIN, reverb_preset.flReflectionsGain);
    alEffectf(effect[0], AL_EAXREVERB_REFLECTIONS_DELAY, reverb_preset.flReflectionsDelay);
    alEffectfv(effect[0], AL_EAXREVERB_REFLECTIONS_PAN, reverb_preset.flReflectionsPan);
    alEffectf(effect[0], AL_EAXREVERB_LATE_REVERB_GAIN, reverb_preset.flLateReverbGain);
    alEffectf(effect[0], AL_EAXREVERB_LATE_REVERB_DELAY, reverb_preset.flLateReverbDelay);
    alEffectfv(effect[0], AL_EAXREVERB_LATE_REVERB_PAN,  reverb_preset.flLateReverbPan);
    alEffectf(effect[0], AL_EAXREVERB_ECHO_TIME, reverb_preset.flEchoTime);
    alEffectf(effect[0], AL_EAXREVERB_ECHO_DEPTH, reverb_preset.flEchoDepth);
    alEffectf(effect[0], AL_EAXREVERB_MODULATION_TIME, reverb_preset.flModulationTime);
    alEffectf(effect[0], AL_EAXREVERB_MODULATION_DEPTH, reverb_preset.flModulationDepth);
    alEffectf(effect[0], AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb_preset.flAirAbsorptionGainHF);
    alEffectf(effect[0], AL_EAXREVERB_HFREFERENCE, reverb_preset.flHFReference);
    alEffectf(effect[0], AL_EAXREVERB_LFREFERENCE, reverb_preset.flLFReference);
    alEffectf(effect[0], AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb_preset.flRoomRolloffFactor);
    alEffecti(effect[0], AL_EAXREVERB_DECAY_HFLIMIT, reverb_preset.iDecayHFLimit);

    alGenAuxiliaryEffectSlots(1, auxiliary_effect_slot);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to generate auxiliary effect slot");

        quit(AUXILIARY_EFFECT_SLOT_GENERATION_FAILURE);
    }

    alAuxiliaryEffectSloti(auxiliary_effect_slot[0], AL_EFFECTSLOT_EFFECT, effect[0]);
}

void load_audio() {
    log_info("Load audio file in data buffer, and copy data buffer to source buffer");

    SF_INFO file_info;

    // the file_info struct must be initialized before using it
    memset (&file_info, 0, sizeof (file_info)) ;

    file = sf_open("Test.wav", SFM_READ, &file_info);

    if (file == NULL) {
        log_error("Failed to open audio file");

        quit(AUDIO_FILE_OPEN_FAILURE);
    }

    size_t samples = file_info.frames * file_info.channels;

    data = (short*) calloc(samples, sizeof(short));

    if (data == NULL) {
        log_error("Failed to allocate memory to data buffer");

        quit(ALLOCATE_MEMORY_TO_DATA_BUFFER_FAILURE);
    }

    log_info(
        "%.2lfMiB allocated to audio data buffer",
        samples * sizeof(short) / (1024.0 * 1024.0)
    );

    sf_readf_short(
        file,
        data,
        file_info.frames);

    // Copy data buffer to source buffer
    alBufferData(
        buffer[0],
        AL_FORMAT_STEREO16,
        data,
        samples * sizeof(short),
        file_info.samplerate
    );

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to copy data buffer to source buffer");

        quit(COPY_DATA_BUFFER_TO_SOURCE_BUFFER_FAILURE);        
    }

    // Bind buffer to source
    alSourcei(source[0], AL_BUFFER, buffer[0]);

    if (alGetError() != AL_NO_ERROR) {
        log_error("Failed to bind buffer to source");

        quit(BIND_BUFFER_TO_SOURCE_FAILURE);
    }

    // Bind effeclt slot to source
    alSource3i(source[0], AL_AUXILIARY_SEND_FILTER, auxiliary_effect_slot[0], 0, AL_FILTER_NULL);
}

void play_audio() {
    log_info("Play audio");

    alSourcePlay(source[0]);

    ALint source_state;

    do {
        alGetSourcei(source[0], AL_SOURCE_STATE, &source_state);
    } while (source_state == AL_PLAYING);
}

void quit(int failure_status) {
    log_info("Quit program gracefully by deallocating resources");

    int exit_status = EXIT_FAILURE;

    /* Depending on the value of `failure_status`, only a subset of functions are called while quitting.
    This eliminates the need of a cumbersome if-else block */
    switch (failure_status) {
        // Quit normally, and execute all functions in order
        case NORMAL:
            exit_status = EXIT_SUCCESS;
        // Quit normally, but exit with EXIT_FAILURE to report error
        default:
        case BIND_BUFFER_TO_SOURCE_FAILURE:
        case COPY_DATA_BUFFER_TO_SOURCE_BUFFER_FAILURE:
            free(data);
        case ALLOCATE_MEMORY_TO_DATA_BUFFER_FAILURE:
            sf_close(file);
            alDeleteAuxiliaryEffectSlots(1, auxiliary_effect_slot);
        case AUDIO_FILE_OPEN_FAILURE:
        case AUXILIARY_EFFECT_SLOT_GENERATION_FAILURE:
            alDeleteEffects(1, effect);
        case AUDIO_EFFECT_GENERATION_FAILURE:
            alDeleteSources(1, source);
        case AUDIO_SOURCE_GENERATION_FAILURE:
            alDeleteBuffers(1, buffer);
        case SOURCE_BUFFER_GENERATION_FAILURE:
            alcMakeContextCurrent(NULL);
        case MAKE_CONTEXT_CURRENT_FAILURE:
            alcDestroyContext(context);
        case CONTEXT_CREATION_FAILURE:
            alcCloseDevice(device);
        case DEFAULT_DEVICE_OPEN_FAILURE:
            exit(exit_status);
    }
}
