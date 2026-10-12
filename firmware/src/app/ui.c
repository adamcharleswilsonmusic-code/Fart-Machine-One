/* SPDX-License-Identifier: GPL-3.0-only */
/* One screen. Animation runs in the main-loop UI, never in the audio interrupt. */
#include "cartoon.h"
static const uint8_t WHITE_K[16]={0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26};
static const uint8_t BLACK_K[11]={1,3,5,8,10,13,15,17,20,22,25};
static struct {
    uint32_t keys,buttons,deadline,puff_at[FMO_PUFFS],puff_count,last_scene;
    uint8_t puff_mask,next_puff,rec_held,rec_cleared;
    uint32_t rec_down_ms,seq_signature;
    int tempo,status_dirty;
    int last,redraw,scene_dirty,caption_dirty,controls_dirty;
    const char *message; char sub[24];
} ui;
static void decimal(int v,char *b) { char t[12]; int n=0; do { t[n++]=(char)('0'+v%10); v/=10; }while(v); while(n) *b++=t[--n]; *b=0; }
int ui_dirty(void) { return 0; }
void ui_say(const char *a,const char *b) { int i; ui.message=a; for(i=0;i<23 && b[i];i++) ui.sub[i]=b[i]; ui.sub[i]=0; ui.deadline=plat_ms()+1400u; ui.caption_dirty=1; }
void ui_init(void) {
    uint32_t i;
    ui.keys=ui.buttons=0; ui.last=-1; ui.message=0; ui.redraw=1;
    ui.puff_mask=ui.next_puff=0; ui.puff_count=0; ui.last_scene=plat_ms();
    for(i=0;i<FMO_PUFFS;i++) ui.puff_at[i]=0;
    ui.rec_held=ui.rec_cleared=0;ui.tempo=om_seq_bpm;ui.seq_signature=0xffffffffu;ui.status_dirty=1;
    plat_glow(0); plat_play_red(0);
}
static void show_hit(int k) {
    uint32_t slot=ui.next_puff;
    ui.last=k; ui.caption_dirty=1;
    /* A bounded four-puff trail: every trigger starts a fresh puff, even if all
     * slots are busy. Never wait for the previous animation to finish. */
    ui.puff_at[slot]=plat_ms(); ui.puff_mask|=(uint8_t)(1u<<slot);
    ui.next_puff=(uint8_t)((slot+1u)%FMO_PUFFS); ui.puff_count++;
    ui.scene_dirty=1;
}
static void trigger_key(int k,int velocity) {om_trigger(k,velocity);show_hit(k);}
static void quiet(void) { om_panic(); ui.puff_mask=0; ui.scene_dirty=1; }
void ui_input_only(void) {
    uint32_t keys=plat_keys(),down=keys & ~ui.keys,buttons=plat_buttons(),pkt; int k;
    uint32_t pressed=buttons & ~ui.buttons, released=ui.buttons & ~buttons;
    while(om_take_seq_hit(&k))show_hit(k);
    if(pressed & (1u<<B_PLAY)) {
        om_seq_toggle_play();
        if(om_seq_playing) {ui.puff_mask=0;ui.scene_dirty=1;}
    }
    if(pressed & (1u<<B_REC)) {ui.rec_held=1;ui.rec_cleared=0;ui.rec_down_ms=plat_ms();}
    if(ui.rec_held && (buttons & (1u<<B_REC)) && !ui.rec_cleared && plat_ms()-ui.rec_down_ms>=1000u) {
        om_seq_clear();ui.rec_cleared=1;ui.puff_mask=0;ui.scene_dirty=1;
        ui_say("LOOP CLEARED","READY TO RECORD");
    }
    if(released & (1u<<B_REC)) {
        if(ui.rec_held && !ui.rec_cleared)om_seq_toggle_record();
        ui.rec_held=0;
    }
    for(k=0;k<NKEYS;k++) if(down & (1u<<k)) trigger_key(k,127);
    ui.keys=keys; ui.buttons=buttons;
    for(k=0;k<NE;k++) { int d=plat_enc(k);
        if(d && k==EN_SELECT) {
            ui.tempo+=d;if(ui.tempo<40)ui.tempo=40;if(ui.tempo>240)ui.tempo=240;
            om_seq_set_tempo(ui.tempo);
        } else if(d && k>=EN_K1) {
            int p=k-EN_K1; const om_param_t *info=om_param_info(p);
            int v=proj.par[p]+d*(p==P_TIME?10:1);
            if(v<info->lo)v=info->lo; if(v>info->hi)v=info->hi;
            proj.par[p]=(int16_t)v; om_set(p,v); ui.controls_dirty=1;
        }
    }
    /* Channel 1, MIDI F3..G5 (53..79). Note-off does not truncate one-shots. */
    for(k=0;k<64 && plat_midi_in(&pkt);k++) {
        uint32_t st=(pkt>>8)&255u,note=(pkt>>16)&127u,vel=(pkt>>24)&127u;
        if(st==0x90 && vel && note>=53 && note<=79) trigger_key((int)note-53,(int)vel);
        if(st==0xb0 && (note==120 || note==123)) quiet();
    }
    plat_play_red(om_seq_recording);
    plat_leds((om_seq_playing?(1u<<B_PLAY):0u)|(om_seq_recording?(1u<<B_REC):0u),keys | om_active_keys);
}
static void draw_scene(uint32_t now) {
    uint32_t i; int reacting=0;
    cv_begin(240,120,FMO_PAPER);
    for(i=0;i<FMO_PUFFS;i++) if(ui.puff_mask & (1u<<i)) {
        uint32_t age=now-ui.puff_at[i];
        cartoon_puff(age,i); if(age<160u) reacting=1;
    }
    cartoon_man(reacting);
    cv_blit(0,30);
    ui.last_scene=now; ui.scene_dirty=0;
}
static void draw_caption(void) {
    const char *label=ui.message?ui.message:ui.last>=0?om_sample_name(ui.last):"PRESS ANY KEY";
    cv_begin(240,28,FMO_PAPER);
    if(ui.message) {
        cv_text((240-text_w(&FONT_XS,label))/2,1,&FONT_XS,label,FMO_INK);
        cv_text((240-text_w(&FONT_XS,ui.sub))/2,14,&FONT_XS,ui.sub,FMO_INK);
    } else cv_text((240-text_w(&FONT_S,label))/2,4,&FONT_S,label,FMO_INK);
    cv_blit(0,150); ui.caption_dirty=0;
}
static void draw_controls(void) {
    static const char *const names[4]={"1 REV","2 DELAY","3 TIME","4 RPTS"};
    int k; char value[12];
    cv_begin(240,46,FMO_INK);
    for(k=0;k<4;k++) {
        int x=k*60,n; decimal(proj.par[k],value);
        for(n=0;value[n];n++);
        if(k==P_TIME) {value[n++]='m';value[n++]='s';} else value[n++]='%';
        value[n]=0;
        if(k)cv_rect(x,8,1,31,RGB(76,91,100));
        cv_text(x+(60-text_w(&FONT_XS,names[k]))/2,7,&FONT_XS,names[k],RGB(179,211,192));
        cv_text(x+(60-text_w(&FONT_XS,value))/2,24,&FONT_XS,value,C_WHITE);
    }
    cv_blit(0,178); ui.controls_dirty=0;
}
static void draw_status(void) {
    char text[40],number[12];int n=0,i;
    const char *mode=om_seq_recording?"REC ":om_seq_playing?"PLAY ":"STOP ";
    if(om_placeholder()){text[n++]='T';text[n++]=' ';}
    while(*mode)text[n++]=*mode++;
    decimal(om_seq_bpm,number);for(i=0;number[i];i++)text[n++]=number[i];
    text[n++]=' ';text[n++]='B';text[n++]='P';text[n++]='M';
    if(om_seq_playing) {
        int step=om_seq_step+1;
        text[n++]=' ';text[n++]=(char)('0'+step/10);text[n++]=(char)('0'+step%10);
        text[n++]='/';text[n++]='1';text[n++]='6';
    }
    text[n]=0;
    cv_begin(240,16,FMO_INK);
    cv_text((240-text_w(&FONT_XS,text))/2,2,&FONT_XS,text,
            om_seq_recording?RGB(255,162,130):RGB(179,211,192));
    cv_blit(0,224);ui.status_dirty=0;
}

