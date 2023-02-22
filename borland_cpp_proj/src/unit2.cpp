//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "form.h"
#include "Unit2.h"
#include "version.h"
#include <io.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormCreate(TObject *Sender)
{
char buff[100];

sprintf(buff,"version %d.%d.%d",major_version,minor_version,release);
Label2->Caption=buff;


}
//---------------------------------------------------------------------------

void __fastcall TForm2::SpeedButton1Click(TObject *Sender)
{
Close();    
}
//---------------------------------------------------------------------------




