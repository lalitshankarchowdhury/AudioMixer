#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "include/log.h"
#include "include/AL/al.h"
#include "include/AL/alc.h"
#include "include/sndfile.h"

void init();
void load_audio();
void play_audio ();
void quit(int failure_status);

ALCdevice* device;
ALCcontext* context;
ALuint buffer[1];
ALuint source[1];
enum failure_statuses {
    NORMAL,
    DEFAULT_DEVICE_OPEN_FAILURE,
    CONTEXT_CREATION_FAILURE,
    MAKE_CONTEXT_CURRENT_FAILURE,
    SOURCE_BUFFER_GENERATION_FAILURE,
    AUDIO_SOURCE_GENERATION_FAILURE,
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

    context = alcCreateContext(device, NULL);

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
}

void load_audio() {
    log_info("Load audio file in data buffer, and copy data buffer to source buffer");

    SF_INFO file_info;

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
        case AUDIO_FILE_OPEN_FAILURE:
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
