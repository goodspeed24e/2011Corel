#ifndef PROTOTYP_H
#define PROTOTYP_H

#include "common.h"
#include "qsize.h"
#include "parm.h"

extern int  close();
extern int  fclose();
extern int  fprintf();
extern int  fread();
extern int  fscanf();
extern int  fwrite();
extern int  open();
extern int  printf();
extern int  printf();
extern int  read();
extern int  sscanf();
extern int  system(char*);
extern int  write();



extern real PSF_LPC_MEM a10[];
extern real PSF_LPC_MEM k10;
extern void adapt_coder();
extern void bsf_adapter (real input[], real COEFF_MEM p_out[]);
extern void cb_excitation (int  ix, real v[]);
extern int  cb_index (real[]);
extern void code_frame();
extern void gain_adapter (real[], COEFF_MEM real[]);
extern int  get_index();
extern void init_bsf_adapter(real COEFF_MEM []);
extern void init_coder();
extern void init_gain_adapter(real COEFF_MEM[]);
extern void init_io();
extern void init_postfilter();
extern void init_pwf_adapter (real COEFF_MEM [], real COEFF_MEM []);
extern void iresp_vcalc(real COEFF_MEM[],real COEFF_MEM[],real COEFF_MEM[],real[]);
extern void mem_update (real input[], real output[]);
extern void postfilter(real[], real[]);
extern real predict_gain(real[], real[]);
extern void psf_adapter(real[]);
extern void put_index(int x);
extern void pwf_adapter (real input[], real COEFF_MEM z[],real COEFF_MEM p[]);
extern void pwfilter2(real QMEM input[], real output[]);
extern int  read_sound_buffer(int n, real buf[]);
extern void shape_conv(real h[], real shen[]);
extern void trev_conv(real h[], real target[], real pn[]);
extern void update_sfilter(real[]);
extern int  write_sound_buffer(int n, real buf[]);
extern void zresp(real[]);

#endif /*PROTOTYP_H*/
