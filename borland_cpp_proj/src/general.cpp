//----------------------------------------------------------------------------
// general header for dpp ver 1.x by Petre Rodan <rodan@subdimension.com>
//----------------------------------------------------------------------------

#include <system.hpp>

//----------------------------------------------------------------------------
#include "general.h"
#include <dir.h>
#include <time.h>
#include <stdio.h>
#include <dos.h>
#include <math.h>
#include <vcl/clipbrd.hpp>

#pragma hdrstop
#include "form.h"
#include "Unit3.h"
#include "Unit4.h"
#include "processing.h"
#include "analysis.h"
#include "palette.h"
#include "koh.h"

#pragma nopackwarning

extern unsigned char vpl_data[12][768];
extern unsigned char vpl_current;

//extern class mask m;

unsigned char must_change_grad=1;
unsigned char shrink=1;
int sg_current_row=0;
float zoom=2;
int fheight;
bool Drawing,moving;

struct bmph8bit bmph;
struct termograma t;

TPoint Origin, MovePt;
TPoint Origin2, MovePt2;
TDrawingTool DrawingTool;
AnsiString CurrentFile;
int Xref,Yref;

//---------------------------------------------------------------------------

void open_dtv(char *fn)
{
fpos_t fpos;

 close_dtv();
 t.tf=fopen(fn,"rb");
 fpos=10;fsetpos(t.tf,&fpos);fread(&t.th.nst,sizeof(t.th.nst),1,t.tf);
 fpos=12;fsetpos(t.tf,&fpos);fread(&t.th.nstv,sizeof(t.th.nstv),1,t.tf);
 fpos=16;fsetpos(t.tf,&fpos);fread(&t.th.frn,sizeof(t.th.frn),1,t.tf);
 fpos=34;fsetpos(t.tf,&fpos);fread(&t.th.tsc[0],sizeof(t.th.tsc[0]),1,t.tf);
 fpos=38;fsetpos(t.tf,&fpos);fread(&t.th.tsc[1],sizeof(t.th.tsc[0]),1,t.tf);
 fpos=46;fsetpos(t.tf,&fpos);fread(&t.th.inform,sizeof(char),64,t.tf);
 fpos=0;fsetpos(t.tf,&fpos);fread(&t.th.allinfo,sizeof(char),111,t.tf);

 if ((t.th.atf = (unsigned char *)malloc(t.th.nst*t.th.nstv*sizeof(unsigned char)+1) ) == NULL)
        {
         Application->MessageBox("unable to allocate memory", "fatal failure", MB_OK);
        }
 fpos=0;fsetpos(t.tf,&fpos);fread(t.th.atf,sizeof(char),t.th.nst*t.th.nstv,t.tf);
 t.isloaded=1;
 fclose(t.tf);
}

//---------------------------------------------------------------------------

void close_dtv(void)
{
int i;

if (t.isloaded) {free(t.th.atf);}
t.isloaded=0;
t.d=0;
t.np=0;
t.xi=0;
t.yi=0;
t.xf=0;
t.yf=0;
t.panta=0;
t.ordonata=0;
t.tmin=1000;
t.tmax=-1000;
t.iscalc=0;
t.iscalcrect=0;

for (i=0;i<256;i++)
 {t.hist[i]=0;
  t.histcum[i]=0;
  t.histeq[i]=0;
 }

Form3->Image2->Canvas->Brush->Color = clWindow;
Form3->Image2->Canvas->Pen->Color = clWindow;
Form3->Image2->Canvas->Rectangle(0,0,600,300);
reset_graph();
}

//----------------------------------------------------------------------------

