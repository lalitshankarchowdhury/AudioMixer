#pragma once

#include <AL/al.h>
#include <sndfile.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    SNDFILE* file;
    ALuint buffer;
    ALuint source;
    long frames;
    int sample_rate;
    int channels;
    ALenum format;
    bool playing;
} Clip;

typedef struct
{
    // A pointer to a dynamic array of pointers to Clip
    Clip** clips;

    int num_clips;

    // A a pointer to dynamic array of clip positions in track
    time_t* clip_positions;
} Track;

enum audio_failure_statuses {
    AUDIO_FAILURE,
    AUDIO_SUCCESS
};

int audioInitSubsystem();
int audioLoadClip(Clip* clip, char const* clip_file_name);
int audioPlayClips(Clip clips[], int num_clips);
bool audioIsClipPlaying(Clip* clip);
void audioUnloadClip(Clip* clip);
void audioAddClipToTrack(Track* track, Clip* clip, int clip_position);
void audioPlayTrack(Track* track);
void audioQuitSubsystem();
