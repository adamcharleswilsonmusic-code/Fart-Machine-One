/* SPDX-License-Identifier: GPL-3.0-only */
/* Fart Machine One: one-shot samples + the retained FoMni plate + simple echo.
 * Single producer (UI), single consumer (audio IRQ). All DSP state belongs to render. */
#include "omni.h"
#include "fastmath.h"
#ifndef FMO_SAMPLE_BANK_HEADER
#define FMO_SAMPLE_BANK_HEADER "sample_bank.h"
#endif
#include FMO_SAMPLE_BANK_HEADER
#ifdef OM_HOST
#define OM_POOL
#else
#define OM_POOL __attribute__((section(".pool")))
#endif
#include "plate.h"
static const om_param_t params[P_NPARAMS] = {
    {"REVERB",0,100,15}, {"DELAY",0,100,0}, {"TIME ms",80,500,240}, {"REPEATS",0,70,25}
};
const om_param_t *om_param_info(int p) { return p >= 0 && p < P_NPARAMS ? &params[p] : 0; }
const char *om_sample_name(int k) { return k >= 0 && k < FMO_KEYS ? fmo_samples[fmo_map[k]].name : ""; }
int om_placeholder(void) { return FMO_PLACEHOLDER; }
volatile uint32_t om_active_keys, om_triggers, om_dropped;
#define QN 128u
#define BARRIER() __asm__ volatile("" ::: "memory")
typedef struct { int type, a, b; } command_t;
static command_t queue[QN];
static volatile uint32_t qw, qr, panic_pending;
static int par[P_NPARAMS];
static float rev_mix, echo_mix, feedback, delay_frames;
typedef struct { uint32_t frame, half, age; int key, active; float gain; } voice_t;
static voice_t voices[FMO_VOICES];
static uint32_t age;
#define ECHO_N 22052u
static float echo[ECHO_N] OM_POOL;
static uint32_t echo_pos;
/* Fractional sixteenth clock: increment bpm*4 per output sample, wrap at 60*44100.
 * Keeping the fractional remainder avoids drift at tempos not dividing the rate. */
