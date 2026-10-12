/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#define FMO_KEYS 27
#define FMO_VOICES 8
enum { P_REVERB, P_DELAY, P_TIME, P_FEEDBACK, P_NPARAMS };
typedef struct { const char *name; int16_t lo, hi, def; } om_param_t;
const om_param_t *om_param_info(int p);
void om_init(void);
void om_set(int p, int value);
void om_trigger(int key, int velocity);
void om_panic(void);
void om_render(int32_t *out, uint32_t n, uint32_t gain_q12);
const char *om_sample_name(int key);
int om_placeholder(void);
extern volatile uint32_t om_active_keys, om_triggers, om_dropped;

/* One volatile 16-sixteenth-note pattern; commands are consumed by the audio ISR. */
#define FMO_STEPS 16
void om_seq_toggle_play(void);
void om_seq_toggle_record(void);
void om_seq_clear(void);
void om_seq_set_tempo(int bpm);
int om_take_seq_hit(int *key); /* UI consumes playback hits; never feeds them back into audio */
extern volatile uint8_t om_seq_playing, om_seq_recording, om_seq_step;
extern volatile uint16_t om_seq_bpm;
extern volatile uint32_t om_seq_mask[FMO_STEPS], om_seq_ticks, om_seq_hit_dropped;
