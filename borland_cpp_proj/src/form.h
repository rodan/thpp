//---------------------------------------------------------------------------
#ifndef FORMH
#define FORMH
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\FileCtrl.hpp>
#include <vcl\VCFrmla1.hpp>
#include <vcl\OleCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\Mask.hpp>
#include <vcl\ComCtrls.hpp>
#include "Grids.hpp"
#include <vcl\Dialogs.hpp>
#include <vcl\Menus.hpp>
#include <Graphics.hpp>

//---------------------------------------------------------------------------

//enum TDrawingTool {dtLine, dtRectangle, dtEllipse, dtRoundRect};
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TOpenDialog *OpenDialog1;
	TSaveDialog *SaveDialog1;
	TSaveDialog *SaveDialog2;
	TPopupMenu *PopupMenu1;
	TMenuItem *copytoclipboard1;
	TMenuItem *saveas1;
	TMenuItem *info1;
	TPopupMenu *PopupMenu3;
	TMenuItem *copytoclipboard3;
	TMenuItem *saveas3;
    TMainMenu *MainMenu1;
    TMenuItem *File1;
    TMenuItem *Image5;
    TMenuItem *View1;
    TMenuItem *Help1;
    TMenuItem *Palette1;
    TMenuItem *edgedetect1;
    TMenuItem *Nagao1;
    TMenuItem *gradient1;
    TMenuItem *N2561;
    TMenuItem *color1;
    TMenuItem *grey1;
    TMenuItem *hmetal1;
    TMenuItem *hmetal11;
    TMenuItem *hotblue1;
    TMenuItem *hotblue21;
    TMenuItem *pertrue1;
    TMenuItem *pericolor1;
    TMenuItem *rainbow1;
    TMenuItem *external1;
    TMenuItem *default1;
    TMenuItem *transitinal1;
    TMenuItem *about1;
    TMenuItem *savegaphimage1;
    TMenuItem *savegraphvalues1;
    TMenuItem *saveall1;
    TMenuItem *N1;
    TMenuItem *exit1;
    TMenuItem *getinfo1;
    TMenuItem *export1;
    TMenuItem *bmp1;
    TMenuItem *tools2;
    TMenuItem *areaevaluation1;
    TMenuItem *histogram1;
    TMenuItem *normal1;
    TMenuItem *metrics1;
    TMenuItem *view_log_b;
    TMenuItem *view_graph_b;
    TMenuItem *lookuptable1;
    TMenuItem *hmetal21;
    TProgressBar *ProgressBar1;
    TMenuItem *savevaluessamefile1;
    TMenuItem *deleteimages1;
    TMenuItem *view_info_b;
    TMenuItem *maskeitor1;
    TMenuItem *mknormal1;
    TMenuItem *mhrf1;
    TMenuItem *histogram2;
    TMenuItem *histeq1;
    TMenuItem *openthermo1;
    TPanel *Panel3;
    TSpeedButton *SpeedButton1;
    TPanel *Panel4;
    TDirectoryListBox *DirectoryListBox1;
    TDriveComboBox *DriveComboBox1;
    TPanel *Panel5;
    TFileListBox *FileListBox1;
    TPanel *Panel8;
    TImage *Image4;
    TMenuItem *zoom2;
    TMenuItem *mm;
    TPanel *Panel7;
    TPanel *Panel2;
    TImage *Image1;
    TPanel *Panel6;
    TImage *Image3;
    TMenuItem *zoom3;
    TMenuItem *original1;
    TMenuItem *N2001;
    TMenuItem *N501;
    TMenuItem *optimize1;
    TLabel *Label1;
    TMenuItem *Analysis1;
    TMenuItem *segmentation1;
    TMenuItem *KohonenCSOM1;
    TMenuItem *median1;
    TMenuItem *enhancement1;
    void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall SpeedButton1Click(TObject *Sender);
	
	void __fastcall MaskEdit3Change(TObject *Sender);
	void __fastcall copytoclipboard1Click(TObject *Sender);
	void __fastcall saveas1Click(TObject *Sender);
	void __fastcall info1Click(TObject *Sender);
	
	void __fastcall BitBtn8Click(TObject *Sender);
	
	void __fastcall Image1MouseMove(TObject *Sender, TShiftState Shift, int X,
	int Y);
	void __fastcall Image1MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall Image1MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall copytoclipboard3Click(TObject *Sender);
	void __fastcall saveas3Click(TObject *Sender);
	
	
	
	

	
	void __fastcall Exit1Click(TObject *Sender);

	void __fastcall bmp1Click(TObject *Sender);
    void __fastcall getinfo1Click(TObject *Sender);
    void __fastcall lookuptable1Click(TObject *Sender);
    void __fastcall savegaphimage1Click(TObject *Sender);
    void __fastcall savegraphvalues1Click(TObject *Sender);
    void __fastcall saveall1Click(TObject *Sender);
    void __fastcall view_graph_bClick(TObject *Sender);
    void __fastcall view_log_bClick(TObject *Sender);
    void __fastcall about1Click(TObject *Sender);
    void __fastcall Nagao1Click(TObject *Sender);
    void __fastcall gradient1Click(TObject *Sender);
    void __fastcall normal1Click(TObject *Sender);
    void __fastcall areaevaluation1Click(TObject *Sender);
    void __fastcall aliase1Click(TObject *Sender);
    
    void __fastcall default1Click(TObject *Sender);
    void __fastcall color1Click(TObject *Sender);
    void __fastcall grey1Click(TObject *Sender);
    void __fastcall hmetal1Click(TObject *Sender);
    void __fastcall hmetal11Click(TObject *Sender);
    void __fastcall hmetal2(TObject *Sender);
    void __fastcall hotblue1Click(TObject *Sender);
    void __fastcall hotblue21Click(TObject *Sender);
    void __fastcall pertrue1Click(TObject *Sender);
    void __fastcall pericolor1Click(TObject *Sender);
    void __fastcall rainbow1Click(TObject *Sender);
    void __fastcall external1Click(TObject *Sender);

    void __fastcall savevaluessamefile1Click(TObject *Sender);
    void __fastcall deleteimages1Click(TObject *Sender);


    void __fastcall view_info_bClick(TObject *Sender);

    
    void __fastcall maskeitor1Click(TObject *Sender);
    void __fastcall levelslice1Click(TObject *Sender);
    
    void __fastcall mknormal1Click(TObject *Sender);
    
    
    
    void __fastcall mhrf1Click(TObject *Sender);
    void __fastcall histogram2Click(TObject *Sender);
    
    void __fastcall histogram1Click(TObject *Sender);
    void __fastcall histeq1Click(TObject *Sender);
    
    
    
    
    
    void __fastcall openthermo1Click(TObject *Sender);
    
    void __fastcall Image1DblClick(TObject *Sender);
    void __fastcall mmClick(TObject *Sender);
    void __fastcall zoom2Click(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall N501Click(TObject *Sender);
    void __fastcall original1Click(TObject *Sender);
    void __fastcall N2001Click(TObject *Sender);
    void __fastcall FormDblClick(TObject *Sender);
    void __fastcall Image4Click(TObject *Sender);
    void __fastcall optimize1Click(TObject *Sender);
    
    void __fastcall KohonenCSOM1Click(TObject *Sender);
        void __fastcall metrics1Click(TObject *Sender);
        void __fastcall FileListBox1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
