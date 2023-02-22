//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Unit3.h"
#include "form.h"
#include <vcl/clipbrd.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "general.h"

TForm3 *Form3;
extern struct termograma t;


//---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm3::FormCreate(TObject *Sender)
{
Form3->Image2->Canvas->Brush->Color = clWindow;
Form3->Image2->Canvas->Pen->Color = clWindow;
Form3->Image2->Canvas->Rectangle(0,0,1200,300);
Form3->StringGrid1->ColWidths[7] = 300;
Form3->StringGrid1->ColWidths[0] = 70;
Form3->StringGrid1->Cells[0][0] = "file";
Form3->StringGrid1->Cells[1][0] = "tsc_min";
Form3->StringGrid1->Cells[2][0] = "tsc_max";
Form3->StringGrid1->Cells[3][0] = "tsc_res";
Form3->StringGrid1->Cells[4][0] = "timg_min";
Form3->StringGrid1->Cells[5][0] = "timg_max";
Form3->StringGrid1->Cells[6][0] = "timg_med";
Form3->StringGrid1->Cells[7][0] = "info";

// resetgraph();
}
//---------------------------------------------------------------------------
void __fastcall TForm3::cptoclipboar1Click(TObject *Sender)
{
Clipboard()->Assign(Form3->Image2->Picture);
}
//---------------------------------------------------------------------------
void __fastcall TForm3::saveas1Click(TObject *Sender)
{
  if (SaveDialog1->Execute()){
//    CurrentFile = SaveDialog1->FileName;
    Image2->Picture->SaveToFile( SaveDialog1->FileName );
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm3::CheckBox4Click(TObject *Sender)
{
if (CheckBox4->Checked) {CheckBox1->Checked=0;}    
}
//---------------------------------------------------------------------------

void __fastcall TForm3::CheckBox3Click(TObject *Sender)
{
if (CheckBox3->Checked) {CheckBox1->Checked=0;}    
}
//---------------------------------------------------------------------------

void __fastcall TForm3::CheckBox1Click(TObject *Sender)
{
if (CheckBox1->Checked) {CheckBox3->Checked=0;CheckBox4->Checked=0;}    
}
//---------------------------------------------------------------------------


void __fastcall TForm3::SpeedButton1Click(TObject *Sender)
{
  if ((t.d == 1) || (t.d == 2)) {calcul();}
  if (t.d == 5) {calculrect();}
  if ((t.d == 3) || (t.d == 4)) {calcul();}
  show_graph();
  show_info();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::SpeedButton2Click(TObject *Sender)
{
Panel2->Visible=!Panel2->Visible;
Image1->Visible=0;
Image2->Visible=1;
Memo1->Visible=0;
StringGrid1->Visible=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::SpeedButton5Click(TObject *Sender)
{
Image1->Visible=0;
Image2->Visible=1;
Memo1->Visible=0;
StringGrid1->Visible=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::SpeedButton4Click(TObject *Sender)
{
Image1->Visible=0;
Image2->Visible=0;
Memo1->Visible=0;
Panel2->Visible=0;
StringGrid1->Visible=1;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::SpeedButton3Click(TObject *Sender)
{
Image1->Visible=0;
Image2->Visible=0;
Memo1->Visible=1;
Panel2->Visible=0;
StringGrid1->Visible=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::SpeedButton6Click(TObject *Sender)
{
Form1->histogram2->Click();
Form3->Image1->Visible=1;
Form3->Image2->Visible=0;
Form3->Memo1->Visible=0;
Panel2->Visible=0;
Form3->StringGrid1->Visible=0;

}
//---------------------------------------------------------------------------

void __fastcall TForm3::CheckBox2Click(TObject *Sender)
{


if (Form3->CheckBox2->Checked) {Form3->FormStyle=fsStayOnTop;}
else {Form3->FormStyle=fsNormal;}
}
//---------------------------------------------------------------------------


void __fastcall TForm3::copytoclipboard1Click(TObject *Sender)
{
Clipboard()->Assign(Form3->Image1->Picture);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::saveas2Click(TObject *Sender)
{
  if (SaveDialog1->Execute()){
    Image1->Picture->SaveToFile( SaveDialog1->FileName );
   }
}
//---------------------------------------------------------------------------

