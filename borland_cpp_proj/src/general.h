//----------------------------------------------------------------------------
// general header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#ifndef generalH
#define generalH

//----------------------------------------------------------------------------
#include <vcl/dstring.h>
#include <stdio.h>
#include <Types.hpp>
#include <Graphics.hpp>
#pragma nopackwarning

//---------------------------------------------------------------------------
struct dtv_header
{
public:               // offsets
 unsigned char disz;  //0
 unsigned char diver; //1
 short unsigned shtp; //2
 char fltp;           //4
 char tc;             //5
 char cftime[4];      //6
 short unsigned nst;  //10
 short unsigned nstv; //12
 short unsigned shtph;//14
 short int frn;       //16
 short unsigned nstl; //  18
 unsigned char bgx;   //  20
 unsigned char bgy;   //  21
 float hszk;          //  22
 float vszk;          //  26
 float frtk;          //  30
 float tsc[3];        //34 38 42
 char inform[64];     //46
 float ttmin;         //100
 float ttmax;         //104
 char allinfo[111];   //108
 unsigned char *atf;
};

//---------------------------------------------------------------------------

enum drwtype {d_none			=0,
			  d_linear_normal	=1,
              d_linear_aliased	=2,
              d_bilinear_normal	=3,
              d_bilinear_aliased=4,
              d_area_normal		=5
              };
enum calctype {c_none	=0,
			   c_linear	=1,
               c_area	=5
              };

enum TDrawingTool {dtLine, dtRectangle, dtEllipse, dtRoundRect};

struct termograma
{
public:
 struct dtv_header th;
 FILE *tf;
 FILE *pf;
 char *bmpf;
 AnsiString tfstr;
 float *pt;
 double mmpp,mmlen,pixellen,pixelarea;
 unsigned np;
 int xi,yi,xf,yf;
 float panta,ordonata;
 float tmin,tmax,tmed;
 bool iscalc;
 bool iscalcrect;
 bool isloaded;
 enum drwtype z;
 unsigned d;
 unsigned c;
 char dtvstr[1000];
 float hist[256];
 float histcum[256];
 float histeq[256];
 float histmax;
};

//---------------------------------------------------------------------------

struct bmph8bit
{
public:
 unsigned char h1[26];
 unsigned char h2[28];
 unsigned char pale[256*4];
};

//void viewdtv(char *fn);
void open_dtv(char *fn);
void load_ntermo(int n);
void close_dtv(void);
int init_dtv(termograma *dest, short unsigned size_x, short unsigned size_y, short unsigned slide_no);
int create_dtv(termograma *dest, char *filename);
int cpy_dtv(termograma *dest, termograma *src, int options);
int destroy_dtv(termograma *dest);

void get_name(const char *s,const char *diff,const char *ext2, char *output);
void get_n_name(char *prefix,char *diff,char *ext2,int i);

void create_bmp(char *bmpf);
void view_dtv(char *fn);
void view_dtvprev(char *fn);

void reset_dtvprev();
void calcul();
void calculrect();
void refresh_calc();

void show_lutgraph(int x1,int x2);
void refresh_image();

void show_info();
void show_graph();
void reset_graph();

void dts();
void dts4();
void show_grad();

void show_hist();
void lut_prev(int n);
void show_lutprev(int n);
void show_lutgraph(int tlmin, int tlmax);

float get_temp(unsigned x, unsigned y);
unsigned char get_pixel_temp(unsigned x, unsigned y);
float get_temp_n(unsigned x, unsigned y, int i);
void get_info(char *fn);

int round(float number);
int sort_function( const void *a, const void *b);

void DrawShape(TPoint TopLeft, TPoint BottomRight, TPenMode AMode);
void DrawShapeimg3(TPoint TopLeft, TPoint BottomRight, TPenMode AMode);

//---------------------------------------------------------------------------

#endif