void ui_frame(void) {
    uint32_t i,now; ui_input_only(); now=plat_ms();
    if(ui.message && (int32_t)(now-ui.deadline)>=0) {ui.message=0;ui.caption_dirty=1;}
    for(i=0;i<FMO_PUFFS;i++) if((ui.puff_mask & (1u<<i)) && now-ui.puff_at[i]>=FMO_PUFF_MS) {
        ui.puff_mask&=(uint8_t)~(1u<<i); ui.scene_dirty=1;
    }
    {
        uint32_t sig=((uint32_t)om_seq_bpm<<16)|((uint32_t)om_seq_step<<2)|((uint32_t)om_seq_playing<<1)|om_seq_recording;
        if(sig!=ui.seq_signature){ui.seq_signature=sig;ui.status_dirty=1;}
    }
    if(ui.redraw) {
        cv_begin(240,30,FMO_INK);
        cv_text((240-text_w(&FONT_B,"FART MACHINE ONE"))/2,7,&FONT_B,"FART MACHINE ONE",FMO_PAPER);
        cv_blit(0,0);
        ui.scene_dirty=ui.caption_dirty=ui.controls_dirty=ui.status_dirty=1; ui.redraw=0;
    }
    /* Timed animation updates at 25 fps, plus immediate trigger/expiry redraws;
     * only the scene strip moves.
     * Unsigned elapsed time makes expiry/retrigger safe across the millisecond wrap. */
    if(ui.scene_dirty || (ui.puff_mask && now-ui.last_scene>=FMO_ANIM_MS)) draw_scene(now);
    if(ui.caption_dirty) draw_caption();
    if(ui.controls_dirty) draw_controls();
    if(ui.status_dirty) draw_status();
}
