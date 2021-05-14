#include "../main/main.h"
#include "../audio/audio.h"
#include "../log/log.h"

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    assert(audioInitSubsystem() == AUDIO_SUCCESS);

    AudioClip clip[2];

    assert(audioLoadClip(&clip[0], "Test1.wav") == AUDIO_SUCCESS);
    assert(audioLoadClip(&clip[1], "Test2.wav") == AUDIO_SUCCESS);

    int total_clips = 2, clips_left = 2;

    while (true) {
        sleep(1);

        for (int i = total_clips - clips_left; i < total_clips; ++i) {
            if (audioPlayClip(&clip[i]) == AUDIO_FAILURE) {
                audioUnloadClip(&clip[i]);

                --clips_left;
            }
        }

        if (clips_left == 0) {
            break;
        }
    }

    audioQuitSubsystem();

    return EXIT_SUCCESS;
}
