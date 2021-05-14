#include "../main/main.h"
#include "../audio/audio.h"
#include "../log/log.h"

#include <assert.h>
#include <stdlib.h>

int main()
{
    assert(audioInitSubsystem() == AUDIO_SUCCESS);

    AudioClip clip;

    assert(audioLoadClip(&clip, "Test.wav") == AUDIO_SUCCESS);

    log_info(
        "Audio clip loaded: %ld frames, %dHz, %s audio",
        clip.frames,
        clip.sample_rate,
        (clip.channels == 1) ? "Mono" : "Stereo");

    audioPlayClip(&clip);

    audioUnloadClip(&clip);

    audioQuitSubsystem();

    return EXIT_SUCCESS;
}