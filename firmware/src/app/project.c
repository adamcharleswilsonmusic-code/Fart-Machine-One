/* SPDX-License-Identifier: GPL-3.0-only */
/* Deliberately no song/project storage: predictable defaults on every power-on. */
project_t proj;
int project_load(void) { int i; for(i=0;i<P_NPARAMS;i++) proj.par[i]=om_param_info(i)->def; return 0; }
void project_apply(void) { int i; for(i=0;i<P_NPARAMS;i++) om_set(i,proj.par[i]); }
