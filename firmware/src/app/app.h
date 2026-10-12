/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "plat.h"
#include "../dsp/omni.h"
#ifndef OM_VERSION
#define OM_VERSION "DEV"
#endif
typedef struct { int16_t par[P_NPARAMS]; } project_t;
extern project_t proj;
int project_load(void);
void project_apply(void);
void ui_init(void);
void ui_frame(void);
void ui_input_only(void);
int ui_dirty(void);
void ui_say(const char *a, const char *b);
