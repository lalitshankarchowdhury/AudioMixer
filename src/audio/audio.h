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
} Clip;

enum audio_failure_statuses {
    AUDIO_FAILURE,
    AUDIO_SUCCESS
};

int audioInitSubsystem();
int audioLoadClip(Clip* clip, char const* clip_file_name);
void audioUnloadClip(Clip* clip);
int audioPlayClips(Clip clips[], int num_clips);
bool audioIsClipPlaying(Clip* clip);
void audioQuitSubsystem();