int create_dtv(termograma *n, char *filename)
{
FILE *dtv_out;
fpos_t fpos;
int i;

 dtv_out=fopen(filename,"wb");

 fpos=0;fsetpos(dtv_out,&fpos);fwrite(&n->th.disz,sizeof(char),1,dtv_out);
 fpos=1;fsetpos(dtv_out,&fpos);fwrite(&n->th.diver,sizeof(char),1,dtv_out);
 fpos=2;fsetpos(dtv_out,&fpos);fwrite(&n->th.shtp,sizeof(short),1,dtv_out);
 fpos=4;fsetpos(dtv_out,&fpos);fwrite(&n->th.fltp,sizeof(char),1,dtv_out);
 fpos=5;fsetpos(dtv_out,&fpos);fwrite(&n->th.tc,sizeof(char),1,dtv_out);
 for (i=0;i<4;i++)
        {fpos=6+i;fsetpos(dtv_out,&fpos);fwrite(&n->th.cftime[i],sizeof(char),1,dtv_out);}
 fpos=10;fsetpos(dtv_out,&fpos);fwrite(&n->th.nst,sizeof(short),1,dtv_out);
 fpos=12;fsetpos(dtv_out,&fpos);fwrite(&n->th.nstv,sizeof(short),1,dtv_out);
 fpos=14;fsetpos(dtv_out,&fpos);fwrite(&n->th.shtph,sizeof(short),1,dtv_out);
 fpos=16;fsetpos(dtv_out,&fpos);fwrite(&n->th.frn,sizeof(short),1,dtv_out);
 fpos=18;fsetpos(dtv_out,&fpos);fwrite(&n->th.nstl,sizeof(short),1,dtv_out);
 fpos=20;fsetpos(dtv_out,&fpos);fwrite(&n->th.bgx,sizeof(short),1,dtv_out);
 fpos=21;fsetpos(dtv_out,&fpos);fwrite(&n->th.bgy,sizeof(short),1,dtv_out);
 fpos=22;fsetpos(dtv_out,&fpos);fwrite(&n->th.hszk,sizeof(float),1,dtv_out);
 fpos=26;fsetpos(dtv_out,&fpos);fwrite(&n->th.vszk,sizeof(float),1,dtv_out);
 fpos=30;fsetpos(dtv_out,&fpos);fwrite(&n->th.frtk,sizeof(float),1,dtv_out);
 fpos=34;fsetpos(dtv_out,&fpos);fwrite(&n->th.tsc[0],sizeof(float),1,dtv_out);
 fpos=38;fsetpos(dtv_out,&fpos);fwrite(&n->th.tsc[1],sizeof(float),1,dtv_out);
 fpos=42;fsetpos(dtv_out,&fpos);fwrite(&n->th.tsc[2],sizeof(float),1,dtv_out);
 fpos=46;fsetpos(dtv_out,&fpos);fwrite(&n->th.inform,sizeof(char),64,dtv_out);
 fpos=110;fsetpos(dtv_out,&fpos);
 for(i=0;i<=(n->th.nst*n->th.nstv);i++)
        {
         fputc(n->th.atf[i+110],dtv_out);
        }

 fclose(dtv_out);

return 0;
}

//----------------------------------------------------------------------------

int cpy_dtv(termograma *dest, termograma *src, int options)
{
 int i;

 dest->th.disz = src->th.disz;
 dest->th.diver = src->th.diver;
 dest->th.shtp = src->th.shtp;
 dest->th.fltp = src->th.fltp;
 dest->th.tc = src->th.tc;
 for (i=0;i<=4;i++)
        { dest->th.cftime[i] = src->th.cftime[i]; }
 dest->th.nst = src->th.nst;
 dest->th.nstv = src->th.nstv;
 dest->th.shtph = src->th.shtph;
 dest->th.frn = 1;
 dest->th.nstl = src->th.nstl;
 dest->th.bgx = src->th.bgx;
 dest->th.bgy = src->th.bgy;
 dest->th.hszk = src->th.hszk;
 dest->th.vszk = src->th.vszk;
 dest->th.frtk = src->th.frtk;
 for (i=0;i<=3;i++)
        { dest->th.tsc[i] = src->th.tsc[i]; }
 snprintf(dest->th.inform,63,"%s",src->th.inform);
 dest->th.ttmin=src->th.ttmin;
 dest->th.ttmax=src->th.ttmax;
 snprintf(dest->th.allinfo,110,"%s",src->th.allinfo);

 if (options)
        {
         // do smth to t.th.atf
        }

return 0;
}

//----------------------------------------------------------------------------

int destroy_dtv(termograma *dest)
{
 free(dest->th.atf);
 free(dest);
 
 return 0;
}

int init_dtv(termograma *dest,short unsigned size_x, short unsigned size_y, short unsigned slide_no)
{

 dest->th.nst=size_x;
 dest->th.nstv=size_y;
 dest->th.frn=slide_no;

 dest->th.atf=(unsigned char*)malloc(sizeof(unsigned char)*dest->th.nst*dest->th.nstv*dest->th.frn+200);

return 0;
}


//----------------------------------------------------------------------------

void get_name(const char *s,const char *diff,const char *ext2, char *output)
{
 char drive[MAXDRIVE];
 char dir[MAXDIR];
 char file[MAXFILE];
 char file2[MAXFILE];
 char ext[MAXEXT];
 char *name;

 if (!strcmp(diff,"prev"))
    {
     name = tempnam("\\tmp","prev");
     sprintf(output,"%s.%s",name,ext2);
     free(name);
    }
 else
    {
     fnsplit(s,drive,dir,file,ext);
     sprintf(file2,"%s%s",diff,file);
     fnmerge(output,drive,dir,file2,ext2);
    }
}

//---------------------------------------------------------------------------

