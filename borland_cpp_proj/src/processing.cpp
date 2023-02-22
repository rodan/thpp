//----------------------------------------------------------------------------
// image processing header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//----------------------------------------------------------------------------

#include <system.hpp>

//----------------------------------------------------------------------------
#include <time.h>
#include <stdio.h>
#include <dos.h>
#include <math.h>
#include <vcl/clipbrd.hpp>
#include "processing.h"

#include "general.h"
#include "analysis.h"

#include "form.h"
#include "unit2.h"
#include "unit3.h"
#include "unit4.h"

#pragma nopackwarning

extern struct termograma t;
extern struct bmph8bit bmph;

//extern char curpath[1000],curpathp[1000];
extern int sg_current_row;

extern TPoint Origin, MovePt;
extern TPoint Origin2, MovePt2;

extern TDrawingTool DrawingTool;
extern AnsiString CurrentFile;
extern int Xref,Yref;

extern unsigned char must_change_grad;
extern unsigned char shrink;
extern int sg_current_row;

//extern unsigned aa;
extern float zoom;

extern int fheight;
extern bool Drawing,moving;


extern unsigned char vpl_data[12][768];
extern unsigned char vpl_current;

struct mask_struct msk;
int deriv;

//----------------------------------------------------------------------------

void mkbmp(char *fn,char *nfn)
{
char buff[200];

open_dtv(fn);
create_bmp(nfn);
sprintf(buff,"> file %s created",nfn);
Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

float e(int x,int y,int size)
 {return msk.c[size+x][size+y];}

float w(int x,int y,int size)
 {return msk.w[size+x][size+y];}

void e(int x,int y,int size,float v)
 {msk.c[size+x][size+y]=v;}

void w(int x,int y,int size,float v)
 {msk.w[size+x][size+y]=v;}

//----------------------------------------------------------------------------

void reset_mask(int dim)
{
int i,j;
for (j=-dim;j<=dim;j++)
        for (i=-dim;i<=dim;i++)
                e(i,j,dim,0);
}

void reset_w(int dim)
{
int i,j;
for (j=-dim;j<=dim;j++)
        for (i=-dim;i<=dim;i++)
                w(i,j,dim,0);
}

//----------------------------------------------------------------------------
void fill_w(int x, int y, FILE *dtvfile,int size,int fill_type)
{
int i,j;
float v;

if (fill_type==0)
  {for (j=-size;j<=size;j++)
    {
    for (i=-size;i<=size;i++)
        {if (e(i,j,size))
            {
            v=get_pixel_temp(x+i,y+j);
            w(i,j,size,v);
            }
         }
    }
  }
else
  {for (j=-size;j<=size;j++)
    {
    for (i=-size;i<=size;i++)
        {if (e(i,j,size))
            {
            v=get_temp(x+i,y+j);
            w(i,j,size,v);
            }
         }
    }
  }

}

//----------------------------------------------------------------------------

float statistic_medium(int x,int y,int mask_size)
{
float v,card,result;
int i,j;

v=0;
card=(2*mask_size+1)*(2*mask_size+1);

for (j=-mask_size;j<=mask_size;j++)
 for (i=-mask_size;i<=mask_size;i++) {v+=get_pixel_temp(x+i,y+j);}

result=(1/card)*v;
msk.stat_medium=result;
return result;
}

//----------------------------------------------------------------------------

float statistic_variance(int x,int y,int mask_size)
{
float sg2,card,medium,result;
int i,j;

sg2=0;
card=(2*mask_size+1)*(2*mask_size+1);

for (j=-mask_size;j<=mask_size;j++)
 for (i=-mask_size;i<=mask_size;i++)
   {sg2+=(get_pixel_temp(x+i,y+j))*(get_pixel_temp(x+i,y+j));
   }
medium=statistic_medium(x,y,mask_size);
result=(1/card)*sg2-medium*medium;
msk.stat_variance=result;
return result;
}

//----------------------------------------------------------------------------

float statistic_mw_medium(int x,int y,int mask_size)
{
float v,card,result;
int i,j;

v=0;
card=0;

for (j=-mask_size;j<=mask_size;j++)
 for (i=-mask_size;i<=mask_size;i++)
   {v+=e(i,j,2)*get_pixel_temp(x+i,y+j);
    card+=e(i,j,2);
   }

result=(1/card)*v;
msk.stat_medium=result;
return result;
}

//----------------------------------------------------------------------------

float statistic_mw_variance(int x,int y,int mask_size)
{
float sg2,card,medium,result;
int i,j;

sg2=0;
card=0;

for (j=-mask_size;j<=mask_size;j++)
 for (i=-mask_size;i<=mask_size;i++)
   {sg2+=(get_pixel_temp(x+i,y+j))*(get_pixel_temp(x+i,y+j));
   card+=e(i,j,2);
   }
medium=statistic_mw_medium(x,y,mask_size);
result=(1/card)*sg2-medium*medium;
msk.stat_variance=result;
return result;
}

//----------------------------------------------------------------------------

float statistic_local_filter(int x,int y,int mask_size,int ftype)
{
//ftype==0  : median filter
//ftype==10 : Low Pass Filter
//ftype==20 : High Pass Filter
//ftype==254 : No Pass Filter
//ftype==255 : All Pass Filter

//median
if (ftype==0) {return statistic_medium(x,y,mask_size);}
//no pass
if (ftype==254) {return 0;}
//all pass
if (ftype==255) {return get_pixel_temp(x,y);}

return -255; //error, filter not implemented
}

//----------------------------------------------------------------------------

void fill_mask(int maskname,int size,int version)
{ //maskname:10 median, 20 nagao[0-8]
int i,j;
float v;

float median[201][201];
float nagao[8][5][5]=
{ {{0, 1, 1, 1, 0}, {0, 1, 1, 1, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},  //nagao_N
  {{1, 1, 0, 0, 0}, {1, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},  //nagao_NW
  {{0, 0, 0, 0, 0}, {1, 1, 0, 0, 0}, {1, 1, 1, 0, 0}, {1, 1, 0, 0, 0}, {0, 0, 0, 0, 0}},  //nagao_W
  {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {1, 1, 1, 0, 0}, {1, 1, 0, 0, 0}},  //nagao_SW
  {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 1, 1, 1, 0}, {0, 1, 1, 1, 0}},  //nagao_S
  {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 1, 1, 0}, {0, 0, 1, 1, 1}, {0, 0, 0, 1, 1}},  //nagao_SE
  {{0, 0, 0, 0, 0}, {0, 0, 0, 1, 1}, {0, 0, 1, 1, 1}, {0, 0, 0, 1, 1}, {0, 0, 0, 0, 0}},  //nagao_E
  {{0, 0, 0, 1, 1}, {0, 0, 1, 1, 1}, {0, 0, 1, 1, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}}; //nagao_NE


//v=0; //????????/
if (maskname==10)
  {for (i=-size;i<=size;i++) for (j=-size;j<=size;j++) {median[i][j]=1;}

   v=(2*size+1)*(2*size+1);
   msk.coef=1.000/v;

   for (j=-size;j<=size;j++)
    {for (i=-size;i<=size;i++)
        {v = median[i][j];
         e(i,j,2,msk.coef*v);
        }
    }
   }

if (maskname==20)
  {for (j=-2;j<=2;j++)
   {for (i=-2;i<=2;i++)
    {v=nagao[version][j+2][i+2]/7;
     e(i,j,2,v);
    }
   }
  }

}

//----------------------------------------------------------------------------

void loadmskfile(char *fn)
{
FILE *maskf;
int i,j;
float v;

 maskf=fopen(fn,"rb");

 for (j=0;j<5;j++)
    for(i=0;i<5;i++)
        {fread(&v,sizeof(float),1,maskf);
         msk.c[i][j]=v;
        }
 fclose(maskf);
}

//----------------------------------------------------------------------------

void savemskfile(char *fn)
{
FILE *maskf;
int i,j;
float v;

 maskf=fopen(fn,"wb");

 for (j=0;j<5;j++)
    for(i=0;i<5;i++)
        {v=msk.c[i][j];
         fwrite(&v,sizeof(float),1,maskf);
        }
 fclose(maskf);
}

//----------------------------------------------------------------------------

void mask2form4()
{
int i,j;
char tempp[12];

for (j=-2;j<=2;j++)
   {
	for (i=-2;i<=2;i++)
     {
      sprintf(tempp,"%.4f",e(i,j,2));
      Form4->StringGrid1->Cells[i+3][j+3]=tempp;}///?
   }
}

//----------------------------------------------------------------------------

void form42mask()
{
int i,j;
float v1[5][5]={ {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0,}, {0, 0, 0, 0,}};
float v2; //????????????
float coef;

coef=0;
deriv=0;

for (j=0;j<=2*2;j++)
   {
	for (i=0;i<=2*2;i++)
     {//debug_me
      //if ((AnsiString)Form4->StringGrid1->Cells[i+1][j+1] != NULL )
            {sscanf(Form4->StringGrid1->Cells[i+1][j+1].c_str(),"%f",&v1[i][j]);
            if ((v1[i][j]<-20000) || (v1[i][j]>20000)) {v1[i][j]=0.00;}
            coef+=v1[i][j];
            }
     }
   }

   if (coef==0) {coef=1;deriv=1;}
   if ((coef<0.001) && (coef>-0.001)) {deriv=1;coef=1;} //derivative kernel not well centered

for (j=-2;j<=2;j++)
   {
	for (i=-2;i<=2;i++)
     {v2=(1.00/coef)*v1[i+2][j+2];
      e(i,j,2,v2);
     }
   }
}

//----------------------------------------------------------------------------

float mw_convol(int mask_size)
{
int i,j;
float ret;

ret=0;

for (j=-mask_size;j<=mask_size;j++)
   {
	for (i=-mask_size;i<=mask_size;i++)
     {
      ret+=e(i,j,mask_size)*w(i,j,mask_size);
     }
   }

return ret;
}

//----------------------------------------------------------------------------

void mklut(char *fn,char *nfn,float nmin, float nmax)
{
float nres,vmin,vres,vmax,temp;
int i,ovf=0;
char buff[200];
struct termograma *n;

 open_dtv(fn);

 vmin=t.th.tsc[1];
 vres=t.th.tsc[0];
 vmax=256*vres+vmin;

 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 if (nmin<vmin) nmin=vmin;
 if (nmax>vmax) nmax=vmax;
 nres=(nmax-nmin)/256.00001;
 n->th.tsc[1]=nmin;
 n->th.tsc[0]=nres;


 for(i=0;i<(n->th.nst*n->th.nstv);i++)
        {
         temp=t.th.atf[i+110]*vres+vmin;
         if (temp>nmax) {temp=nmax-0.0001;ovf++;}
         if (temp<nmin) {temp=nmin+0.0001;ovf++;}
         n->th.atf[i+110]=floor((temp-nmin)/nres);
        }

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_lut(%06.2f, %06.2f) overflow=%06.2f%%",nfn,nmin,nmax,ovf*.0015751);
 Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_normal(char *fn,char *nfn)
{
float min,max;
int i;
float temp;

 open_dtv(fn);
 min=1000.01;
 max=-1000.01;

 for(i=110;i<63598;i++)
   {temp=t.th.tsc[0]*t.th.atf[i]+t.th.tsc[1];
    if (temp>max) max=temp;
    if (temp<min) min=temp;
   }

mklut(fn,nfn,min+0.001,max-0.001);
}

//----------------------------------------------------------------------------

void mk_level_slice(char *fn,char *nfn,int n_cluster)
{
int i;
char buff[200];
struct termograma *n;

open_dtv(fn);

n=(termograma *)malloc(sizeof(termograma));
init_dtv(n,t.th.nst,t.th.nstv,1);
cpy_dtv(n,&t,0);

for(i=0;i<(t.th.nst*t.th.nstv);i++)
    {n->th.atf[i+110]=floor(n_cluster*t.th.atf[i+110]/256)*256/(n_cluster-1)-0.001;}

create_dtv(n,nfn);
destroy_dtv(n);

sprintf(buff,"> file %s created w/proc mk_level_slice(%d)",nfn,n_cluster);
Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_m(char *fn,char *nfn,int sensit, int mask_size, int fill_type)
{
 if (deriv) mk_mask_der(fn,nfn,sensit,mask_size,fill_type);
 else mk_mask_med(fn,nfn,mask_size);
}

//----------------------------------------------------------------------------

void mk_mask_med(char *fn,char *nfn,int mask_size)
{
struct termograma *n;
float temp;
int i,j;
char buff[200];

 open_dtv(fn);
 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 for (j=0;j<mask_size;j++)
        for (i=0;i<(n->th.nst);i++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
 for (j=(n->th.nstv-mask_size);j<(n->th.nstv);j++)
        for (i=0;i<n->th.nst;i++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
 for (i=0;i<mask_size;i++)
        for (j=0;j<(n->th.nstv);j++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
 for (i=(n->th.nst-mask_size);i<(n->th.nst);i++)
        for (j=0;j<(n->th.nstv);j++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];

 for (j=mask_size;j<(n->th.nstv-mask_size);j++)
  {
   for (i=mask_size;i<(n->th.nst-mask_size);i++)
      {
       fill_w(i,j,t.tf,mask_size,0);
       temp=mw_convol(mask_size);
       n->th.atf[j*n->th.nst+i+110]=floor(temp);
       if (temp>255) {n->th.atf[j*n->th.nst+i+110] = 255;}
       if (temp<0) {n->th.atf[j*n->th.nst+i+110] = 0;}
      }
  }

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_median()",nfn);
 Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_mask_der(char *fn,char *nfn, int n_dt, int mask_size, int fill_type)
{
float temp,res,tmin,tmax;
int i,j,ovf=0;
char buff[200];
struct termograma *n;

 tmin=0;
 res=n_dt/256.00;
 tmax=n_dt;

 open_dtv(fn);

 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 n->th.tsc[0]=res;
 n->th.tsc[1]=tmin;

 for (j=0;j<mask_size;j++)
        for (i=0;i<n->th.nst;i++)
                n->th.atf[j*n->th.nst+i+110]=0;
 for (j=n->th.nstv-mask_size;j<n->th.nstv;j++)
        for (i=0;i<n->th.nst;i++)
                n->th.atf[j*n->th.nst+i+110]=0;
 for (i=0;i<mask_size;i++)
        for (j=0;j<n->th.nstv;j++)
                n->th.atf[j*n->th.nst+i+110]=0;
 for (i=n->th.nst-mask_size;i<n->th.nst;i++)
        for (j=0;j<n->th.nstv;j++)
                n->th.atf[j*n->th.nst+i+110]=0;

if (fill_type==0)
 {
  for (j=mask_size;j<n->th.nstv-mask_size;j++)
  {for (i=mask_size;i<n->th.nst-mask_size;i++)
      {
       fill_w(i,j,t.tf,mask_size,0);
       temp=fabs(mw_convol(mask_size));
       n->th.atf[j*n->th.nst+i+110]=floor(temp);
       if (temp>255) {n->th.atf[j*n->th.nst+i+110] = 255;ovf++;}
       if (temp<0) {n->th.atf[j*n->th.nst+i+110] = 0;ovf++;}
      }
  }
 }

if (fill_type==1)
  {for (j=2;j<n->th.nstv-2;j++)
    {for (i=2;i<n->th.nst-2;i++)
      {
       fill_w(i,j,t.tf,mask_size,1);
       temp=fabs(mw_convol(mask_size));
       n->th.atf[j*n->th.nst+i+110]=floor((temp-tmin)/res);
       if (temp>tmax) {n->th.atf[j*n->th.nst+i+110] = tmax-0.001;ovf++;}
       if (temp<tmin) {n->th.atf[j*n->th.nst+i+110] = tmin+0.001;ovf++;}
      }
    }
  }

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_m_deriv(%d) ovf=%07.3f%%",nfn,n_dt,ovf*.0015751);
 Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_mhrf(char *fn, char *nfn, float t1, float t2, float t3, float wmax, int options)
{
float stat_var[100],dsv;
int i,j,k;
int show_pikes=0,show_edges=0,show_near_edges=0,show_homogeneous=0,rpl_black=0,rpl_back=0;
char buff[200];
int pike=0,edge=0,near_edge=0,homog=0;
int rpl_col;
struct termograma *n;

 open_dtv(fn);

 n = (termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 if (options>31) {rpl_back=1;options-=32;}
 if (options>15) {rpl_black=1;options-=16;}
 if (options>7) {show_homogeneous=1;options-=8;}
 if (options>3) {show_near_edges=1;options-=4;}
 if (options>1) {show_edges=1;options-=2;}
 if (options>0) {show_pikes=1;}

 if (rpl_back) {rpl_col=floor(atof(Form4->MaskEdit7->Text.c_str()));}

 if (rpl_black && !show_homogeneous)
  {for (j=0;j<wmax;j++) for (i=0;i<n->th.nst;i++) n->th.atf[j*n->th.nst+i+110]=0;
   for (j=n->th.nstv-wmax;j<n->th.nstv;j++) for (i=0;i<n->th.nst;i++) n->th.atf[j*n->th.nst+i+110]=0;
   for (i=0;i<wmax;i++) for (j=0;j<n->th.nstv;j++) n->th.atf[j*n->th.nst+i+110]=0;
   for (i=n->th.nst-wmax;i<n->th.nst;i++) for (j=0;j<n->th.nstv;j++) n->th.atf[j*n->th.nst+i+110]=0;
  }
 else
  {for (j=0;j<wmax;j++) for (i=0;i<n->th.nst;i++) n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
   for (j=n->th.nstv-wmax;j<n->th.nstv;j++) for (i=0;i<n->th.nst;i++) n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
   for (i=0;i<wmax;i++) for (j=0;j<n->th.nstv;j++) n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
   for (i=n->th.nst-wmax;i<n->th.nst;i++) for (j=0;j<n->th.nstv;j++) n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
  }



 for (j=wmax;j<n->th.nstv-wmax;j++)
  {
  for (i=wmax;i<n->th.nst-wmax;i++)
      {
        k=1;
        stat_var[k]=statistic_variance(i,j,k);

       for (k=2;k<=wmax;k++)
       {
        stat_var[k]=statistic_variance(i,j,k);
        dsv=stat_var[k]-stat_var[k-1];
         if (dsv < -t1)
            {//salt & pepper noise usually
             pike++;
             if (show_pikes) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)round(statistic_local_filter(i,j,k-1,0));}
             else { if (rpl_black) {n->th.atf[j*n->th.nst+i+110]=0;}
                    if (rpl_back) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)rpl_col;}
                  }
             k=wmax+1;
            }
          else
            { if ((dsv > t2) || ((dsv < t2) && (stat_var[k] > t3)) )
                   { if (k>floor(wmax/2)) {//near edge
                                    near_edge++;
                                    if (show_near_edges) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)round(statistic_local_filter(i,j,k-1,0));}
                                    else { if (rpl_black) {n->th.atf[j*n->th.nst+i+110]=0;}
                                           if (rpl_back) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)rpl_col;}
                                         }
                                    k=wmax+1;
                                   }
                     else
                      { //semi-window configuration
                        while (k<wmax)
                           {k++;
                            stat_var[k]=statistic_variance(i,j,k);
                            dsv=stat_var[k]-stat_var[k-1];
                               if ((dsv>t2) || (stat_var[k]>t3) || (k==wmax))
                                 {//important data (edges)
                                 edge++;
                                 if (show_edges) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)round(statistic_local_filter(i,j,k,255));}
                                 else { if (rpl_black) {n->th.atf[j*n->th.nst+i+110]=0;}
                                        if (rpl_back) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)rpl_col;}
                                      }
                                 k=wmax+1;
                                 }
                               else
                                 {//homog2++;
                                 }
                           }
                      }
                   }
              else
                   { if (k==wmax)
                      {//homogeneous area
                       homog++;
                       if (show_homogeneous) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)round(statistic_local_filter(i,j,k,0));}
                       else { if (rpl_black) {n->th.atf[j*n->th.nst+i+110]=0;}
                              if (rpl_back) {n->th.atf[j*n->th.nst+i+110]=(unsigned char)rpl_col;}
                            }
                       k=wmax+1;
                      }
                   }
            }
       }
      }
  }

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_MHRF(t1:%.2f t2:%.2f t3:%.2f wmax:%.0f)",nfn,t1,t2,t3,wmax);
 Form3->Memo1->Lines->Add(buff);
 sprintf(buff," pike:%d edge:%d near-edge:%d homogeneous:%d ",pike,edge,near_edge,homog);
 Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_Nagao(char *fn,char *nfn)
{
float stat_var[8],stat_med[8],temp,comp;
char buff[200];
int i,j,k,finger;
struct termograma *n;

 open_dtv(fn);

 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 for (j=0;j<2;j++)
        for (i=0;i<n->th.nst;i++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
 for (j=246;j<n->th.nstv;j++)
        for (i=0;i<n->th.nst;i++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
 for (i=0;i<2;i++)
        for (j=0;j<n->th.nstv;j++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];
 for (i=254;i<n->th.nst;i++)
        for (j=0;j<n->th.nstv;j++)
                n->th.atf[j*n->th.nst+i+110]=t.th.atf[j*n->th.nst+i+110];

 for (j=2;j<246;j++)
  {
   for (i=2;i<254;i++)
      { fill_w(i,j,t.tf,2,1);
        comp=90000;
        for (k=0;k<8;k++)
          {fill_mask(20,2,k);
           stat_var[k]=statistic_mw_variance(i,j,2);
           stat_med[k]=msk.stat_medium;
           if (stat_var[k]<comp) {comp=stat_var[k];finger=k;}
          }
        temp=stat_med[finger];
        n->th.atf[j*n->th.nst+i+110]=(unsigned char)round(temp);
      }
  }

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_Nagao()",nfn);
 Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_th_gradient(char *fn,char *nfn,int n_dt,int type)
{
float tmin,res,tmax,temp,temp2;
float cdr,cst,ls,lj;
int i,j,ovf=0;
char buff[200];
struct termograma *n;

 open_dtv(fn);

 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 tmin=n_dt*(-1.000001);
 res=n_dt/128.0000001;
 tmax=res*256+tmin;

 n->th.tsc[0]=res;
 n->th.tsc[1]=tmin;

        for (j=0;j<248;j++)
         {
          for (i=0;i<256;i++)
                {
                     if ((i!=0) && (i!=255) && (j!=0) && (j!=247))
                         {
                           cst=get_temp(i-1,j);
                           cdr=get_temp(i+1,j);
                           ls=get_temp(i,j-1);
                           lj=get_temp(i,j+1);
                           temp=ls-lj;
                           temp2=cdr-cst;
                           if (temp>tmax) {temp=tmax-0.0001;ovf++;}
                           if (temp<tmin) {temp=tmin;ovf++;}
                           if (temp2>tmax) {temp2=tmax-0.0001;ovf++;}
                           if (temp2<tmin) {temp2=tmin;ovf++;}
                           if(fabs(temp)>fabs(temp2)) {n->th.atf[j*256+i+110]=floor((temp-tmin)/res);}
                           else {n->th.atf[j*256+i+110]=floor((temp2-tmin)/res);};
                         }
                        else {n->th.atf[j*256+i+110]=floor((-tmin)/res);};
                }
         }

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_th_gradient(%d) overflow=%06.2f%%",nfn,n_dt,ovf*.0015751);
 Form3->Memo1->Lines->Add(buff);
}

//----------------------------------------------------------------------------

void mk_histeq(char *fn,char *nfn)
{
int i;
char buff[200];
struct termograma *n;

 open_dtv(fn);
 get_hist();
 get_histcum();

 n=(termograma *)malloc(sizeof(termograma));
 init_dtv(n,t.th.nst,t.th.nstv,1);
 cpy_dtv(n,&t,0);

 for (i=0;i<256;i++)
        {t.histeq[i]=round(255*(t.histcum[i]-t.histcum[0])/(1-t.histcum[0]));}

 for(i=0;i<(n->th.nst*n->th.nstv);i++)
        {n->th.atf[i+110]=t.histeq[t.th.atf[i+110]];}

 create_dtv(n,nfn);
 destroy_dtv(n);

 sprintf(buff,"> file %s created w/proc mk_histeq()",nfn);
 Form3->Memo1->Lines->Add(buff);
}

