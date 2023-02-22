//---------------------------------------------------------------------------
#ifndef Unit3H
#define Unit3H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
#include <Mask.hpp>
//---------------------------------------------------------------------------
class TForm3 : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel3;
    TImage *Image2;
    TRichEdit *Memo1;
    TPopupMenu *PopupMenu1;
    TMenuItem *cptoclipboar1;
    TMenuItem *saveas1;
    TSaveDialog *SaveDialog1;
    TStringGrid *StringGrid1;
    TPanel *Panel1;
    TPanel *Panel2;
    TCheckBox *CheckBox4;
    TCheckBox *CheckBox3;
    TMaskEdit *MaskEdit8;
    TMaskEdit *MaskEdit9;
    TCheckBox *CheckBox1;
    TSpeedButton *SpeedButton1;
    TSpeedButton *SpeedButton2;
    TLabel *Label1;
    TSpeedButton *SpeedButton3;
    TSpeedButton *SpeedButton4;
    TSpeedButton *SpeedButton5;
    TImage *Image1;
    TSpeedButton *SpeedButton6;
    TCheckBox *CheckBox2;
        TPopupMenu *PopupMenu2;
        TMenuItem *copytoclipboard1;
        TMenuItem *saveas2;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall cptoclipboar1Click(TObject *Sender);
    void __fastcall saveas1Click(TObject *Sender);
    void __fastcall CheckBox4Click(TObject *Sender);
    void __fastcall CheckBox3Click(TObject *Sender);
    void __fastcall CheckBox1Click(TObject *Sender);
    
    void __fastcall SpeedButton1Click(TObject *Sender);
    void __fastcall SpeedButton2Click(TObject *Sender);
    void __fastcall SpeedButton5Click(TObject *Sender);
    void __fastcall SpeedButton4Click(TObject *Sender);
    void __fastcall SpeedButton3Click(TObject *Sender);
    void __fastcall SpeedButton6Click(TObject *Sender);
    void __fastcall CheckBox2Click(TObject *Sender);
        void __fastcall copytoclipboard1Click(TObject *Sender);
        void __fastcall saveas2Click(TObject *Sender);
    
private:	// User declarations
public:		// User declarations
    __fastcall TForm3(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm3 *Form3;
//---------------------------------------------------------------------------
#endif