void create_bmp(char *fn)
{
FILE *bmpf;
char bmp1[26]={'B','M','6',0XFC,0,0,0,0,0,0,0X36,4,0,0,0X28,0,0,0,0,1,0,0,0xF8,0,0,0};
char bmp2[28]={1,0,8,0,0,0,0,0,0,0XF8,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0};
short int sf;
unsigned int f,i;//c

memcpy(bmph.h1,bmp1,sizeof(bmp1));
memcpy(bmph.h2,bmp2,sizeof(bmp2));

for(sf=0;sf<256;sf++)
 {
  bmph.pale[sf*4+2]=floor(4*vpl_data[vpl_current][sf*3]);
  bmph.pale[sf*4+1]=floor(4*vpl_data[vpl_current][sf*3+1]);
  bmph.pale[sf*4+0]=floor(4*vpl_data[vpl_current][sf*3+2]);
  bmph.pale[sf*4+3]=0;
  }

bmpf=fopen(fn,"wb");
fwrite(&bmph,sizeof(bmph),1,bmpf);
for(f=0;f<248;f++){for(i=0;i<256;i++) {fputc(t.th.atf[110+(248-f-1)*256+i],bmpf);}}
fclose(bmpf);
}

//---------------------------------------------------------------------------

void view_dtv(char *fn)
{
 open_dtv(fn);
 create_bmp(t.bmpf);
 dts();
 dts4();
 show_grad();
}

//----------------------------------------------------------------------------

void view_dtvprev(char *fn)
{
 open_dtv(fn);
 create_bmp(t.bmpf);
 dts4();
}

void reset_dtvprev()
{

if (t.dtvstr!="")
 {view_dtvprev(t.dtvstr);}

}

//----------------------------------------------------------------------------

void calcul()
{
float sx,sy,sxy,sx2;
float x1,y1,x2,y2;
int i;

t.tmin=1000;
t.tmax=-1000;
t.tmed=0;
t.iscalc=0;
t.iscalcrect=0;

if (t.xi==t.xf) t.xf++;
if (t.yi==t.yf) t.yf++;
if (t.yf==0) t.yf=1;

if (t.xi-t.xf && t.yi && t.yf)
{
x1=t.xi;
x2=t.xf;
y1=t.yi;
y2=t.yf;

sx=x1+x2;
sy=y1+y2;
sxy=x1*y1+x2*y2;
sx2=x1*x1+x2*x2;

t.pixellen=sqrt( (t.xi-t.xf)*(t.xi-t.xf)+(t.yi-t.yf)*(t.yi-t.yf) );
t.panta=(sxy-(sx*sy)/2)/(sx2-(sx*sx)/2);
t.ordonata=sy/2-(t.panta*sx/2);

if (t.panta==1) {t.panta+=0.0000001;}
if (t.panta==-1) {t.panta+=0.0000001;}

if (x1<x2)
 {if (y1<y2)
   {t.np=floor(sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)));
    t.pt=(float *)malloc((t.np+2)*sizeof(float));
    if (t.panta<1 && t.panta>-1)
        {for(i=0;i<(x2-x1);i++){
         t.pt[i]=get_temp(x1+i,floor(t.panta*(x1+i)+t.ordonata));
         if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
         if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
         t.tmed+=t.pt[i];
        }t.np=i;}

    if (t.panta>=1 || t.panta<=-1)
        {for(i=0;i<(y2-y1);i++){
         t.pt[i]=get_temp(floor((y1+i-t.ordonata)/t.panta),y1+i);
         if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
         if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
         t.tmed+=t.pt[i];
        }t.np=i;}

   }

  if (y1>y2 || y1==y2)
   {t.np=floor(sqrt((x2-x1)*(x2-x1)+(y1-y2)*(y1-y2)));
    t.pt=(float *)malloc((t.np+2)*sizeof(float));
      if (t.panta<1 && t.panta>-1)
        {for(i=0;i<(x2-x1);i++){
         t.pt[i]=get_temp(x1+i,floor(t.panta*(x1+i)+t.ordonata));
         if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
         if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
         t.tmed+=t.pt[i];
        }t.np=i;}

      if (t.panta>=1 || t.panta<=-1)
        {for(i=0;i<(y1-y2);i++){
         t.pt[i]=get_temp(floor((y1-i-t.ordonata)/t.panta),y1-i);
         if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
         if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
         t.tmed+=t.pt[i];
        }t.np=i;}

  }

 }

