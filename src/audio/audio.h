#pragma once

#include <AL/al.h>
#include <sndfile.h>

typedef struct {
    SNDFILE* file;
    ALuint buffer;
    long frames;
    int sample_rate;
    int channels;
    ALenum format;
} AudioClip;

enum audio_failure_statuses {
    AUDIO_SUCCESS,
    AUDIO_FAILURE
};

int audioInitSubsystem();
int audioLoadClip(AudioClip* clip, char const* clip_file_name);
void audioUnloadClip(AudioClip* clip);
void audioPlayClip(AudioClip* clip);
void audioQuitSubsystem();