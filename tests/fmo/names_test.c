/* SPDX-License-Identifier: GPL-3.0-only */
#define main scenario_main
#include "../../host/omni_host.c"
#undef main
#include <assert.h>
int main(void) {
    static const char *const names[]={"TOOT","TRUMP","PFFT","RASPBERRY","WIND","FLATULENCE","BOTTOM BURP","AIR BISCUIT","RIPPER","SBD","LET RIP"};
    boot(); run_ms(50);
    assert(text_w(&FONT_XS,"REC 240 BPM 16/16")<=224); /* compact loop status fits */
    assert(FONT_S.h<=28);
    for(int k=0;k<27;k++) {
        const char *name=om_sample_name(k);
        assert(!strcmp(name,names[k%11]));
        assert(text_w(&FONT_S,name)<=224);
        for(const char *p=name;*p;p++)assert(*p>=' ' && *p<='~' && !(*p>='0' && *p<='9'));
        if(k<11)printf("key %d: %s, width %d / 224 pixels\n",k,name,text_w(&FONT_S,name));
    }
    puts("PASS: all 11 labels fit, remain unique, repeat without suffixes over 27 keys; loop-status text fits.");
}