if (x2<x1)
 {if (y1<y2)
   {t.np=floor(sqrt((x1-x2)*(x1-x2)+(y2-y1)*(y2-y1)));
    t.pt=(float *)malloc((t.np+2)*sizeof(float));

   if (t.panta<1 && t.panta>-1)
    {for (i=0;i<(x1-x2);i++){
     t.pt[i]=get_temp(x1-i,floor(t.panta*(x1-i)+t.ordonata));
     if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
     if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
     t.tmed+=t.pt[i];
    }t.np=i;}

   if (t.panta>=1 || t.panta<=-1)
    {for (i=0;i<(y2-y1);i++){
     t.pt[i]=get_temp(floor((y1+i-t.ordonata)/t.panta),y1+i);
     if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
     if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
     t.tmed+=t.pt[i];
    }t.np=i;}

 }
 if (y1>y2 || y1==y2)
  {t.np=floor(sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
   t.pt=(float *)malloc((t.np+2)*sizeof(float));
   if (t.panta<1 && t.panta>-1)
    {for (i=0;i<(x1-x2);i++){
     t.pt[i]=get_temp(x1-i,floor(t.panta*(x1-i)+t.ordonata));
     if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
     if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
     t.tmed+=t.pt[i];
    }t.np=i;}
   if (t.panta>=1 || t.panta<=-1)
    {for (i=0;i<(y1-y2);i++){
     t.pt[i]=get_temp(floor((y1-i-t.ordonata)/t.panta),y1-i);
     if (t.pt[i]<t.tmin) t.tmin=t.pt[i];
     if (t.pt[i]>t.tmax) t.tmax=t.pt[i];
     t.tmed+=t.pt[i];
     }t.np=i;}

   }
 }

if (t.panta && t.ordonata)
{t.iscalc=1;
 t.tmed=t.tmed/t.np;
}
}

}

//----------------------------------------------------------------------------

void calculrect()
{
unsigned int j,cont=0;
float tttt=0,tmp;
float xip,xfp,yip,yfp;
int i,f;

t.panta=0;t.ordonata=0;
t.tmin=1000.01;
t.tmax=-1000.01;
t.iscalc=0;
t.iscalcrect=0;

xip=t.xi;
xfp=t.xf;
yip=t.yi;
yfp=t.yf;

t.pixelarea=abs((t.xi-t.xf)*(t.yi-t.yf));
if (xip>xfp) {tmp=xip;xip=xfp;xfp=tmp;}
if (yip<yfp) {tmp=yip;yip=yfp;yfp=tmp;}

t.np=(xfp-xip+1)*(yip-yfp+1);
t.pt=(float *)malloc((t.np)*sizeof(float));

for (i=yfp;i<=yip;i++)
   {for (f=xip;f<=xfp;f++)
       {t.pt[cont]=get_temp(f,i);
        cont=cont+1;
       }
   }

for (j=0;j<t.np;j++)
  {if (t.pt[j]<t.tmin) t.tmin=t.pt[j];
   if (t.pt[j]>t.tmax) t.tmax=t.pt[j];
   tttt+=t.pt[j];
  }

t.tmed=tttt/t.np;
t.iscalcrect=1;
free(t.pt);
}

//----------------------------------------------------------------------------

void refresh_calc()
{
if (t.xf || t.yf)
 {if (t.d==1||t.d==2) calcul();
  //if (t.d==3||t.d==4) calculb();
  if (t.d==5) calculrect();
 }
}

//----------------------------------------------------------------------------

void show_lutgraph(int x1,int x2)
{
Form4->Image2->Canvas->Brush->Color = clWindow;
Form4->Image2->Canvas->Pen->Color = clWindow;
Form4->Image2->Canvas->Rectangle(0,0,120,120);
Form4->Image2->Canvas->Pen->Color = clBlack;
Form4->Image2->Canvas->MoveTo(10,5);Form4->Image2->Canvas->LineTo(10,70);
Form4->Image2->Canvas->MoveTo(8,69);Form4->Image2->Canvas->LineTo(76,69);
Form4->Image2->Canvas->Pen->Color = clGray;
Form4->Image2->Canvas->Pen->Style=psDot;
Form4->Image2->Canvas->MoveTo(8,5);Form4->Image2->Canvas->LineTo(76,5);
Form4->Image2->Canvas->MoveTo(8,37);Form4->Image2->Canvas->LineTo(76,37);
Form4->Image2->Canvas->MoveTo(42,5);Form4->Image2->Canvas->LineTo(42,70);
Form4->Image2->Canvas->MoveTo(74,5);Form4->Image2->Canvas->LineTo(74,70);
Form4->Image2->Canvas->Pen->Style=psSolid;
Form4->Image2->Canvas->Pen->Color = clBlue;
Form4->Image2->Canvas->Pen->Width = 2;
Form4->Image2->Canvas->MoveTo(10,70);Form4->Image2->Canvas->LineTo(floor(x1/4)+10,70);//x1/4
Form4->Image2->Canvas->LineTo(floor(x2/4)+10,5);
Form4->Image2->Canvas->LineTo(74,5);
Form4->Image2->Canvas->Pen->Width = 1;
}

