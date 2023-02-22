//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop
#include <dir.h>
#include <time.h>
#include <sys\timeb.h>
#include <stdio.h>
#include <dos.h>
#include <math.h>
#include <vcl/clipbrd.hpp>
#include "form.h"

#include <stdlib.h>
#include <system.hpp>

#include "general.h"
#include "processing.h"
#include "analysis.h"
#include "palette.h"
#include "koh.h"
#include "version.h"

#include "unit2.h"
#include "unit3.h"
#include "unit4.h"

//---------------------------------------------------------------------------
#pragma link "Grids"
#pragma nopackwarning
#pragma resource "*.dfm"
TForm1 *Form1;

extern struct termograma t;
extern struct bmph8bit bmph;

extern int sg_current_row;

extern TPoint Origin, MovePt;
extern TPoint Origin2, MovePt2;

extern TDrawingTool DrawingTool;
extern AnsiString CurrentFile;
extern int Xref,Yref;

extern unsigned char must_change_grad;
extern unsigned char shrink;
extern int sg_current_row;

extern float zoom;

extern int fheight;
extern bool Drawing,moving;

extern unsigned char vpl_data[12][768];
extern unsigned char vpl_current;

//extern class mask m;
extern struct mask_struct msk;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}

void __fastcall viewimg_b();
void __fastcall ch_zoom(float img_zoom, int options);

//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
 char version_info[200];
 char *name2;

 name2 = tempnam("\\tmp","therm");
 t.bmpf=(char *)malloc(( strlen(name2)+10 )*sizeof(char));
 sprintf(t.bmpf,"%s.bmp",name2);
 free(name2);

 sprintf(version_info,"ThPP v%d.%d.%d",major_version,minor_version,release);
 Form1->Caption=version_info;

 t.d=1;
 t.c=0;
 t.iscalc=0;
 t.isloaded=0;
 t.iscalcrect=0;
 fheight=Font->Height;
 ch_zoom(2,4);

 reset_mask(10);
}

//---------------------------------------------------------------------------


void __fastcall viewimg_b()
{
if (Form1->FileListBox1->SelCount)
 {
//  t.dtvstr=(char* )malloc((Form1->FileListBox1->FileName.Length() + 1)*sizeof(char));
//  m.msk.origfile = (char *)malloc((Form1->FileListBox1->FileName.Length() + 1)*sizeof(char));
  sprintf(t.dtvstr, "%s", Form1->FileListBox1->FileName.c_str() );
//  sprintf( msk.origfile, "%s", Form1->FileListBox1->FileName.c_str() );   
  view_dtv(t.dtvstr);
 }
else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");
}

void __fastcall ch_zoom(float img_zoom, int options)
{
int size_x,size_y;
bool show_gr=0,from_resize=0,from_user=0;
char buff[20];

if (options>3) {options-=4;from_user=1;}
if (options>1) {options-=2;from_resize=1;}
if (options>0) {show_gr=1;}

if (img_zoom<0.2) {img_zoom=0.2;}
size_x=t.th.nst;
size_y=t.th.nstv;
if (size_x>2000 || size_x<10) {size_x=256;size_y=248;}

zoom=img_zoom;

Form1->Panel2->Width=size_x*img_zoom;
Form1->Panel2->Height=size_y*img_zoom;
Form1->Panel6->Left=size_x*img_zoom;
Form1->Panel6->Height=size_y*img_zoom;
Form1->ProgressBar1->Top=Form1->Height-71;
Origin2=Point(0,0);
MovePt2 = Point(0,0);

if (show_gr) {must_change_grad=1;show_grad();}
if (shrink)
            {if (from_resize) Form1->Width=size_x*img_zoom+Form1->Panel6->Width;
             if (from_user) Form1->ClientHeight=size_y*img_zoom+Form1->Panel8->Height;
            }

sprintf(buff,"zoom: %.02fx",floor(img_zoom*100.0)/100);
Form1->Label1->Caption=buff;
zoom=img_zoom;
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
//char drive[MAXDRIVE];
//char dir[MAXDIR];
//char file[MAXFILE];
//char ext[MAXEXT];
//char newname[100];
time_t tim;
struct tm *area;
tim = time(NULL);
//FILE *ttt;
char buff[200];

remove(t.bmpf);
close_dtv();

area = localtime(&tim);
sprintf(buff,"> normal exit at %s",asctime(area));
Form3->Memo1->Lines->Add(buff);

//here
//fnsplit(prog_path,drive,dir,file,ext);
//fnmerge(newname,drive,dir,"therm","log");

//ttt=fopen(newname,"a+b");
//fwrite(Form3->Memo1->Text.c_str(), strlen(Form3->Memo1->Text.c_str()), 1, ttt);//*sizeof(Memo1->Lines)
//fclose(ttt);

free(t.bmpf);
}