#define SEQ_PHASE 2646000u
volatile uint8_t om_seq_playing, om_seq_recording, om_seq_step;
volatile uint16_t om_seq_bpm;
volatile uint32_t om_seq_mask[FMO_STEPS], om_seq_ticks, om_seq_hit_dropped;
static uint32_t seq_phase, seq_skip[FMO_STEPS];
static volatile uint32_t seq_epoch;
static uint8_t seq_velocity[FMO_STEPS][FMO_KEYS];
#define HIT_Q 64u
static struct { int key; uint32_t epoch; } seq_hits[HIT_Q];
static volatile uint32_t hit_w, hit_r;
int om_take_seq_hit(int *key) {
    while(hit_r != hit_w) {
        uint32_t r=hit_r,epoch; int k;
        BARRIER(); k=seq_hits[r%HIT_Q].key; epoch=seq_hits[r%HIT_Q].epoch;
        BARRIER(); hit_r=r+1u;
        if(epoch==seq_epoch) { *key=k; return 1; }
    }
    return 0;
}
static void seq_report(int key) {
    uint32_t w=hit_w;
    if(w-hit_r>=HIT_Q) {om_seq_hit_dropped++;return;}
    seq_hits[w%HIT_Q].key=key; seq_hits[w%HIT_Q].epoch=seq_epoch;
    BARRIER();hit_w=w+1u;
}
static void seq_stop(void) {
    int i;om_seq_playing=om_seq_recording=0;om_seq_step=0;seq_phase=0;seq_epoch++;
    for(i=0;i<FMO_STEPS;i++)seq_skip[i]=0;
}
static void seq_erase(void) {
    int i,j;
    for(i=0;i<FMO_STEPS;i++) {om_seq_mask[i]=seq_skip[i]=0;for(j=0;j<FMO_KEYS;j++)seq_velocity[i][j]=0;}
}
static void start_voice(int key,int velocity) {
    uint32_t j;int s=0;
    for(j=0;j<FMO_VOICES;j++) {if(!voices[j].active){s=(int)j;break;}
        if(voices[j].age<voices[s].age)s=(int)j;}
    voices[s]=(voice_t){0,0,++age,key,1,(float)velocity*(1.0f/127.0f)};om_triggers++;
}
static void seq_fire(void) {
    int k;uint32_t mask=om_seq_mask[om_seq_step]&~seq_skip[om_seq_step];
    seq_skip[om_seq_step]=0;om_seq_ticks++;
    for(k=0;k<FMO_KEYS;k++)if(mask&(1u<<k)) {start_voice(k,seq_velocity[om_seq_step][k]);seq_report(k);}
}
static void seq_start(void) {
    om_seq_step=0;seq_phase=0;om_seq_playing=1;seq_fire();
}
static void seq_record_hit(int key,int velocity) {
    uint32_t step=om_seq_step;int canonical=0;
    /* Repeated keyboard copies are the same sound, so store one voice per sound. */
    while(fmo_map[canonical]!=fmo_map[key])canonical++;
    if(seq_phase>=SEQ_PHASE/2u)step=(step+1u)%FMO_STEPS;
    om_seq_mask[step]|=1u<<canonical;seq_velocity[step][canonical]=(uint8_t)velocity;
    /* Live audio already sounded: do not echo a newly quantized-ahead note again
     * at the imminent boundary. It joins playback on the next circuit. */
    if(step!=om_seq_step)seq_skip[step]|=1u<<canonical;
}
static void post(int type, int a, int b) {
    uint32_t w = qw;
    if (w - qr >= QN) { om_dropped++; return; }
    queue[w % QN] = (command_t){type,a,b};
    BARRIER(); qw = w + 1u;
}
void om_set(int p, int v) { if (p >= 0 && p < P_NPARAMS) post(1,p,v); }
void om_trigger(int k, int v) { if (k >= 0 && k < FMO_KEYS && v > 0) post(0,k,v > 127 ? 127 : v); }
void om_seq_toggle_play(void) {post(2,0,0);}
void om_seq_toggle_record(void) {post(3,0,0);}
void om_seq_clear(void) {post(4,0,0);}
void om_seq_set_tempo(int bpm) {post(5,bpm,0);}
void om_panic(void) { panic_pending = 1; }
static void clear_sound(void) {
    uint32_t i;
    for (i=0;i<FMO_VOICES;i++) voices[i].active=0;
    for (i=0;i<ECHO_N;i++) echo[i]=0.0f;
    echo_pos=0; plate_init(); om_active_keys=0;
}
void om_init(void) {
    int i; qw=qr=panic_pending=0; age=om_triggers=om_dropped=0;
    for(i=0;i<P_NPARAMS;i++) par[i]=params[i].def;
    rev_mix=(float)par[P_REVERB]*0.01f; echo_mix=(float)par[P_DELAY]*0.01f;
    feedback=(float)par[P_FEEDBACK]*0.01f; delay_frames=(float)par[P_TIME]*44.1f;
    seq_phase=seq_epoch=hit_w=hit_r=om_seq_ticks=om_seq_hit_dropped=0;
    om_seq_playing=om_seq_recording=om_seq_step=0;om_seq_bpm=120;seq_erase();
    clear_sound();
}
static float limit(float x) { return x > 0.98f ? 0.98f : x < -0.98f ? -0.98f : x; }
void om_render(int32_t *out, uint32_t n, uint32_t gain_q12) {
    uint32_t i,j,keys=0; float master=(float)(gain_q12 > 4096 ? 4096 : gain_q12)*(1.0f/4096.0f);
    if(panic_pending) { qr=qw; panic_pending=0; seq_stop(); clear_sound(); }
    while(qr != qw) {
        command_t c; BARRIER(); c=queue[qr % QN]; BARRIER(); qr++;
        if(c.type==1) par[c.a]=c.b<params[c.a].lo?params[c.a].lo:c.b>params[c.a].hi?params[c.a].hi:c.b;
        else if(c.type==0) {
            start_voice(c.a,c.b);
            if(om_seq_recording)seq_record_hit(c.a,c.b);
        } else if(c.type==2) {
            if(om_seq_playing) {seq_stop();clear_sound();} else seq_start();
        } else if(c.type==3) {
            om_seq_recording=(uint8_t)!om_seq_recording;
            if(om_seq_recording && !om_seq_playing)seq_start();
        } else if(c.type==4) {
            seq_stop();seq_erase();clear_sound();
        } else if(c.type==5) om_seq_bpm=(uint16_t)(c.a<40?40:c.a>240?240:c.a);
    }
    for(i=0;i<n;i++) {
        float dry=0.0f,wl,wr,delayed,in,l,r; uint32_t off,at,next;
        if(om_seq_playing && seq_phase>=SEQ_PHASE) {
            seq_phase-=SEQ_PHASE;om_seq_step=(uint8_t)((om_seq_step+1u)%FMO_STEPS);seq_fire();
        }
        rev_mix+=0.002f*((float)par[P_REVERB]*0.01f-rev_mix);
        echo_mix+=0.002f*((float)par[P_DELAY]*0.01f-echo_mix);
        feedback+=0.002f*((float)par[P_FEEDBACK]*0.01f-feedback);
        delay_frames+=0.0002f*((float)par[P_TIME]*44.1f-delay_frames);
        for(j=0;j<FMO_VOICES;j++) if(voices[j].active) {
            voice_t *v=&voices[j]; const fmo_sample_t *s=&fmo_samples[fmo_map[v->key]];
            uint32_t a=v->frame, b=a+1u<s->length?a+1u:a;
            float x=(float)s->data[a];
            if(v->half) x=0.5f*(x+(float)s->data[b]);
            dry+=x*(1.0f/32768.0f)*v->gain*0.32f;
            v->half^=1u; if(!v->half && ++v->frame>=s->length) v->active=0;
        }
        off=(uint32_t)delay_frames; at=(echo_pos+ECHO_N-off)%ECHO_N; next=(at+ECHO_N-1u)%ECHO_N;
        delayed=echo[at]+(echo[next]-echo[at])*(delay_frames-(float)off);
        in=limit(dry+delayed*feedback); echo[echo_pos]=fm_flush(in);
        if(++echo_pos==ECHO_N) echo_pos=0;
        reverb(dry*0.5f,&wl,&wr);
        l=limit(dry+delayed*echo_mix+wl*rev_mix)*master;
        r=limit(dry+delayed*echo_mix+wr*rev_mix)*master;
        out[2*i]=(int32_t)(l*8388607.0f); out[2*i+1]=(int32_t)(r*8388607.0f);
        if(om_seq_playing)seq_phase+=(uint32_t)om_seq_bpm*4u;
    }
    for(j=0;j<FMO_VOICES;j++) if(voices[j].active) keys|=1u<<voices[j].key;
    om_active_keys=keys;
}
