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

    AudioClip clips[2];

    assert(audioLoadClip(&clips[0], "Test1.wav") == AUDIO_SUCCESS);
    assert(audioLoadClip(&clips[1], "Test2.wav") == AUDIO_SUCCESS);

    audioPlayClips(clips, 2);

    audioUnloadClip(&clips[0]);
    audioUnloadClip(&clips[1]);

    audioQuitSubsystem();

    return EXIT_SUCCESS;
}
