//---------------------------------------------------------------------------
#include <vcl.h>

#pragma hdrstop
#include "Unit4.h"
#include "Unit3.h"
#include "form.h"
#include <math.h>

#include "general.h"
#include "processing.h"
#include "analysis.h"

float prev_zoom=1;

extern AnsiString getfnbuff;
extern AnsiString CurrentFile;
extern struct mask_struct msk;
extern struct termograma t;

//---------------------------------------------------------------------------
//#pragma package(smart_init)
#pragma resource "*.dfm"
#pragma nopackwarning

TForm4 *Form4;

void exec_cancel();

void exec_cancel()
{
Form1->Enabled=1;
Form4->Visible=0;
Form1->FileListBox1->Update();
}

//---------------------------------------------------------------------------
__fastcall TForm4::TForm4(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------

void __fastcall TForm4::FormCreate(TObject *Sender)
{
int i;
char buff[20];

Form4->StringGrid1->ColWidths[0] = 18;
Form4->StringGrid1->RowHeights[0] = 15;
Form4->StringGrid1->Cells[0][0] = " x";
Form4->StringGrid1->Cells[1][0] = "  -2";
Form4->StringGrid1->Cells[2][0] = "  -1";
Form4->StringGrid1->Cells[3][0] = "   0";
Form4->StringGrid1->Cells[4][0] = "  +1";
Form4->StringGrid1->Cells[5][0] = "  +2";
Form4->StringGrid1->Cells[0][1]= "-2";
Form4->StringGrid1->Cells[0][2]= "-1";
Form4->StringGrid1->Cells[0][3] = " 0";
Form4->StringGrid1->Cells[0][4] = "+1";
Form4->StringGrid1->Cells[0][5] = "+2";

for (i=0;i<256;i++)
  {sprintf(buff,"label #%d",i);
   CheckListBox1->Items->Add(buff);
   CheckListBox1->Checked[i]=true;
  }

CheckListBox2->Checked[0]=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton4Click(TObject *Sender)
{
//Form4->OpenDialog1->InitialDir=curpath;
if(Form4->OpenDialog1->Execute())
 {loadmskfile(Form4->OpenDialog1->FileName.c_str());
  mask2form4();
 }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TForm4::SpeedButton3Click(TObject *Sender)
{
reset_mask(100);
mask2form4();
reset_dtvprev();
}
//---------------------------------------------------------------------------
void __fastcall TForm4::SpeedButton6Click(TObject *Sender)
{
exec_cancel();
}
//---------------------------------------------------------------------------
void __fastcall TForm4::SpeedButton21Click(TObject *Sender)
{
Image1->Left=0;
Image1->Top=0;
Image1->Width=512;
Image1->Height=496;
prev_zoom=2;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton22Click(TObject *Sender)
{
if (Image1->Top<-20)
{
Image1->Top+=floor( (Image1->Height)*5/100 );
 }
    
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton23Click(TObject *Sender)
{
Image1->Left=0;
Image1->Top=0;
Image1->Width=1024;
Image1->Height=992;
prev_zoom=4;    
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton26Click(TObject *Sender)
{
if (Image1->Left<-15)
{
Image1->Left+=floor( (Image1->Width)*5/100 );
 }
    
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton25Click(TObject *Sender)
{
Image1->Left=0;
Image1->Top=0;
Image1->Width=256;
Image1->Height=248;
prev_zoom=1;    
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton24Click(TObject *Sender)
{
if (Image1->Left>270-Image1->Width)
{
Image1->Left+=-floor( (Image1->Width)*5/100 );
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton27Click(TObject *Sender)
{
Image1->Left=0;
Image1->Top=0;
Image1->Width=1536;
Image1->Height=1488;
prev_zoom=6;    
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton28Click(TObject *Sender)
{
if (Image1->Top>288-Image1->Height)
{
Image1->Top+=-floor( (Image1->Height)*5/100 );
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton29Click(TObject *Sender)
{
Image1->Left=0;
Image1->Top=0;
Image1->Width=2048;
Image1->Height=1984;
prev_zoom=8;
}
//---------------------------------------------------------------------------


void __fastcall TForm4::SpeedButton1Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
int once=0;
char oldname[1000];
char newname[1000];

form42mask();
mask2form4();

r=0;
ci=0;

if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"prev","dtv",newname);
      mk_m(oldname,newname,TrackBar1->Position,2,1);
      Form1->FileListBox1->Selected[ci]=0;
      once++;
     }
    ci++;
  } while (once==0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 view_dtvprev(newname);
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton2Click(TObject *Sender)
{

AnsiString fn;
static int r,ci;
char oldname[1000];
char newname[1000];

r=0;
ci=0;

if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_msk","dtv",newname);
      mk_m(oldname,newname,TrackBar1->Position,2,1);
      Form1->FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (Form1->FileListBox1->SelCount>0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 Form1->FileListBox1->Update();
 view_dtv(newname);
 }
Form1->Enabled=1;
Form4->Visible=0;
}
//---------------------------------------------------------------------------


void __fastcall TForm4::TrackBar1Change(TObject *Sender)
{
Form4->Label3->Caption=Form4->TrackBar1->Position;
}
//---------------------------------------------------------------------------



void __fastcall TForm4::SpeedButton7Click(TObject *Sender)
{
exec_cancel();
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton9Click(TObject *Sender)
{
int n_pixels;
AnsiString fn;
static int r,ci,once=0;
char oldname[1000];
char newname[1000];

n_pixels=Form4->TrackBar2->Position;

r=0;
ci=0;

if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"prev","dtv",newname);
      mk_level_slice(oldname,newname,n_pixels);
      once++;
     }
    ci++;
  } while (once == 0);
 view_dtvprev(newname);
 once=0;
}

}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton10Click(TObject *Sender)
{
int n_pixels;
AnsiString fn;
static int r,ci;
char oldname[1000];
char newname[1000];

n_pixels=Form4->TrackBar2->Position;

r=0;
ci=0;

if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_LS","dtv",newname);
      mk_level_slice(oldname,newname,n_pixels);
      Form1->FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (Form1->FileListBox1->SelCount>0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 Form1->FileListBox1->Update();
 } else Form3->Memo1->Lines->Add("> err 001 -  no file(s) selected");
 view_dtv(newname);
 Form1->Enabled=1;
 Form4->Visible=0;

}
//---------------------------------------------------------------------------


void __fastcall TForm4::TrackBar2Change(TObject *Sender)
{
Label6->Caption=TrackBar2->Position;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton8Click(TObject *Sender)
{
 reset_dtvprev();
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton12Click(TObject *Sender)
{
 reset_dtvprev();
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton15Click(TObject *Sender)
{
exec_cancel();
}
//---------------------------------------------------------------------------


void __fastcall TForm4::SpeedButton14Click(TObject *Sender)
{
lut_prev(2);
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton11Click(TObject *Sender)
{
lut_prev(1);
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton16Click(TObject *Sender)
{
lut_prev(3);
Form1->Enabled=1;
Form4->Visible=0;

}
//---------------------------------------------------------------------------


void __fastcall TForm4::SpeedButton5Click(TObject *Sender)
{
//Form4->SaveDialog1->InitialDir=curpath;
if(Form4->SaveDialog1->Execute())
 {form42mask();
  mask2form4();
  savemskfile(Form4->SaveDialog1->FileName.c_str());
  mask2form4();
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton13Click(TObject *Sender)
{
TrackBar2->Position=2;
Label6->Caption=2;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton17Click(TObject *Sender)
{
TrackBar2->Position=4;
Label6->Caption=4;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton18Click(TObject *Sender)
{
TrackBar2->Position=8;
Label6->Caption=8;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton19Click(TObject *Sender)
{
TrackBar2->Position=16;
Label6->Caption=16;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton20Click(TObject *Sender)
{
TrackBar2->Position=32;
Label6->Caption=32;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton30Click(TObject *Sender)
{
TrackBar2->Position=64;
Label6->Caption=64;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton31Click(TObject *Sender)
{
TrackBar2->Position=128;
Label6->Caption=128;
}
//---------------------------------------------------------------------------






void __fastcall TForm4::SpeedButton32Click(TObject *Sender)
{
exec_cancel();
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton35Click(TObject *Sender)
{
reset_dtvprev();
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton34Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
int once=0;
int options;
float t1,t2,t3,wmax;
char oldname[1000];
char newname[1000];

t1=atof(Form4->MaskEdit3->Text.c_str());
t2=atof(Form4->MaskEdit4->Text.c_str());
t3=atof(Form4->MaskEdit5->Text.c_str());
wmax=atof(Form4->MaskEdit6->Text.c_str());

options=1*(int)cb_pikes->Checked+2*(int)cb_edges->Checked+4*(int)cb_near_edges->Checked+8*(int)cb_homogeneous->Checked+16*(int)rb_black->Checked+32*(int)rb_back->Checked;
r=0;
ci=0;


if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"prev","dtv",newname);
      mk_mhrf(oldname,newname,t1,t2,t3,wmax,options);
      Form1->FileListBox1->Selected[ci]=0;
      once++;
     }
    ci++;
  } while (once==0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 view_dtvprev(newname);
 }

}
//---------------------------------------------------------------------------




void __fastcall TForm4::SpeedButton33Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
float t1,t2,t3,wmax;
int options;
char oldname[1000];
char newname[1000];

t1=atof(Form4->MaskEdit3->Text.c_str());
t2=atof(Form4->MaskEdit4->Text.c_str());
t3=atof(Form4->MaskEdit5->Text.c_str());
wmax=atof(Form4->MaskEdit6->Text.c_str());

options=1*(int)cb_pikes->Checked+2*(int)cb_edges->Checked+4*(int)cb_near_edges->Checked+8*(int)cb_homogeneous->Checked+16*(int)rb_black->Checked+32*(int)rb_back->Checked;
r=0;
ci=0;

if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"_MHR","dtv",newname);
      mk_mhrf(oldname,newname,t1,t2,t3,wmax,options);
      Form1->FileListBox1->Selected[ci]=0;
     }
    ci++;
  } while (Form1->FileListBox1->SelCount>0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 Form1->FileListBox1->Update();
 view_dtv(newname);
 }
Form1->Enabled=1;
Form4->Visible=0;

}
//---------------------------------------------------------------------------







void __fastcall TForm4::SpeedButton36Click(TObject *Sender)
{
exec_cancel();    
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton37Click(TObject *Sender)
{
reset_dtvprev();    
}
//---------------------------------------------------------------------------


void __fastcall TForm4::SpeedButton38Click(TObject *Sender)
{
AnsiString fn;
static int r,ci;
int once=0;
int i;
int options;
int output_neurons,window_size,max_epoch,neigh;
bool *show_layer;
bool use_0=0,use_1=0;
float max_learn_rate;
char oldname[1000];
char newname[1000];

if (atoi(Form4->MaskEdit8->Text.c_str())<2) {Form4->MaskEdit8->Text="2";}
if (atoi(Form4->MaskEdit11->Text.c_str())>5) {Form4->MaskEdit11->Text="1";}
if (atoi(Form4->MaskEdit11->Text.c_str())<=0) {Form4->MaskEdit11->Text="1";}
if (atof(Form4->MaskEdit13->Text.c_str())>1) {Form4->MaskEdit13->Text="0.9";}

output_neurons=atoi(Form4->MaskEdit12->Text.c_str());
if (output_neurons<3) {output_neurons=3;MaskEdit12->Text=3;}

window_size=atoi(Form4->MaskEdit11->Text.c_str());
max_epoch=atoi(Form4->MaskEdit8->Text.c_str())-1;
//min_evolution=atof(Form4->MaskEdit9->Text.c_str());
neigh=atoi(Form4->MaskEdit10->Text.c_str());
max_learn_rate=atof(Form4->MaskEdit13->Text.c_str());

if (CheckListBox2->Checked[0]) {use_0=1;}
if (CheckListBox2->Checked[1]) {use_1=1;}
//if (cb_random->Checked) {use_random=1;}

options=1+4*(int)use_0+8*(int)use_1;//+16*(int)use_random;

show_layer=(bool *)malloc((output_neurons)*sizeof(bool));

for (i=0;i<output_neurons;i++)
    {show_layer[i]=0;
     if (CheckListBox1->Checked[i])
      {show_layer[i]=1;};
    }

r=0;
ci=0;

if(Form1->FileListBox1->SelCount)
 {
 Form1->ProgressBar1->Max=floor(Form1->FileListBox1->SelCount);
 do
  {   if (Form1->FileListBox1->Selected[ci])
     {
      fn=Form1->FileListBox1->Directory; if (Form1->FileListBox1->Directory.Length()!=3) fn+="\\"; fn+=Form1->FileListBox1->Items->Strings[ci];
      Form1->ProgressBar1->Position=floor(Form1->ProgressBar1->Max-Form1->FileListBox1->SelCount);

      sprintf(oldname,"%s",fn.c_str());
      get_name(oldname,"prev","dtv",newname);
      csom_segmentation(oldname, newname, window_size, output_neurons, max_epoch, neigh, 99999, max_learn_rate, options, show_layer);
      Form1->FileListBox1->Selected[ci]=0;
      once++;
     }
    ci++;
  } while (once==0);

 Form1->FileListBox1->Selected[ci-1]=1;
 Form1->ProgressBar1->Position=0;
 view_dtvprev(newname);
 free(show_layer);
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton39Click(TObject *Sender)
{
if(Form4->SaveDialog2->Execute())
 {
  CurrentFile = SaveDialog2->FileName;
  Form4->Image1->Picture->SaveToFile(CurrentFile);
 }
}
//---------------------------------------------------------------------------


void __fastcall TForm4::MaskEdit12Change(TObject *Sender)
{
int output_neurons;
int neigh;

output_neurons=atoi(MaskEdit12->Text.c_str());
neigh=floor((output_neurons-1)/2);
MaskEdit10->Text=neigh;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::ComboBox1Change(TObject *Sender)
{
if (ComboBox1->Text=="punctiform_01")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="5"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="1"; // window size
    MaskEdit8->Text="50";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="punctiform_02")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="6"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="1"; // window size
    MaskEdit8->Text="50";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="punctiform_99")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=true;
    MaskEdit12->Text="6"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="1"; // window size
    MaskEdit8->Text="200";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="long&thin_01")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="20"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="1"; // window size
    MaskEdit8->Text="20";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="long&thin_02")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=true;
    MaskEdit12->Text="20"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="1"; // window size
    MaskEdit8->Text="50";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="long&thin_99")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="20"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="1"; // window size
    MaskEdit8->Text="200";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="large_01")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="5"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="2"; // window size
    MaskEdit8->Text="50";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="large_02")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="10"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="2"; // window size
    MaskEdit8->Text="50";  // max epochs
    SpeedButton41->Click();
   }

if (ComboBox1->Text=="large_99")
   {CheckListBox2->Checked[0]=true;
    CheckListBox2->Checked[1]=false;
    MaskEdit12->Text="10"; // output neurons
//    MaskEdit10->Text=; // neigh
    MaskEdit13->Text="0.5"; // learn rate
    MaskEdit11->Text="2"; // window size
    MaskEdit8->Text="200";  // max epochs
    SpeedButton41->Click();
   }
if (ComboBox1->Text=="gaussian noise")
   {MaskEdit11->Text="2"; // window size
   }
if (ComboBox1->Text=="no noise")
   {MaskEdit11->Text="1"; // window size
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton41Click(TObject *Sender)
{
int i;
for (i=0;i<256;i++) {CheckListBox1->Checked[i]=1;}
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton40Click(TObject *Sender)
{
int i;
for (i=0;i<256;i++) {CheckListBox1->Checked[i]=0;}
}
//---------------------------------------------------------------------------


void __fastcall TForm4::Image1MouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{

int output_neurons;

if (Panel7->Visible)
 {output_neurons=atoi(Form4->MaskEdit12->Text.c_str());
  Label19->Caption=floor(get_pixel_temp(floor(X/prev_zoom),floor(Y/prev_zoom))*(output_neurons/254.00000001));
 }


}
//---------------------------------------------------------------------------




void __fastcall TForm4::SpeedButton43Click(TObject *Sender)
{
float multiplicator,tmp;

if (Form4->ComboBox2->Text=="mm") {multiplicator=1.0;}
if (Form4->ComboBox2->Text=="cm") {multiplicator=10.0;}
if (Form4->ComboBox2->Text=="m") {multiplicator=1000.0;}
if (Form4->ComboBox2->Text=="inch") {multiplicator=25.4;}

sscanf(Form4->MaskEdit9->Text.c_str(),"%f", &tmp);

t.mmlen=tmp*multiplicator;
t.mmpp=t.mmlen/t.pixellen;

Form1->Enabled=1;
Form4->Visible=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton42Click(TObject *Sender)
{
exec_cancel();        
}
//---------------------------------------------------------------------------

