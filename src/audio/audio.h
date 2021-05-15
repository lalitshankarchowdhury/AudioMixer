#pragma once

#include <AL/al.h>
#include <sndfile.h>
#include <stdbool.h>

typedef struct {
    SNDFILE* file;
    ALuint buffer;
    ALuint source;
    long frames;
    int sample_rate;
    int channels;
    ALenum format;
} AudioClip;

enum audio_failure_statuses {
    AUDIO_FAILURE,
    AUDIO_SUCCESS
};

int audioInitSubsystem();
int audioLoadClip(AudioClip* clip, char const* clip_file_name);
void audioUnloadClip(AudioClip* clip);
int audioPlayClips(AudioClip clips[], int num_clips);
bool audioIsClipPlaying(AudioClip* clip);
void audioQuitSubsystem();
