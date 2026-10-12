/* SPDX-License-Identifier: GPL-3.0-only */
#define main scenario_main
#include "../../host/omni_host.c"
#undef main
#include <assert.h>
static void button(int b) {held_btn=1u<<b;run_ms(20);held_btn=0;run_ms(20);}
int main(void) {
    boot();run_ms(50);
    button(B_REC);assert(om_seq_recording && om_seq_playing);
    held_keys=3;run_ms(10);held_keys=0;run_ms(10);
    assert(om_seq_mask[0]==3 && om_triggers==2 && ui.puff_count==2);
    button(B_REC);assert(om_seq_playing && !om_seq_recording);
    run_ms(2100);assert(om_triggers>=4 && ui.puff_count>=4); /* sequenced notes use the same puff */
    assert(om_seq_hit_dropped==0 && om_dropped==0);
    button(B_PLAY);assert(!om_seq_playing && !om_seq_recording && !om_active_keys && !ui.puff_mask);
    button(B_PLAY);assert(om_seq_playing && om_seq_step==0 && om_seq_mask[0]==3);
    held_btn=1u<<B_REC;run_ms(1050);assert(!om_seq_playing && !om_seq_recording && !ui.puff_mask);
    for(int i=0;i<16;i++)assert(om_seq_mask[i]==0);
    held_btn=0;run_ms(50);assert(!om_seq_recording && !om_seq_playing); /* release after hold is NOT a tap */
    enc_acc[EN_SELECT]=1000;run_ms(20);assert(om_seq_bpm==240);
    enc_acc[EN_SELECT]=-1000;run_ms(20);assert(om_seq_bpm==40);
    enc_acc[EN_SELECT]=80;run_ms(20);assert(om_seq_bpm==120);
    held_keys=1u<<4;run_ms(20);held_keys=0;run_ms(20);assert(ui.last==4 && !om_seq_playing);
    button(B_REC);
    min_q[mi_w++%MQ]=0x09u|0x90u<<8|53u<<16|100u<<24;run_ms(20);
    assert(om_seq_mask[0]&1u); /* MIDI notes record as well as local keys */
    min_q[mi_w++%MQ]=0x0bu|0xb0u<<8|123u<<16;run_ms(20);
    assert(!om_seq_playing && !om_seq_recording && !om_active_keys);
    puts("SEQ UI PASS: REC start/overdub/off, playback puffs, PLAY stop/restart, hold-REC clear without release toggle, SELECT tempo, live playing, MIDI record/panic");
}