void show_graph()
{
float unity;
int d1,d2;
unsigned i;

reset_graph();
Form3->Image2->Canvas->Pen->Color = clBlack;

   if (Form3->CheckBox3->Checked==0) {d1=10*(floor(t.tmin/10));Form3->MaskEdit8->Text=d1;}
    else d1=floor(atof(Form3->MaskEdit8->Text.c_str()));
   if (Form3->CheckBox4->Checked==0) {d2=10*(ceil(t.tmax/10));Form3->MaskEdit9->Text=d2;}
    else d2=floor(atof(Form3->MaskEdit9->Text.c_str()));
   unity=(float)155/(d2-d1);
   Form3->Image2->Canvas->TextOut(2,175,d1);
   Form3->Image2->Canvas->TextOut(2,98,d1+floor((d2-d1)/2));
   Form3->Image2->Canvas->TextOut(2,59,(float) (d1+(d2-d1)*0.75));
   Form3->Image2->Canvas->TextOut(2,136,(float) (d1+(d2-d1)*0.25));
   Form3->Image2->Canvas->TextOut(2,20,d2);
   Form3->Image2->Canvas->Pen->Color = clBlue;
if (t.np)
   {Form3->Image2->Canvas->MoveTo(30,175-floor(unity*(t.pt[0]-d1)));
    for (i=0;i<t.np;i++)
       {Form3->Image2->Canvas->LineTo(30+i,175-floor(unity*(t.pt[i]-d1)));}
    free(t.pt);
   }
}

//----------------------------------------------------------------------------

void reset_graph()
{
Form3->Image2->Canvas->Brush->Color = clWindow;
Form3->Image2->Canvas->Pen->Color = clWindow;
Form3->Image2->Canvas->Rectangle(0,0,300,300);
Form3->Image2->Canvas->Pen->Color = clBlack;
Form3->Image2->Canvas->MoveTo(20,18);Form3->Image2->Canvas->LineTo(20,180);
Form3->Image2->Canvas->Pen->Color = clGray;
Form3->Image2->Canvas->Pen->Style=psDot;
Form3->Image2->Canvas->MoveTo(16,20);Form3->Image2->Canvas->LineTo(295,20);
Form3->Image2->Canvas->MoveTo(16,59);Form3->Image2->Canvas->LineTo(295,59);
Form3->Image2->Canvas->MoveTo(16,98);Form3->Image2->Canvas->LineTo(295,98);
Form3->Image2->Canvas->MoveTo(16,136);Form3->Image2->Canvas->LineTo(295,136);
Form3->Image2->Canvas->Pen->Style=psSolid;
Form3->Image2->Canvas->Pen->Color = clBlack;
Form3->Image2->Canvas->MoveTo(15,175);Form3->Image2->Canvas->LineTo(295,175);
Form3->Image2->Canvas->TextOut(20,3,"temp (°C)");
Form3->Image2->Canvas->TextOut(250,180,"pixels");
}

//----------------------------------------------------------------------------

void show_hist()
{
int i;
char info[100];

Form3->Image1->Canvas->Brush->Color = clWindow;
Form3->Image1->Canvas->Pen->Color = clWindow;
Form3->Image1->Canvas->Rectangle(0,0,600,300);

Form3->Image1->Canvas->Pen->Color = clGray;
Form3->Image1->Canvas->Pen->Style=psDot;

Form3->Image1->Canvas->MoveTo(8,11);Form3->Image1->Canvas->LineTo(524,11);
Form3->Image1->Canvas->MoveTo(8,55);Form3->Image1->Canvas->LineTo(524,55);
Form3->Image1->Canvas->MoveTo(8,99);Form3->Image1->Canvas->LineTo(524,99);
Form3->Image1->Canvas->MoveTo(8,143);Form3->Image1->Canvas->LineTo(524,143);

for (i=0;i<=16;i++)
 {Form3->Image1->Canvas->MoveTo(32*i+10,9);Form3->Image1->Canvas->LineTo(32*i+10,187);
  Form3->Image1->Canvas->Font->Color = clBlue;
  Form3->Image1->Canvas->TextOut(32*i+4,188,i*16);
  Form3->Image1->Canvas->Font->Color = clRed;
  Form3->Image1->Canvas->TextOut(32*i+4,0,floor((i*16*t.th.tsc[0]+t.th.tsc[1])*10)/10);
 }
Form3->Image1->Canvas->Font->Color = clBlack;

Form3->Image1->Canvas->Pen->Style=psSolid;
Form3->Image1->Canvas->Pen->Color = clBlack;
//Form3->Image1->Canvas->MoveTo(10,9);Form3->Image1->Canvas->LineTo(10,190);
Form3->Image1->Canvas->MoveTo(8,187);Form3->Image1->Canvas->LineTo(524,187);
//Form3->Image1->Canvas->TextOut(20,3,"histogram");

Form3->Image1->Canvas->Pen->Color = clBlue;

for (i=0;i<256;i++)
 {Form3->Image1->Canvas->MoveTo(2*i+11,187);
  Form3->Image1->Canvas->LineTo(2*i+11,floor(-(176/t.histmax)*t.hist[i])+187);
 }

 sprintf(info,"hmax=%.01f\%",t.histmax*100);
 Form3->Image1->Canvas->TextOut(475,12,info);
}

