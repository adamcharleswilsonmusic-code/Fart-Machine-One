/* SPDX-License-Identifier: GPL-3.0-only */
/* Original Fart Machine One artwork: integer canvas primitives, no bitmap assets.
 * Coordinates are local to the 240 x 120 scene strip. No DSP, delay or IRQ masking. */
#define FMO_PAPER RGB(251,244,222)
#define FMO_INK RGB(37,48,62)
#define FMO_SKIN RGB(247,177,120)
#define FMO_SHIRT RGB(103,183,145)
#define FMO_PUFF RGB(187,207,168)
#define FMO_PUFF_MS 480u
#define FMO_ANIM_MS 40u
#define FMO_PUFFS 4u

/* Filled ellipse, scanline bounds using integers only. Sizes are small and fixed. */
static void cartoon_oval(int cx,int cy,int rx,int ry,uint16_t colour)
{
    int y,x=0;
    for(y=-ry;y<=0;y++) {
        while(x<rx && (x+1)*(x+1)*ry*ry+y*y*rx*rx<=rx*rx*ry*ry) x++;
        cv_rect(cx-x,cy+y,2*x+1,1,colour);
        if(y) cv_rect(cx-x,cy-y,2*x+1,1,colour);
    }
}
static void cartoon_outlined(int x,int y,int rx,int ry,uint16_t fill)
{
    cartoon_oval(x,y,rx,ry,FMO_INK);
    cartoon_oval(x,y,rx-2,ry-2,fill);
}
static void cartoon_puff(uint32_t age,uint32_t variant)
{
    int phase=(int)(age/FMO_ANIM_MS);
    int x=100-phase*4, y=84-phase/2+(int)(variant%2u)*4;
    int r=4+phase;
    uint16_t ink=phase<7?FMO_PUFF:RGB(221,225,194);
    /* The cloud separates into softer wisps as it travels left. */
    if(phase<8) {
        cartoon_oval(x,y,r+5,r/2+3,ink);
        cartoon_oval(x-6,y-6,r/2+3,r/2+3,ink);
        cartoon_oval(x+4,y-7,r/2+2,r/2+2,ink);
        cartoon_oval(x-4,y+3,r/2+3,r/2+2,ink);
        cartoon_oval(x-4,y-5,2,2,FMO_PAPER);
    } else {
        cartoon_oval(x-8,y-8,5,4,ink);
        cartoon_oval(x+7,y-4,4,3,ink);
        cartoon_oval(x,y+6,6,3,ink);
    }
    if(phase<3) {
        cv_line(106,80,111,78,FMO_PUFF);
        cv_line(106,89,112,90,FMO_PUFF);
    }
}
static void cartoon_man(int reacting)
{
    int hop=reacting?-2:0;
    cartoon_oval(139,113,37,4,RGB(225,215,187));
    /* Stubby legs and shoes behind the round body. */
    cartoon_outlined(128,101+hop,7,13,FMO_SKIN);
    cartoon_outlined(152,101+hop,7,13,FMO_SKIN);
    cartoon_oval(130,111+hop,12,5,FMO_INK);
    cartoon_oval(156,111+hop,12,5,FMO_INK);
    /* Head, neck, generously round shirt and shorts. */
    cartoon_outlined(147,47+hop,9,12,FMO_SKIN);
    cartoon_outlined(142,74+hop,31,30,FMO_SHIRT);
    cartoon_oval(141,94+hop,26,10,FMO_INK);
    cv_rect(118,90+hop,47,9,FMO_INK);
    cv_line(142,97+hop,142,103+hop,FMO_PAPER);
    cartoon_oval(146,74+hop,21,17,RGB(123,201,160));
    /* Short sleeve, hanging arm, tiny hand. */
    cartoon_outlined(162,72+hop,8,16,FMO_SKIN);
    cartoon_oval(160,61+hop,10,8,FMO_INK);
    cartoon_oval(160,60+hop,8,6,FMO_SHIRT);
    cartoon_oval(164,84+hop,6,6,FMO_SKIN);
    cv_line(166,84+hop,166,88+hop,RGB(208,129,88));
    /* Hair silhouette, cheek, ear and nose: facing right, puff behind him. */
    cartoon_oval(146,27+hop,21,22,FMO_INK);
    cartoon_outlined(149,32+hop,19,20,FMO_SKIN);
    cartoon_oval(143,15+hop,16,8,FMO_INK);
    cartoon_oval(131,25+hop,5,11,FMO_INK);
    cartoon_outlined(133,35+hop,5,7,FMO_SKIN);
    cartoon_outlined(168,33+hop,7,5,FMO_SKIN);
    cartoon_oval(163,39+hop,5,3,RGB(234,137,105));
    if(reacting) {
        cv_line(153,27+hop,157,29+hop,FMO_INK);
        cv_line(157,29+hop,152,31+hop,FMO_INK);
        cartoon_oval(160,43+hop,3,4,FMO_INK);
    } else {
        cartoon_oval(156,28+hop,2,3,FMO_INK);
        cv_line(153,21+hop,159,22+hop,FMO_INK);
        cv_line(154,42+hop,158,45+hop,FMO_INK);
        cv_line(158,45+hop,164,42+hop,FMO_INK);
    }
    /* A single hair flick makes the silhouette readable at device size. */
    cv_line(139,8+hop,143,4+hop,FMO_INK);
    cv_line(143,4+hop,148,8+hop,FMO_INK);
}
