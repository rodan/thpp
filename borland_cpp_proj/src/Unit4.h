//---------------------------------------------------------------------------
#ifndef Unit4H
#define Unit4H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Mask.hpp>
#include <checklst.hpp>
#include <CheckLst.hpp>
//---------------------------------------------------------------------------
class TForm4 : public TForm
{
__published:	// IDE-managed Components
    TOpenDialog *OpenDialog1;
    TPanel *Panel1;
    TStringGrid *StringGrid1;
    TPanel *Panel2;
    TImage *Image1;
    TPanel *Panel4;
    TSpeedButton *SpeedButton25;
    TSpeedButton *SpeedButton21;
    TSpeedButton *SpeedButton22;
    TSpeedButton *SpeedButton23;
    TSpeedButton *SpeedButton24;
    TSpeedButton *SpeedButton26;
    TSpeedButton *SpeedButton27;
    TSpeedButton *SpeedButton28;
    TSpeedButton *SpeedButton29;
    TLabel *Label2;
    TLabel *Label3;
    TSpeedButton *SpeedButton4;
    TSpeedButton *SpeedButton3;
    TSpeedButton *SpeedButton1;
    TSpeedButton *SpeedButton6;
    TSpeedButton *SpeedButton2;
    TLabel *Label1;
    TPanel *Panel3;
    TLabel *Label4;
    TSpeedButton *SpeedButton7;
    TSpeedButton *SpeedButton8;
    TSpeedButton *SpeedButton9;
    TSpeedButton *SpeedButton10;
    TTrackBar *TrackBar2;
    TLabel *Label5;
    TLabel *Label6;
    TPanel *Panel5;
    TSpeedButton *SpeedButton12;
    TSpeedButton *SpeedButton14;
    TSpeedButton *SpeedButton15;
    TSpeedButton *SpeedButton16;
    TLabel *Label7;
    TMaskEdit *MaskEdit1;
    TMaskEdit *MaskEdit2;
    TLabel *Label8;
    TLabel *Label9;
    TImage *Image2;
    TSpeedButton *SpeedButton11;
    TSpeedButton *SpeedButton5;
    TSaveDialog *SaveDialog1;
    TSpeedButton *SpeedButton13;
    TSpeedButton *SpeedButton17;
    TSpeedButton *SpeedButton18;
    TSpeedButton *SpeedButton19;
    TSpeedButton *SpeedButton20;
    TSpeedButton *SpeedButton30;
    TSpeedButton *SpeedButton31;
    TTrackBar *TrackBar1;
    TPanel *Panel6;
    TSpeedButton *SpeedButton32;
    TSpeedButton *SpeedButton33;
    TSpeedButton *SpeedButton34;
    TSpeedButton *SpeedButton35;
    TLabel *Label10;
    TLabel *Label11;
    TMaskEdit *MaskEdit3;
    TMaskEdit *MaskEdit4;
    TMaskEdit *MaskEdit5;
    TMaskEdit *MaskEdit6;
    TLabel *Label12;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label15;
    TCheckBox *cb_edges;
    TCheckBox *cb_near_edges;
    TCheckBox *cb_homogeneous;
    TCheckBox *cb_pikes;
    TLabel *Label16;
    TRadioButton *rb_black;
    TRadioButton *rb_back;
    TLabel *Label17;
    TBevel *Bevel1;
    TBevel *Bevel2;
    TMaskEdit *MaskEdit7;
    TPanel *Panel7;
    TLabel *Label18;
    TSpeedButton *SpeedButton36;
    TSpeedButton *SpeedButton37;
    TSpeedButton *SpeedButton38;
    TSpeedButton *SpeedButton39;
    TBevel *Bevel3;
    TMaskEdit *MaskEdit10;
    TLabel *Label20;
    TLabel *Label21;
    TLabel *Label22;
    TMaskEdit *MaskEdit11;
    TMaskEdit *MaskEdit12;
    TLabel *Label23;
    TMaskEdit *MaskEdit8;
    TBevel *Bevel5;
    TLabel *Label24;
    TSaveDialog *SaveDialog2;
    TLabel *Label25;
    TMaskEdit *MaskEdit13;
    TComboBox *ComboBox1;
    TLabel *Label26;
    TLabel *Label27;
    TCheckListBox *CheckListBox1;
    TSpeedButton *SpeedButton40;
    TSpeedButton *SpeedButton41;
    TCheckBox *cb_random;
    TLabel *Label19;
    TLabel *Label28;
    TLabel *Label29;
    TCheckListBox *CheckListBox2;
        TPanel *Panel8;
        TComboBox *ComboBox2;
        TMaskEdit *MaskEdit9;
        TLabel *Label30;
        TSpeedButton *SpeedButton42;
        TSpeedButton *SpeedButton43;
    void __fastcall FormCreate(TObject *Sender);
    
    
    void __fastcall SpeedButton4Click(TObject *Sender);
    
