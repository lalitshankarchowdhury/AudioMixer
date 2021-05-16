#include "../main/main.h"
#include "../audio/audio.h"
#include "../log/log.h"

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    assert(audioInitSubsystem() == AUDIO_SUCCESS);

    Clip clips[2];

    assert(audioLoadClip(&clips[0], "Test1.wav") == AUDIO_SUCCESS);
    assert(audioLoadClip(&clips[1], "Test2.wav") == AUDIO_SUCCESS);

    Track track;

    memset(&track, 0, sizeof(Track));

    time_t clip_position1, clip_position2;

    time(&clip_position1);
    time(&clip_position2);

    audioAddClipToTrack(&track, &clips[0], clip_position1);
    audioAddClipToTrack(&track, &clips[1], clip_position2 + 10);

    audioPlayTrack(&track);

    audioQuitSubsystem();

    return EXIT_SUCCESS;
}
