
#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

void audio_init(void);
void audio_play_tone(uint32_t frequency, uint32_t duration_ms);
void audio_stop(void);
void audio_update(void);

#endif
