/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual UI + host platform, with memory/undefined-behavior sanitizers. */
#define main scenario_main
#include "../../host/omni_host.c"
#undef main
#include <assert.h>
static uint32_t pixels(int start,int end) {
    uint32_t h=2166136261u;
    for(int i=start*240;i<end*240;i++)h=(h^fb[i])*16777619u;
    return h;
}
int main(void) {
    boot(); run_ms(50);
    uint32_t idle=pixels(30,150),header=pixels(0,30),controls=pixels(178,240);
    assert(!ui.puff_mask && ui.puff_count==0);
    held_keys=1;run_ms(50);held_keys=0;run_ms(10);
    assert(ui.puff_count==1 && om_triggers==1 && ui.puff_mask);
    assert(pixels(30,150)!=idle);
    assert(pixels(0,30)==header && pixels(178,240)==controls);
    for(int k=0;k<12;k++) {held_keys=1u<<(k%27);run_ms(10);held_keys=0;run_ms(10);}
    assert(ui.puff_count==13 && om_triggers==13 && om_dropped==0);
    assert(__builtin_popcount(ui.puff_mask)==4);
    run_ms(520);assert(!ui.puff_mask && pixels(30,150)==idle);
    assert(pixels(0,30)==header && pixels(178,240)==controls);
    min_q[mi_w++%MQ]=0x09u|0x90u<<8|53u<<16|127u<<24;run_ms(30);
    assert(ui.puff_count==14 && om_triggers==14 && ui.puff_mask);
    held_btn=1u<<B_PLAY;run_ms(30);held_btn=0;run_ms(30); /* starts the empty loop */
    held_btn=1u<<B_PLAY;run_ms(30);held_btn=0;
    assert(!ui.puff_mask && !om_active_keys);
    min_q[mi_w++%MQ]=0x09u|0x90u<<8|53u<<16;run_ms(20);assert(ui.puff_count==14);
    /* Animation lifetime survives uint32 millisecond rollover (no long simulation). */
    now_ms=UINT32_MAX-100u;trigger_key(0,127);ui_frame();assert(ui.puff_mask);
    now_ms=100u;ui_frame();assert(ui.puff_mask);
    now_ms=500u;ui_frame();assert(!ui.puff_mask);
    ui_say("UPDATE MODE IN","3...");ui_frame();assert(ui.message && !strcmp(ui.sub,"3..."));
    puts("UI: rapid/repeated/MIDI puffs, bounded slots, expiry, PLAY clears, stable control strips, timer wrap and update caption PASS");
}
