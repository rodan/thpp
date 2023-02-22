//----------------------------------------------------------------------------
// image processing header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#ifndef processingH
#define processingH

#include <dir.h>
#include <time.h>
#include <stdio.h>
#include <dos.h>
#include <math.h>

//----------------------------------------------------------------------------

struct mask_struct
{
public:
 int size;
 char *name;
 char *info;
 FILE *mf;
 float c[201][201];
 float w[201][201];
 char *str;
 float coef;
// char origfile[1000];
 float stat_medium;
 float stat_variance;
 float stat_dispersion;
};

 void reset_mask(int dim);
 void reset_w(int dim);
 void fill_mask(int maskname,int mask_size,int version);
 void fill_w(int x, int y, FILE *fn,int window_size,int fill_type);
 float apply_mask();
 float mw_convol(int size);
 float statistic_medium(int x,int y,int mask_size);
 float statistic_variance(int x,int y,int mask_size);
 float statistic_local_filter(int x,int y,int mask_size,int type);
 float statistic_mw_medium(int x,int y,int mask_size);
 float statistic_mw_variance(int x,int y,int mask_size);
 float statistic_mw_local_filter(int x,int y,int mask_size,int type);
 void mask2form4();
 void form42mask();
// mask(int d=100){dim=d;reset_mask();}
 void apply_mask2window(int masksize);
 void e(int x,int y,int size,float v);
 float e(int x,int y,int size);
 void w(int x,int y,int size,float v);
 float w(int x,int y,int size);

//----------------------------------------------------------------------------

void mkbmp(char *fn,char *nfn);
void mkasc(char *fn,char *nfn);
void mklut(char *fn,char *nfn,float nmin, float nmax);
void mk_normal(char *fn,char *nfn); //automatic LUT

void mk_histeq(char *fn,char *nfn);

void loadmskfile(char *fn);
void savemskfile(char *fn);

//detection of kernel type
void mk_m(char *fn,char *nfn, int n_dt, int size, int fill_type);
//smoothing filter
void mk_mask_med(char *fn, char *nfn, int size);
//derivative filter
void mk_mask_der(char *fn, char *nfn, int n_dt, int size, int fill_type);
void mk_mask_transf(char *fn, char *nfn, int size);
//maximum homogeneous region filter - Yifeng Wu & Henri Maitre 'Optical Engineering' 31(8), 1785-1792(August 1992)
void mk_mhrf(char *fn, char *nfn, float t1, float t2, float t3, float wmax, int options);
//Nagao filter
void mk_Nagao(char *fn,char *nfn);

void mk_level_slice(char *fn,char *nfn,int n);
void mk_th_gradient(char *fn,char *nfn,int n_dt,int type);

void mk_heat_capacity(char *output); // temporal stuff
void mk_stv(char *output); // spatio_temporal_var

//----------------------------------------------------------------------------

#endif
