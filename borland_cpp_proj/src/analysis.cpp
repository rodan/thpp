//----------------------------------------------------------------------------
// analysis header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//----------------------------------------------------------------------------

#include <system.hpp>

#include <math.h>
#include "analysis.h"

#include "general.h"
#include "koh.h"
#include "processing.h"
#include "unit3.h"

#pragma hdrstop
#pragma nopackwarning

extern struct termograma t;
extern struct bmph8bit bmph;

extern int sg_current_row;

extern TPoint Origin, MovePt;
extern TPoint Origin2, MovePt2;

extern TDrawingTool DrawingTool;
extern int Xref,Yref;

extern unsigned char must_change_grad;
extern unsigned char shrink;
extern int sg_current_row;

extern float zoom;

extern int fheight;
extern bool Drawing,moving;

extern unsigned char vpl_data[12][768];
extern unsigned char vpl_current;

//----------------------------------------------------------------------------

void get_hist()
{
int i,j;
int xip,xfp,yip,yfp,tmp;

t.histmax=0;
for (i=0;i<256;i++) {t.hist[i]=0;}

 if (t.d == 5)
   {
    xip=t.xi;
    xfp=t.xf;
    yip=t.yi;
    yfp=t.yf;

    if (xip>xfp) {tmp=xip;xip=xfp;xfp=tmp;}
    if (yip<yfp) {tmp=yip;yip=yfp;yfp=tmp;}

    t.np=(xfp-xip+1)*(yip-yfp+1);

    for (i=yfp;i<=yip;i++)
      {for (j=xip;j<=xfp;j++)
        {
         t.hist[t.th.atf[i*t.th.nst+j+110]]++;
        }
      }
    for (i=0;i<256;i++)
       {
        t.hist[i]=t.hist[i]/t.np;
        if (t.hist[i]>t.histmax) {t.histmax=t.hist[i];}
       }
   }
 else
   {
    for (i=0;i<(t.th.nst*t.th.nstv);i++) {t.hist[t.th.atf[i+110]]++;}
    for (i=0;i<256;i++)
       {
        t.hist[i]=t.hist[i]/(t.th.nst*t.th.nstv);
        if (t.hist[i]>t.histmax) {t.histmax=t.hist[i];}
       }
   }

 show_hist();
}

//----------------------------------------------------------------------------

void get_histcum()
{
int i,j;

 for (i=0;i<256;i++) {t.histcum[i]=0;}
 for (i=0;i<256;i++) for (j=0;j<=i;j++) {t.histcum[i]+=t.hist[j];}
}

//----------------------------------------------------------------------------

int csom_segmentation(char *fn, char *nfn, int window_size, int output_neurons, int max_epoch, int neigh_neurons, float min_evolution, float max_learn_rate, int options, bool *show_layer)
{
struct csom csom_01;
struct termograma *n;
float tmin,tmax,res;
char buff[400];
int i,j,x,y;
int eticheta;
int pixel_pos;
int csom_max_v,csom_max_h,elements;

 open_dtv(fn);

 tmin=0;
 tmax=output_neurons-1;
 res=(float)(tmax-tmin)/256;

 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 n->th.tsc[0]=res;
 n->th.tsc[1]=tmin;

 csom_max_v=n->th.nstv-2*window_size;
 csom_max_h=n->th.nst-2*window_size;

 elements=csom_max_v*csom_max_h;

 csom_init(csom_01, output_neurons, elements, max_epoch, neigh_neurons, min_evolution, max_learn_rate, options, show_layer);

 for (y=0;y<csom_max_v;y++)
   for (x=0;x<csom_max_h;x++)
        {pixel_pos=y*(256-(2*window_size))+x;
         if (csom_01.use_0) {csom_01.x0[pixel_pos]=statistic_medium(x+window_size,y+window_size,window_size);}
         if (csom_01.use_1) {csom_01.x1[pixel_pos]=statistic_variance(x+window_size,y+window_size,window_size);}
        }

 csom_main(csom_01);

for (j=0;j<window_size;j++) for (i=0;i<256;i++) {n->th.atf[j*256+i+110]=0;}
for (j=248-window_size;j<248;j++) for (i=0;i<256;i++) {n->th.atf[j*256+i+110]=0;}
for (i=0;i<window_size;i++) for (j=0;j<248;j++) {n->th.atf[j*256+i+110]=0;}
for (i=256-window_size;i<256;i++) for (j=0;j<248;j++) {n->th.atf[j*256+i+110]=0;}

for (j=0;j<csom_max_v;j++)
    for (i=0;i<csom_max_h;i++)
        {eticheta=csom_01.csom_out[j*(256-2*window_size)+i];
         n->th.atf[(j+window_size)*256+i+window_size+110]=csom_01.colormap[eticheta];
        }

create_dtv(n,nfn);
destroy_dtv(n);

sprintf(buff,"> process csom_segmentation(%d, %d, %d, %.02f, %d, %d) > %s ", csom_01.input_neurons, csom_01.output_neurons, csom_01.max_neigh, csom_01.max_learn_rate, csom_01.max_epoch+1, window_size, nfn);
Form3->Memo1->Lines->Add(buff);

sprintf(buff,"> ending data -> epoch=%d;  evolution=%.04f", csom_01.epoch, csom_01.evolution);
Form3->Memo1->Lines->Add(buff);

if (csom_01.output_neurons<20)
 {
  sprintf(buff,"> neuron # | gray level | sinapses  ");
  Form3->Memo1->Lines->Add(buff);
  for (i=0;i<csom_01.output_neurons;i++)
    {sprintf(buff,">    %-.3d      |    %-.3d       |    %d",i ,(int)csom_01.colormap[i], csom_01.activations[i]);
     //sprintf(buff,">    %-.3d      |    %-.3d       |    %d      |   %f ",i ,(int)csom_01.colormap[i], csom_01.activations[i], csom_01.w0[i]);
     Form3->Memo1->Lines->Add(buff);
    }
}

sprintf(buff,"done");
Form3->Memo1->Lines->Add(buff);

csom_destruct(csom_01);
return 0;
}

//----------------------------------------------------------------------------