    void __fastcall SpeedButton3Click(TObject *Sender);
    
    
    void __fastcall SpeedButton6Click(TObject *Sender);
    void __fastcall SpeedButton21Click(TObject *Sender);
    void __fastcall SpeedButton22Click(TObject *Sender);
    void __fastcall SpeedButton23Click(TObject *Sender);
    void __fastcall SpeedButton26Click(TObject *Sender);
    void __fastcall SpeedButton25Click(TObject *Sender);
    void __fastcall SpeedButton24Click(TObject *Sender);
    void __fastcall SpeedButton27Click(TObject *Sender);
    void __fastcall SpeedButton28Click(TObject *Sender);
    void __fastcall SpeedButton29Click(TObject *Sender);
    
    void __fastcall SpeedButton1Click(TObject *Sender);
    void __fastcall SpeedButton2Click(TObject *Sender);
    
    void __fastcall TrackBar1Change(TObject *Sender);
    
    
    void __fastcall SpeedButton7Click(TObject *Sender);
    void __fastcall SpeedButton9Click(TObject *Sender);
    void __fastcall SpeedButton10Click(TObject *Sender);
    
    void __fastcall TrackBar2Change(TObject *Sender);
    void __fastcall SpeedButton8Click(TObject *Sender);
    void __fastcall SpeedButton12Click(TObject *Sender);
    void __fastcall SpeedButton15Click(TObject *Sender);
    
    void __fastcall SpeedButton14Click(TObject *Sender);
    void __fastcall SpeedButton11Click(TObject *Sender);
    
    
    
    void __fastcall SpeedButton16Click(TObject *Sender);
    
    void __fastcall SpeedButton5Click(TObject *Sender);
    void __fastcall SpeedButton13Click(TObject *Sender);
    void __fastcall SpeedButton17Click(TObject *Sender);
    void __fastcall SpeedButton18Click(TObject *Sender);
    void __fastcall SpeedButton19Click(TObject *Sender);
    void __fastcall SpeedButton20Click(TObject *Sender);
    void __fastcall SpeedButton30Click(TObject *Sender);
    void __fastcall SpeedButton31Click(TObject *Sender);
    
    
    
    void __fastcall SpeedButton32Click(TObject *Sender);
    void __fastcall SpeedButton35Click(TObject *Sender);
    void __fastcall SpeedButton34Click(TObject *Sender);
    
    
    void __fastcall SpeedButton33Click(TObject *Sender);
    
    
    
    
    
    
    
    void __fastcall SpeedButton36Click(TObject *Sender);
    void __fastcall SpeedButton37Click(TObject *Sender);
    
    void __fastcall SpeedButton38Click(TObject *Sender);
    void __fastcall SpeedButton39Click(TObject *Sender);
    
    void __fastcall MaskEdit12Change(TObject *Sender);
    
    
    
    
    void __fastcall ComboBox1Change(TObject *Sender);
    void __fastcall SpeedButton41Click(TObject *Sender);
    void __fastcall SpeedButton40Click(TObject *Sender);
    
    void __fastcall Image1MouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall SpeedButton43Click(TObject *Sender);
        void __fastcall SpeedButton42Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TForm4(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm4 *Form4;
//---------------------------------------------------------------------------
#endif
