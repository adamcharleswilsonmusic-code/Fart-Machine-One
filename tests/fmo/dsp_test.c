/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "omni.h"
static int32_t b[512];
static double energy(int frames) { double e=0; while(frames) {int n=frames>256?256:frames;om_render(b,n,4096);for(int i=0;i<2*n;i++){ assert(b[i]>=-8220836 && b[i]<=8220836);double x=b[i];e+=x*x;} frames-=n;}return e; }
static void dry(void){om_init();om_set(P_REVERB,0);om_set(P_DELAY,0);energy(44100);}
int main(void) {
    dry(); assert(energy(44100)==0); /* silent boot */
    for(int k=0;k<27;k++){dry();om_trigger(k,127);assert(energy(2048)>1e6);assert(om_triggers==1);energy(44100);assert(om_active_keys==0);assert(energy(44100)==0);}
    dry();om_trigger(-1,127);om_trigger(27,127);om_trigger(0,0);assert(energy(1024)==0 && om_triggers==0);
    dry();om_trigger(0,127);double loud=energy(4096);dry();om_trigger(0,63);double quiet=energy(4096);assert(quiet>loud*.23 && quiet<loud*.26);
    dry();om_set(P_DELAY,100);om_set(P_TIME,500);om_set(P_FEEDBACK,0);energy(44100);om_trigger(0,127);energy(17640);double echo=energy(22050);assert(echo>1e6);energy(44100);assert(energy(44100)==0);
    dry();om_set(P_REVERB,100);energy(44100);om_trigger(0,127);energy(44100);double tail=energy(44100);assert(tail>1e4);energy(44100*10);assert(energy(44100)<tail*.01);
    dry();for(int k=0;k<27;k++)om_trigger(k,127);assert(energy(256)>1e6);assert(om_triggers==27);assert(__builtin_popcount(om_active_keys)==8);
    om_set(P_REVERB,100);om_set(P_DELAY,100);om_set(P_FEEDBACK,70);energy(44100);om_panic();assert(energy(44100)==0);
    dry();for(int i=0;i<200;i++)om_trigger(i%27,127);assert(om_dropped==72);om_panic();assert(energy(1024)==0);
    dry();om_trigger(0,127);om_render(b,256,0);for(int i=0;i<512;i++)assert(b[i]==0);
    puts("DSP: 27 keys, one-shots, velocity, echo, reverb decay, 8 voices, limits, panic, queue overflow, mute PASS");
}