//---------------------------------------------------------------------------


void __fastcall TForm1::SpeedButton1Click(TObject *Sender)
{
shrink=1;
Form1->Panel1->Visible=0;
Form1->Width-=Form1->Panel1->Width;
}

//---------------------------------------------------------------------------

void __fastcall TForm1::MaskEdit3Change(TObject *Sender)
{
DrawingTool = dtLine;

if(t.isloaded)
{
view_graph_b->Click();
if (t.d==5) {dts();refresh_image();}
t.d=2;
if (t.xf || t.yf)
   {refresh_calc();
    show_graph();
    show_info();
   }
}t.d=2;
}

//---------------------------------------------------------------------------

void __fastcall TForm1::copytoclipboard1Click(TObject *Sender)
{
  Clipboard()->Assign(Image1->Picture);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::saveas1Click(TObject *Sender)
{
  if (SaveDialog1->Execute()){
    CurrentFile = SaveDialog1->FileName;
    Image1->Picture->SaveToFile(CurrentFile);
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::info1Click(TObject *Sender)
{
 Form3->Memo1->Lines->Add("> not fully implemented");
}
//---------------------------------------------------------------------------





void __fastcall TForm1::BitBtn8Click(TObject *Sender)
{
static int r,ci;
AnsiString fn;
char oldname[1000];
char newname[1000];

view_log_b->Click();

ci=0;
r=0;

if(FileListBox1->SelCount)
 {
 ProgressBar1->Max=floor(FileListBox1->SelCount);
 do
  {   if (FileListBox1->Selected[ci])
     {
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);
      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_N","dtv",newname);
      mk_normal(oldname,newname);

      FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (FileListBox1->SelCount>0);

 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 viewimg_b();

 FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");

}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TForm1::Image1MouseMove(TObject *Sender, TShiftState Shift,
	int X, int Y)
{
float Ygr;
int size_x, size_y, current_x, current_y;

size_x=t.th.nst*zoom;
size_y=t.th.nstv*zoom;
current_x=X;
current_y=Y;

if (current_x >= size_x) {current_x=size_x-1;}
if (current_y >= size_y) {current_y=size_y-1;}
if (current_x <= 0) {current_x=1;}
if (current_y <= 0) {current_y=1;}

if ((Drawing) && (t.isloaded) && !(moving) )
{
 Image1->Canvas->MoveTo(current_x/zoom, current_y/zoom);
 DrawShape(Origin, MovePt, pmNotXor);
 MovePt = Point(current_x/zoom, current_y/zoom);
 DrawShape(Origin, MovePt, pmNotXor);
 show_info();
 t.xf=current_x/zoom;
 t.yf=current_y/zoom;
}

if (t.isloaded)
 {Form3->Image2->Canvas->TextOut(20,3,"temp (°C)               ");
  Form3->Image2->Canvas->Font->Color=clRed;
  Form3->Image2->Canvas->TextOut(80,3,floor(get_temp(floor(current_x/zoom),floor(current_y/zoom))*100)/100);
  Form3->Image2->Canvas->Font->Color=clBlack;
  Form1->Image3->Canvas->Pen->Mode = pmNotXor;
  Form1->Image3->Canvas->Pen->Width = 2;
  Ygr=(zoom*(256*t.th.tsc[0]+t.th.tsc[1]-get_temp(floor(current_x/zoom),floor(current_y/zoom))))/t.th.tsc[0];
  Image3->Canvas->MoveTo(0, floor(Ygr));
  DrawShapeimg3(Origin2, MovePt2, pmNotXor);
  Origin2=Point(0,floor(Ygr));
  MovePt2 = Point(13, Ygr);
  DrawShapeimg3(Origin2, MovePt2, pmNotXor);
 }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Image1MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
moving=0;

if (Button==mbLeft && t.isloaded)
{if (t.isloaded) {dts();}

Image1->Canvas->Pen->Width = 1;
Image1->Canvas->Pen->Mode = pmNotXor;
Drawing = 1;
t.xi=X/zoom;
t.yi=Y/zoom;
t.iscalc=0;
t.iscalcrect=0;
t.xf=0;
t.yf=0;
reset_graph();
show_info();
view_graph_b->Click();
Image1->Canvas->MoveTo(X/zoom,Y/zoom);
Origin = Point(X/zoom, Y/zoom);
MovePt = Origin;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Image1MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
Drawing = 0;
moving = 0;
if (t.xf || t.yf)
 {if ((t.d == 1) || (t.d == 2)) {calcul();}
  if (t.d == 5) {calculrect();}
  if ((t.d == 3) || (t.d == 4)) {calcul();} //???
  if (t.d!=5) {show_graph();}
  show_info();
 }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::copytoclipboard3Click(TObject *Sender)
{
  Clipboard()->Assign(Image3->Picture);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::saveas3Click(TObject *Sender)
{
  if (SaveDialog1->Execute()){
    CurrentFile = SaveDialog1->FileName;
    Image3->Picture->SaveToFile(CurrentFile);
   }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Exit1Click(TObject *Sender)
{
Close();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::bmp1Click(TObject *Sender)
{
static int r,ci;
AnsiString fn;
char oldname[1000];
char newname[1000];

view_log_b->Click();

ci=0;
r=0;
if(FileListBox1->SelCount)
{
ProgressBar1->Max=floor(FileListBox1->SelCount);

do
{   if (FileListBox1->Selected[ci])
     {//FileListBox1->TopIndex=ci-FileListBox1->Height/FileListBox1->ItemHeight;
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);
      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"","bmp",newname);
      mkbmp(oldname,newname);
      FileListBox1->Selected[ci]=0;
     }
   ci++;
 } while (FileListBox1->SelCount>0);

 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 viewimg_b();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");

}
//---------------------------------------------------------------------------

void __fastcall TForm1::getinfo1Click(TObject *Sender)
{
static int r,ci;
AnsiString fn;
char curpathp[200];
char buff[200];

//Form1->Image1->Canvas->Brush->Color = clBtnFace;
//Form1->Image1->Canvas->Pen->Color = clBtnFace;
//Form1->Image1->Canvas->Rectangle(0,0,300,300);
view_info_b->Click();

ci=0;
r=0;

if(FileListBox1->SelCount)
{
getcwd(curpathp, 249);
sprintf(buff,"> current path is %s",curpathp);
Form3->Memo1->Lines->Add(buff);

ProgressBar1->Max=floor(FileListBox1->SelCount);

do
{   if (FileListBox1->Selected[ci])
     {
     //FileListBox1->TopIndex=ci-FileListBox1->Height/FileListBox1->ItemHeight;
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);
      sg_current_row++;
      get_info(fn.c_str());
      FileListBox1->Selected[ci]=0;
     }
   ci++;
 } while (FileListBox1->SelCount>0);
 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 viewimg_b();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");
}

//---------------------------------------------------------------------------

void __fastcall TForm1::lookuptable1Click(TObject *Sender)
{
 if (t.isloaded)
 {
 view_dtvprev(t.dtvstr);
 lut_prev(4);
 lut_prev(0);
 Form4->Panel1->Visible=0;
 Form4->Panel5->Visible=1;
 Form4->Panel3->Visible=0;
 Form4->Panel6->Visible=0;
 Form4->Panel7->Visible=0;
 Form4->Panel8->Visible=0;
 Form4->Visible=1;
 Form1->Enabled=0;
 }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::savegaphimage1Click(TObject *Sender)
{
char buff[200];
char curpathp[1000];

Form3->Memo1->Visible=0;Form3->Image2->Visible=1;
getcwd(curpathp, 999);SaveDialog2->InitialDir=curpathp;
  if (SaveDialog1->Execute()){
    CurrentFile = SaveDialog1->FileName;
    Form3->Image2->Picture->SaveToFile(CurrentFile);
    sprintf(buff,"> file %s created",SaveDialog1->FileName.c_str());
    Form3->Memo1->Lines->Add(buff);
   }

}

//---------------------------------------------------------------------------

void __fastcall TForm1::savegraphvalues1Click(TObject *Sender)
{
unsigned i;
FILE *ttt;
char buff[200];
char curpathp[1000];


if (strlen(SaveDialog2->FileName.c_str())>5)
 {if (t.xi || t.xf)
    {ttt=fopen(SaveDialog2->FileName.c_str(),"a+b");
     for (i=0;i<t.np;i++){fprintf(ttt,"%06.2f \n",t.pt[i]);}
     fclose(ttt);
     sprintf(buff,"> file %s updated",SaveDialog2->FileName.c_str());
     Form3->Memo1->Lines->Add(buff);
    }  else{Form3->Memo1->Lines->Add("> err 003 -  no values to save");}
  }
 else
{getcwd(curpathp, 999);SaveDialog2->InitialDir=curpathp;
if (SaveDialog2->Execute())
 {
 if (t.xi || t.xf)
    {ttt=fopen(SaveDialog2->FileName.c_str(),"a+b");
     for (i=0;i<t.np;i++){fprintf(ttt,"%06.2f \n",t.pt[i]);}
     fclose(ttt);
     sprintf(buff,"> file %s created",SaveDialog2->FileName.c_str());
     Form3->Memo1->Lines->Add(buff);
    }    else{Form3->Memo1->Lines->Add("> err 003 -  no values to save");}
 }
 }

}

//---------------------------------------------------------------------------

void __fastcall TForm1::saveall1Click(TObject *Sender)
{
char buff[200];
char oldname[1000];
char newname[1000];
char curpathp[1000];

Form3->Memo1->Visible=0;Form3->Image2->Visible=1;
getcwd(curpathp, 999);SaveDialog2->InitialDir=curpathp;
  if (SaveDialog1->Execute()){
    CurrentFile = SaveDialog1->FileName;
    Image1->Picture->SaveToFile(CurrentFile);
    sprintf(oldname,"%s",CurrentFile.c_str());
    get_name(oldname,"_temperaturi_","bmp",newname);
    Image3->Picture->SaveToFile(newname);  //getname(fn.c_str(),"_N","dtv")
    get_name(oldname,"_grafic_","bmp",newname);
    Form3->Image2->Picture->SaveToFile(newname);
    sprintf(buff,"> file %s created",SaveDialog1->FileName.c_str());
    Form3->Memo1->Lines->Add(buff);
   }

}

//---------------------------------------------------------------------------

void __fastcall TForm1::view_graph_bClick(TObject *Sender)
{
Form3->Image1->Visible=0;
Form3->Memo1->Visible=0;
Form3->Image2->Visible=1;
Form3->StringGrid1->Visible=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::view_log_bClick(TObject *Sender)
{
Form3->Image1->Visible=0;
Form3->Memo1->Visible=1;
Form3->Image2->Visible=0;
Form3->StringGrid1->Visible=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::about1Click(TObject *Sender)
{
 TForm2 *Form2;
 Form2 = new TForm2(Application);
 Form2->ShowModal();
 delete Form2;

}

//---------------------------------------------------------------------------

void __fastcall TForm1::Nagao1Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
char oldname[1000];
char newname[1000];

r=0;
ci=0;

if(FileListBox1->SelCount)
 {
 ProgressBar1->Max=floor(FileListBox1->SelCount);
 do
  {   if (FileListBox1->Selected[ci])
     {
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_NAG","dtv",newname);
      mk_Nagao(oldname,newname);
      FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (FileListBox1->SelCount>0);

 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 view_dtv(newname);
 FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");

}
//---------------------------------------------------------------------------


void __fastcall TForm1::gradient1Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
int n_dt;
char oldname[1000];
char newname[1000];

r=0;
ci=0;
n_dt=8;

if(FileListBox1->SelCount)
 {
 ProgressBar1->Max=floor(FileListBox1->SelCount);
 do
  {   if (FileListBox1->Selected[ci])
     {
     //FileListBox1->TopIndex=ci-FileListBox1->Height/FileListBox1->ItemHeight;
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_TG","dtv",newname);
      mk_th_gradient(oldname,newname,n_dt,0);
      FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (FileListBox1->SelCount>0);

 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 viewimg_b();

 FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");

}
//---------------------------------------------------------------------------




void __fastcall TForm1::normal1Click(TObject *Sender)
{
DrawingTool = dtLine;
///normal_rb->Checked=1;

if (t.isloaded)
{
view_graph_b->Click();
if (t.d==5) {dts();refresh_image();}
t.d=1;
if (t.xf || t.yf)
   {refresh_calc();
    show_graph();
    show_info();
   }
}t.d=1;
    
}
//---------------------------------------------------------------------------

void __fastcall TForm1::areaevaluation1Click(TObject *Sender)
{

DrawingTool = dtRectangle;

if (t.isloaded)
{
view_graph_b->Click();
if (t.d!=5) {dts();refresh_image();}
t.d=5;
if (t.xf || t.yf)
   {refresh_calc();
    show_info();
   }
reset_graph();
}t.d=5;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::aliase1Click(TObject *Sender)
{
DrawingTool = dtLine;
///aa_rb->Checked=1;
///sscanf(MaskEdit3->Text.c_str(),"%d", &aa);

if (t.isloaded)
{
view_graph_b->Click();
if (t.d==5) {dts();refresh_image();}
t.d=2;
if (t.xf || t.yf)
   {refresh_calc();
    show_graph();
    show_info();
   }

  }t.d=2;
}
//---------------------------------------------------------------------------




void __fastcall TForm1::default1Click(TObject *Sender)
{
ch_pal(0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::color1Click(TObject *Sender)
{
ch_pal(1);
}


//---------------------------------------------------------------------------

void __fastcall TForm1::grey1Click(TObject *Sender)
{
ch_pal(2);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::hmetal1Click(TObject *Sender)
{
ch_pal(3);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::hmetal11Click(TObject *Sender)
{
ch_pal(4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::hmetal2(TObject *Sender)
{
ch_pal(5);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::hotblue1Click(TObject *Sender)
{
ch_pal(6);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::hotblue21Click(TObject *Sender)
{
ch_pal(7);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::pertrue1Click(TObject *Sender)
{
ch_pal(8);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::pericolor1Click(TObject *Sender)
{
ch_pal(9);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::rainbow1Click(TObject *Sender)
{
ch_pal(10);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::external1Click(TObject *Sender)
{
ch_pal(11);
}
//---------------------------------------------------------------------------



void __fastcall TForm1::savevaluessamefile1Click(TObject *Sender)
{
unsigned i;
FILE *ttt;
char buff[200];
char curpathp[1000];

getcwd(curpathp, 999);SaveDialog2->InitialDir=curpathp;
if (SaveDialog2->Execute())
 {
 if (t.xi || t.xf)
    {ttt=fopen(SaveDialog2->FileName.c_str(),"a+b");
     for (i=0;i<t.np;i++){fprintf(ttt,"%06.2f \n",t.pt[i]);}
     fclose(ttt);
     sprintf(buff,"> file %s created",SaveDialog2->FileName.c_str());
     Form3->Memo1->Lines->Add(buff);
    }    else{Form3->Memo1->Lines->Add("> err 003 -  no values to save");}
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::deleteimages1Click(TObject *Sender)
{
static int r,ci;
AnsiString fn;
char buff[200];

Form1->Image1->Canvas->Brush->Color = clBlack;
Form1->Image1->Canvas->Pen->Color = clBlack;
Form1->Image1->Canvas->Rectangle(0,0,300,300);

Form1->Image3->Canvas->Pen->Mode = pmCopy;
Form1->Image3->Canvas->Brush->Color = clWhite;
Form1->Image3->Canvas->Pen->Color = clWhite;
Form1->Image3->Canvas->Rectangle(18,0,60,2000);

//Origin2=Point(0,0);
//MovePt2 = Point(0,0);

view_log_b->Click();
ci=0;
r=0;
close_dtv();

if(FileListBox1->SelCount)
{

if (MessageDlg("Are you sure you want to delete selected files?",
            mtWarning, TMsgDlgButtons() << mbCancel << mbOK , 0) == mrOk)
{
ProgressBar1->Max=floor(FileListBox1->SelCount);
do
{   if (FileListBox1->Selected[ci])
     {FileListBox1->TopIndex=ci-FileListBox1->Height/FileListBox1->ItemHeight;
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);
      remove(fn.c_str());
      FileListBox1->Selected[ci]=0;
      sprintf(buff,"> file %s deleted",fn.c_str());
      Form3->Memo1->Lines->Add(buff);
     }
   ci++;
 } while (FileListBox1->SelCount>0);

 FileListBox1->Update();
 ProgressBar1->Position=0;
}
} else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");

}
//---------------------------------------------------------------------------


void __fastcall TForm1::view_info_bClick(TObject *Sender)
{
Form3->Image1->Visible=0;
Form3->Memo1->Visible=0;
Form3->Image2->Visible=0;
Form3->StringGrid1->Visible=1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::maskeitor1Click(TObject *Sender)
{
 if (t.isloaded)
 {
 view_dtvprev(t.dtvstr);
 Form4->Panel1->Visible=1;
 Form4->Panel3->Visible=0;
 Form4->Panel5->Visible=0;
 Form4->Panel6->Visible=0;
 Form4->Panel7->Visible=0;
 Form4->Panel8->Visible=0;
 Form4->Visible=1;
 Form1->Enabled=0;
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::levelslice1Click(TObject *Sender)
{
 if (t.isloaded)
 {
 view_dtvprev(t.dtvstr);
 Form4->Panel1->Visible=0;
 Form4->Panel5->Visible=0;
 Form4->Panel3->Visible=1;
 Form4->Panel6->Visible=0;
 Form4->Panel7->Visible=0;
 Form4->Panel8->Visible=0;
 Form4->Visible=1;
 Form1->Enabled=0;
 }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::mknormal1Click(TObject *Sender)
{
static int r,ci;
AnsiString fn;
char oldname[1000];
char newname[1000];

view_log_b->Click();

ci=0;
r=0;

if(FileListBox1->SelCount)
 {
 ProgressBar1->Max=floor(FileListBox1->SelCount);
 do
  {   if (FileListBox1->Selected[ci])
     {
      fn=FileListBox1->Directory;
      if (FileListBox1->Directory.Length()!=3) {fn+="\\";}
      fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);
      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_N","dtv",newname);
      mk_normal(oldname,newname);
      FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (FileListBox1->SelCount>0);

 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 view_dtv(newname);
 FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");
}
//---------------------------------------------------------------------------


void __fastcall TForm1::mhrf1Click(TObject *Sender)
{
 if (t.isloaded)
 {
 view_dtvprev(t.dtvstr);
 Form4->Panel1->Visible=0;
 Form4->Panel3->Visible=0;
 Form4->Panel5->Visible=0;
 Form4->Panel7->Visible=0;
 Form4->Panel6->Visible=1;
 Form4->Panel8->Visible=0;
 Form4->Visible=1;
 Form1->Enabled=0;
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::histogram2Click(TObject *Sender)
{
get_hist();
Form3->Memo1->Visible=0;
Form3->Image1->Visible=1;
Form3->Image2->Visible=0;
Form3->StringGrid1->Visible=0;

}

//---------------------------------------------------------------------------

void __fastcall TForm1::histogram1Click(TObject *Sender)
{
get_hist();
histogram2->Click();

}

//---------------------------------------------------------------------------

void __fastcall TForm1::histeq1Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
char oldname[1000];
char newname[1000];

r=0;
ci=0;

if(FileListBox1->SelCount)
 {
 ProgressBar1->Max=floor(FileListBox1->SelCount);
 do
  {   if (FileListBox1->Selected[ci])
     {
      fn=FileListBox1->Directory; if (FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=FileListBox1->Items->Strings[ci];
      ProgressBar1->Position=floor(ProgressBar1->Max-FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_heq","dtv",newname);
      mk_histeq(oldname,newname);
      FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (FileListBox1->SelCount>0);

 FileListBox1->Selected[ci-1]=1;
 ProgressBar1->Position=0;
 view_dtv(newname);

 FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");

}
//---------------------------------------------------------------------------






void __fastcall TForm1::openthermo1Click(TObject *Sender)
{
if (!Form1->Panel1->Visible)
 {Form1->Panel1->Visible=1;
  Form1->Width=Form1->Panel7->Width+Form1->Panel1->Width;
  shrink=0;
 }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Image1DblClick(TObject *Sender)
{
openthermo1->Click();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::mmClick(TObject *Sender)
{
ch_zoom(zoom-0.25,t.isloaded+4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::zoom2Click(TObject *Sender)
{
ch_zoom(zoom+0.25,t.isloaded+4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormResize(TObject *Sender)
{
float opt_zoom;
int size_y;

size_y=t.th.nstv;
if (size_y<5) {size_y=248;}

Form1->ProgressBar1->Top=Form1->Height-71;

if (Panel1->Visible)
   {Form1->Panel7->Width=Form1->Width-Form1->Panel1->Width-10;
   }
else
   {if (shrink)
         {opt_zoom=(float) (Form1->ClientHeight-Form1->Panel8->Height)/size_y;
          ch_zoom(opt_zoom,t.isloaded+2);
         }

    if (Form1->Width > Screen->Width - Form1->Panel1->Width+30)
         {Form1->Panel7->Width=Form1->Width-Form1->Panel1->Width-10;
          Form1->Panel1->Visible=1;
         }
    else {Form1->Panel7->Width=Form1->Width;}
   }

 if (Form1->Width>500) Form1->Image4->Visible=1;
 else Form1->Image4->Visible=0;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::N501Click(TObject *Sender)
{
ch_zoom(0.5,t.isloaded+4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::original1Click(TObject *Sender)
{
ch_zoom(1,t.isloaded+4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N2001Click(TObject *Sender)
{
ch_zoom(2,t.isloaded+4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormDblClick(TObject *Sender)
{
openthermo1->Click();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image4Click(TObject *Sender)
{
Form1->about1->Click();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::optimize1Click(TObject *Sender)
{
float opt_zoom;
int size_y;

size_y=t.th.nstv;
if (size_y<5) {size_y=248;}

shrink=0;
Form1->Left=0;
Form1->WindowState=wsMaximized;
Form3->FormStyle=fsStayOnTop;
Form3->CheckBox2->Checked=1;
Form3->Top=Screen->Height-Form3->Height-50;
Form3->Left=4;

opt_zoom=(float) (Form1->Height-Form3->Height-75.00)/size_y;
ch_zoom(opt_zoom,t.isloaded+2);

}
//---------------------------------------------------------------------------


void __fastcall TForm1::KohonenCSOM1Click(TObject *Sender)
{
 if (t.isloaded)
 {
 view_dtvprev(t.dtvstr);
 Form4->Panel3->Visible=0;
 Form4->Panel1->Visible=0;
 Form4->Panel5->Visible=0;
 Form4->Panel6->Visible=0;
 Form4->Panel7->Visible=1;
 Form4->Panel8->Visible=0;
 Form4->Visible=1;
 Form1->Enabled=0;
 }

}
//---------------------------------------------------------------------------




void __fastcall TForm1::metrics1Click(TObject *Sender)
{
 if (t.isloaded)
 {
 view_dtvprev(t.dtvstr);
 Form4->Panel1->Visible=0;
 Form4->Panel3->Visible=0;
 Form4->Panel5->Visible=0;
 Form4->Panel6->Visible=0;
 Form4->Panel7->Visible=0;
 Form4->Panel8->Visible=1;
 Form4->Visible=1;
 Form1->Enabled=0;
 }

}
//---------------------------------------------------------------------------


void __fastcall TForm1::FileListBox1Click(TObject *Sender)
{
Form3->Visible=1;
viewimg_b();
t.d=1;
DrawingTool = dtLine;
if (Form3->Image1->Visible) {get_hist();}
}
//---------------------------------------------------------------------------

