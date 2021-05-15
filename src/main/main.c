#include "../main/main.h"
#include "../audio/audio.h"
#include "../log/log.h"

#include <assert.h>
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

    assert(audioPlayClips(clips, 2) == AUDIO_SUCCESS);

    int clips_left_to_play = 2;

    while (clips_left_to_play > 0) {
        sleep(1);

        for (int i = 0; i < 2; ++i) {
            if (!audioIsClipPlaying(&clips[i])) {
                audioUnloadClip(&clips[i]);

                --clips_left_to_play;
            }
        }
    }

    audioQuitSubsystem();

    return EXIT_SUCCESS;
}