//----------------------------------------------------------------------------

void refresh_image()
{
TPoint Origin,MovePt;

Origin = Point(t.xi,t.yi);
MovePt = Point(t.xf,t.yf);

Form1->Image1->Canvas->Pen->Width = 1;
Form1->Image1->Canvas->Pen->Mode = pmNotXor;
DrawShape(Origin, MovePt, pmNotXor);
}

//----------------------------------------------------------------------------

void show_info()
{
Form3->Image2->Canvas->Brush->Color = clWindow;
Form3->Image2->Canvas->Pen->Color = clWindow;
Form3->Image2->Canvas->Rectangle(300,0,1200,300);
Form3->Image2->Canvas->Pen->Color = clBlack;

if (t.isloaded)
  {Form3->Image2->Canvas->TextOut(200,3,"file");
   Form3->Image2->Canvas->TextOut(220,3,t.dtvstr);

   if (t.iscalc)
    {Form3->Image2->Canvas->TextOut(320,100,"slope");
     Form3->Image2->Canvas->TextOut(320,115,"y(0)");
     Form3->Image2->Canvas->TextOut(320,130,"tmin (°C)");
     Form3->Image2->Canvas->TextOut(320,145,"tmax (°C)");
     Form3->Image2->Canvas->TextOut(320,160,"tmed (°C)");
     if (Form4->ComboBox2->Text=="mm") {Form3->Image2->Canvas->TextOut(320,175,"lenght (mm)");}
     if (Form4->ComboBox2->Text=="cm") {Form3->Image2->Canvas->TextOut(320,175,"lenght (cm)");}
     if (Form4->ComboBox2->Text=="m") {Form3->Image2->Canvas->TextOut(320,175,"lenght (m)");}
     if (Form4->ComboBox2->Text=="inch") {Form3->Image2->Canvas->TextOut(320,175,"lenght (inch)");}
     Form3->Image2->Canvas->TextOut(380,100,floor(t.panta*1000)/1000);
     Form3->Image2->Canvas->TextOut(380,115,floor(t.ordonata*1000)/1000);
     Form3->Image2->Canvas->TextOut(380,130,floor(t.tmin*100)/100);
     Form3->Image2->Canvas->TextOut(380,145,floor(t.tmax*100)/100);
     Form3->Image2->Canvas->TextOut(380,160,floor(t.tmed*100)/100);
     if (Form4->ComboBox2->Text=="mm") {Form3->Image2->Canvas->TextOut(380,175,floor(t.mmpp*t.pixellen*100)/100);}
     if (Form4->ComboBox2->Text=="cm") {Form3->Image2->Canvas->TextOut(380,175,floor(t.mmpp*t.pixellen*100)/100/10);}
     if (Form4->ComboBox2->Text=="m") {Form3->Image2->Canvas->TextOut(380,175,floor(t.mmpp*t.pixellen*100)/100/1000);}
     if (Form4->ComboBox2->Text=="inch") {Form3->Image2->Canvas->TextOut(380,175,floor(t.mmpp*t.pixellen*100)/100/25.4);}

     //Form1->sv2->Enabled=1;
     //Form1->sv->Enabled=1;
    }
   if (t.iscalcrect)
    {Form3->Image2->Canvas->TextOut(320,100,"tmin (°C)");
     Form3->Image2->Canvas->TextOut(320,115,"tmax (°C)");
     Form3->Image2->Canvas->TextOut(320,130,"tmed (°C)");
     if (Form4->ComboBox2->Text=="mm") {Form3->Image2->Canvas->TextOut(320,145,"area (mm²)");}
     if (Form4->ComboBox2->Text=="cm") {Form3->Image2->Canvas->TextOut(320,145,"area (cm²)");}
     if (Form4->ComboBox2->Text=="m") {Form3->Image2->Canvas->TextOut(320,145,"area (m²)");}
     Form3->Image2->Canvas->TextOut(380,100,floor(t.tmin*100)/100);
     Form3->Image2->Canvas->TextOut(380,115,floor(t.tmax*100)/100);
     Form3->Image2->Canvas->TextOut(380,130,floor(t.tmed*100)/100);
     if (Form4->ComboBox2->Text=="mm") {Form3->Image2->Canvas->TextOut(380,145,floor(t.mmpp*t.mmpp*t.pixelarea*100)/100);}
     if (Form4->ComboBox2->Text=="cm") {Form3->Image2->Canvas->TextOut(380,145,floor(t.mmpp*t.mmpp*t.pixelarea/100*100)/100);}
     if (Form4->ComboBox2->Text=="m") {Form3->Image2->Canvas->TextOut(380,145,floor(t.mmpp*t.mmpp*t.pixelarea/1000000*100)/100);}
    }
   if (t.xi || t.yi)
    {Form3->Image2->Canvas->TextOut(320,40,"x1");
     Form3->Image2->Canvas->TextOut(320,55,"y1");
     Form3->Image2->Canvas->TextOut(380,40,t.xi);
     Form3->Image2->Canvas->TextOut(380,55,t.yi);
    }
   if (t.xf || t.yf)
    {Form3->Image2->Canvas->TextOut(320,70,"x2");
     Form3->Image2->Canvas->TextOut(320,85,"y2");
     Form3->Image2->Canvas->TextOut(380,70,t.xf);
     Form3->Image2->Canvas->TextOut(380,85,t.yf);
    }
  }
}

