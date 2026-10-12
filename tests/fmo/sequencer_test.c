/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "omni.h"
static int32_t out[512];
static void run(unsigned frames) {
    while(frames) {unsigned n=frames>256?256:frames;om_render(out,n,4096);frames-=n;
        for(unsigned i=0;i<2*n;i++)assert(out[i]>=-8220836 && out[i]<=8220836);}
}
static void until_step(int step) {for(unsigned n=0;n<300000 && om_seq_step!=step;n++)run(1);assert(om_seq_step==step);}
int main(void) {
    int key,count;
    om_init();assert(!om_seq_playing && !om_seq_recording && om_seq_bpm==120);
    om_seq_toggle_play();run(1);assert(om_seq_ticks==1 && om_seq_step==0);
    run(5512);assert(om_seq_step==0);run(1);assert(om_seq_step==1); /* first boundary frame 5513 */
    run(88201-5514);assert(om_seq_ticks==17 && om_seq_step==0); /* exact 2-second bar + first sample */
    om_init();om_seq_set_tempo(137);om_seq_toggle_play();run(2646001);
    assert(om_seq_ticks==549 && om_seq_step==4); /* no fractional drift after one minute */
    om_init();om_seq_toggle_play();run(2756);om_seq_set_tempo(240);run(1379);
    assert(om_seq_step==0);run(1);assert(om_seq_step==1); /* tempo changes preserve fractional position */
    om_init();om_seq_toggle_record();run(1);
    until_step(15);run(3307);om_trigger(3,127);run(1);assert(om_seq_mask[0]==8);
    until_step(0);assert(om_triggers==1); /* end-of-bar quantization wraps and skips immediate echo */
    om_seq_toggle_record();run(1);run(88200);assert(om_triggers==2);
    om_init();om_seq_toggle_record();run(1);
    assert(om_seq_playing && om_seq_recording && om_seq_step==0);
    om_trigger(0,127);om_trigger(1,100);om_trigger(11,32);run(1);
    assert(om_seq_mask[0]==3 && om_triggers==3); /* key 11 repeats sample 0: one stored sound */
    run(3306);om_trigger(2,127);run(1);assert(om_seq_mask[1]==4);
    until_step(1);assert(om_triggers==4); /* quantized-ahead live hit is not doubled */
    om_seq_toggle_record();run(1);assert(!om_seq_recording && om_seq_playing);
    unsigned before=om_triggers;run(88200);assert(om_triggers==before+3);
    count=0;while(om_take_seq_hit(&key)){assert(key>=0 && key<=2);count++;}assert(count==3);
    om_seq_toggle_play();run(1);assert(!om_seq_playing && !om_seq_recording && !om_active_keys);
    assert(om_seq_mask[0]==3 && om_seq_mask[1]==4);
    before=om_triggers;run(100000);assert(om_triggers==before); /* stopped means no sequenced audio */
    om_seq_toggle_play();run(1);assert(om_seq_step==0 && om_triggers==before+2);
    om_seq_set_tempo(500);run(1);assert(om_seq_bpm==240);
    om_seq_set_tempo(-5);run(1);assert(om_seq_bpm==40);
    om_seq_clear();run(1);assert(!om_seq_playing && !om_seq_recording && om_seq_bpm==40);
    for(int i=0;i<16;i++)assert(om_seq_mask[i]==0);
    assert(!om_take_seq_hit(&key)); /* old-transport animation events are discarded */
    om_trigger(7,127);run(1);assert(om_active_keys); /* live keys still work while stopped */
    om_panic();run(1);assert(!om_active_keys && !om_seq_playing);
    om_init();assert(om_seq_bpm==120);for(int i=0;i<16;i++)assert(om_seq_mask[i]==0);
    puts("SEQ DSP PASS: exact sample-clock timing, 16-step wrap, 137 BPM minute, quantization, overdub/dedup, no doubled live hit, stop/restart, clear, tempo limits, live keys, stale puff cancellation, reboot defaults");
}