//----------------------------------------------------------------------------

float get_temp(unsigned x, unsigned y)
{
return(t.th.atf[y*256+x+110]*t.th.tsc[0]+t.th.tsc[1]);
}

//----------------------------------------------------------------------------

unsigned char get_pixel_temp(unsigned x, unsigned y)
{
return(t.th.atf[y*256+x+110]);
}

//----------------------------------------------------------------------------

void dts()
{
Form1->Image1->Picture->LoadFromFile(t.bmpf);
}

//----------------------------------------------------------------------------

void dts4()
{
Form4->Image1->Picture->LoadFromFile(t.bmpf);
}

//----------------------------------------------------------------------------

void show_grad()

{
int f,i,k;

 Form1->Image3->Canvas->Pen->Width = 1;
 Form1->Image3->Canvas->Pen->Color = clWhite;
 Form1->Image3->Canvas->Pen->Mode = pmCopy;
 Form1->Image3->Canvas->Brush->Color = clWhite;
 Form1->Image3->Canvas->Rectangle(14,0,60,2000);
 Form1->Image3->Canvas->Pen->Color = clBlack;

//if (Form1->Image3->Canvas->Pixels[5][55]==clWhite)
if (must_change_grad)
{for(f=0;f<floor(zoom*256);f++)
   {for(i=0;i<14;i++)
       {k=floor(f/zoom);
        Form1->Image3->Canvas->Pixels[i][floor(256*zoom)-1-f]=TColor (4*(vpl_data[vpl_current][k*3]+256*vpl_data[vpl_current][k*3+1]+65536*vpl_data[vpl_current][k*3+2]));
       }
   }
}

must_change_grad=0;

 for (f=1;f<floor(16*zoom)-1;f++)
  {Form1->Image3->Canvas->MoveTo(12, f*16);
   Form1->Image3->Canvas->LineTo(16, f*16);
   Form1->Image3->Canvas->TextOut(17,f*16-6,floor((256*t.th.tsc[0]+t.th.tsc[1]-(f*16*t.th.tsc[0]/zoom))*10)/10);
  }

   Form1->Image3->Canvas->Font->Color = clRed;

   Form1->Image3->Canvas->TextOut(17,-1,floor((256*t.th.tsc[0]+t.th.tsc[1]-(0*8*t.th.tsc[0]))*10)/10);
   Form1->Image3->Canvas->TextOut(17,floor(256*zoom-27),floor((256*t.th.tsc[0]+t.th.tsc[1]-(32*8*t.th.tsc[0]))*10)/10);

   Form1->Image3->Canvas->Font->Color = clBlack;
}

//----------------------------------------------------------------------------

void get_info(char *fn)
{
float min,max,med,temp,tempsss;
int i;
char filenam[100];
char buff[200];

 open_dtv(fn);
 min=1000.01;
 max=-1000.01;
 tempsss=0;

 for(i=110;i<63598;i++)
   {temp=t.th.atf[i]*t.th.tsc[0]+t.th.tsc[1];
    if (temp>max) max=temp;
    if (temp<min) min=temp;
    tempsss+=temp;
   }

med=tempsss/63487;
fnsplit(fn,NULL,NULL,filenam,NULL);

sprintf(buff,"%08s",filenam);
Form3->StringGrid1->Cells[0][sg_current_row] = buff;
sprintf(buff,"%+06.2f",t.th.tsc[1]);
Form3->StringGrid1->Cells[1][sg_current_row] = buff;
sprintf(buff,"%+06.2f",256*t.th.tsc[0]+t.th.tsc[1]);
Form3->StringGrid1->Cells[2][sg_current_row] = buff;
sprintf(buff,"%+06.2f",t.th.tsc[0]);
Form3->StringGrid1->Cells[3][sg_current_row] = buff;
sprintf(buff,"%+06.2f",min);
Form3->StringGrid1->Cells[4][sg_current_row] = buff;
sprintf(buff,"%+06.2f",max);
Form3->StringGrid1->Cells[5][sg_current_row] = buff;
sprintf(buff,"%+06.2f",med);
Form3->StringGrid1->Cells[6][sg_current_row] = buff;
sprintf(buff,"%+08s",t.th.inform);
Form3->StringGrid1->Cells[7][sg_current_row] = buff;

}

//----------------------------------------------------------------------------

void DrawShape(TPoint TopLeft, TPoint BottomRight, TPenMode AMode)
   {
    Form1->Image1->Canvas->Pen->Mode = AMode;

    switch (DrawingTool){
           case dtLine : {
                Form1->Image1->Canvas->MoveTo(TopLeft.x, TopLeft.y);
                Form1->Image1->Canvas->LineTo(BottomRight.x, BottomRight.y);
                break;
           }
           case dtRectangle : {
                Form1->Image1->Canvas->Rectangle(TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y);
                break;
           }
           case dtEllipse : {
                Form1->Image1->Canvas->Ellipse(TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y);
                break;
           }
    }
}

//----------------------------------------------------------------------------

void DrawShapeimg3(TPoint TopLeft, TPoint BottomRight, TPenMode AMode)
   {
    Form1->Image3->Canvas->Pen->Mode = AMode;

    Form1->Image3->Canvas->MoveTo(TopLeft.x, TopLeft.y);
    Form1->Image3->Canvas->LineTo(BottomRight.x, BottomRight.y);
    }

//----------------------------------------------------------------------------


void lut_prev(int n)
{
static float tmin,tmax;
static int r,ci;
float nmin,nmax,tlmin,tlmax;
AnsiString fn;
int once=0;
static int once2=0;
char oldname[1000];
char newname[1000];

if (once2==0)
{tmax=256*t.th.tsc[0]+t.th.tsc[1];
 tmin=t.th.tsc[1];
}

once2=1;

nmin=atof(Form4->MaskEdit2->Text.c_str());
nmax=atof(Form4->MaskEdit1->Text.c_str());

if (nmin<tmin) {nmin=tmin+0.001; Form4->MaskEdit2->Text=floor(tmin*100)/100;}
if (nmax>tmax) {nmax=tmax-0.001; Form4->MaskEdit1->Text=floor(tmax*100)/100;}

if (n==0)
      {
       Form4->MaskEdit2->Text=floor(tmin*100)/100;
       Form4->MaskEdit1->Text=floor(tmax*100)/100;
       tlmin=0;tlmax=256;
       show_lutgraph(tlmin,tlmax);
      }

if (n==1)
      {
       nmin=atof(Form4->MaskEdit2->Text.c_str());
       nmax=atof(Form4->MaskEdit1->Text.c_str());
       tlmin=256*(nmin-tmin)/(tmax-tmin);
       tlmax=256*(nmax-tmin)/(tmax-tmin);
       show_lutgraph(tlmin,tlmax);
      }

if (n==2)
{
ci=0;
r=0;

tlmin=256*(nmin-tmin)/(tmax-tmin);
tlmax=256*(nmax-tmin)/(tmax-tmin);
show_lutgraph(tlmin,tlmax);

if ((nmin==0) || (nmax==0)) {Form3->Memo1->Lines->Add("> err 002 -  not sufficient data entered");}
else
if(Form1->FileListBox1->SelCount)
 {
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"prev","dtv",newname);
      mklut(oldname,newname,nmin,nmax);
      //Form1->FileListBox1->Selected[ci]=0;
      once++;
     }
    ci++;
  } while (once==0); //(Form1->FileListBox1->SelCount>0);

 view_dtvprev(newname);
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");
}

if (n==3)
{
//view_log_b->Click();

ci=0;
r=0;

tlmin=256*(nmin-tmin)/(tmax-tmin);
tlmax=256*(nmax-tmin)/(tmax-tmin);
show_lutgraph(tlmin,tlmax);

if ((nmin==0) || (nmax==0)) {Form3->Memo1->Lines->Add("> err 002 -  not sufficient data entered");}
else
if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_","dtv",newname);
      mklut(oldname,newname,nmin,nmax);
      Form1->FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (Form1->FileListBox1->SelCount>0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 view_dtv(newname);
 Form1->FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");
}

if (n==4)
{once2=0;}

}

int round(float number)
{
float test;

 test = number-floor(number);

 if (test>0.5) {return ceil(number);}
 return floor(number);
}

int sort_function( const void *a, const void *b)
{
return(*(float*)a-*(float*)b);
}

