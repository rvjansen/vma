/* ====================================================================
||
|| VMAgui - GUI viewer/extractor/creator for VMARC Hives
||
|| This little utility allows you to view and extract subfiles from
|| archives in VMARC format.
||
|| Written by:  Leland Lucius (vma@homerow.net>
||
|| Copyright:  Public Domain (just use your conscience)
||
 ====================================================================*/

#include <wx/defs.h>

#include <wx/bmpbuttn.h>
#include <wx/dynarray.h>
#include <wx/panel.h>
#include <wx/sizer.h>

WX_DECLARE_OBJARRAY( wxBitmap, BitmapArray );

// ====================================================================
// The application
// ====================================================================
class MyFrame;
class MyApp : public wxApp
{
public:
    MyApp(){};
    bool OnInit();

    void MacOpenFile( const wxString& fileName );

    MyFrame *frame;
};

// ====================================================================
// The GUI dialog
// ====================================================================
class MyFrame : public wxFrame
{
public:
    MyFrame();

    bool LoadArchive( wxString name = wxT("") );
    bool SaveArchive();

private:
    void OnCloseWindow( wxCloseEvent& event );
    void OnClose( wxCloseEvent& event );
    void OnQuit( wxCommandEvent& event );
    void OnCtrlA( wxCommandEvent& event );
    void OnEnter( wxCommandEvent& event );
    void OnNew( wxCommandEvent& event );
    void OnOpen( wxCommandEvent& event );
    void OnSave( wxCommandEvent& event );
    void OnAdd( wxCommandEvent& event );
    void OnDelete( wxCommandEvent& event );
    void OnExtract( wxCommandEvent& event );
    void OnExtAll( wxCommandEvent& event );
    void OnView( wxCommandEvent& event );
    void OnProperties( wxCommandEvent& event );
    void OnSettings( wxCommandEvent& event );
    void OnMode( wxCommandEvent& event );
    void OnColumnClick( wxListEvent& event );
    void OnActivated( wxListEvent& event );

    void LoadSettings( void );
    void SaveSettings( void );

    bool Extract( SUBFILE *sf, wxString dest, int nItem );

    bool OpenArchive();
    void CloseArchive();
    bool ReadArchive();

    void AutoSizeColumns( void );
    wxString Format( SUBFILE *sf, int nField );
    wxString OutputName( SUBFILE *sf );
    static int wxCALLBACK Compare( long item1, long item2, long sortData );

    wxButton *AddButton( wxSizer *sz, int id, const char *const *image, const wxChar *tip );

private:
    wxStatusBar     *m_Status;
    wxPanel         *m_Panel;
    wxButton        *m_New;
    wxButton        *m_Open;
    wxButton        *m_Save;
    wxButton        *m_Add;
    wxButton        *m_Delete;
    wxButton        *m_Extract;
    wxButton        *m_ExtAll;
    wxButton        *m_View;
    wxButton        *m_Props;
    wxButton        *m_Settings;

    wxTextCtrl      *m_Pattern;

    wxRadioButton   *m_Auto;
    wxRadioButton   *m_Text;
    wxRadioButton   *m_Binary;

    wxListView      *m_List;

    enum Format
    {
        FmtAuto = 0,
        FmtText,
        FmtBinary
    };

    enum Field
    {
        Fn = 0,
        Ft,
        Fm,
        Date,
        Recfm,
        Lrecl,
        Cbytes,
        Ubytes,
        Verrel,
        Method,
        Dtype,
        Count
    };

    void *m_vma;
    wxConfigBase *m_Config;

    wxString m_Filename;
    wxString m_OpenPath;
    wxString m_SavePath;
    wxString m_FromUCM;
    wxString m_ToUCM;
    wxString m_TxtViewer;
    wxString m_BinViewer;
    wxString m_Filter;
    wxString m_Msg;
    wxString m_Action;
    bool m_Stats;
    int m_Format;

    int m_ImageUnknown;
    int m_ImageText;
    int m_ImageBinary;
    int m_ImageUp;
    int m_ImageDown;

    SUBFILE *m_sf;
    Field m_SortBy;
    int m_Dir;
    char m_SortDir[ Count ];

    wxImageList *m_Images;

    BitmapArray *m_Bitmaps;

#if defined( DEBUG )
    wxLogWindow *m_Log;
#endif

    DECLARE_EVENT_TABLE()
};

// controls and menu constants
enum
{
    ID_NEW = wxID_HIGHEST + 1,
    ID_OPEN,
    ID_SAVE,
    ID_ADD,
    ID_DELETE,
    ID_EXTRACT,
    ID_EXTALL,
    ID_VIEW,
    ID_PROPS,
    ID_SETTINGS,

    ID_OUTPAT,

    ID_AUTO,
    ID_TEXT,
    ID_BINARY,

    ID_LIST,

    ID_ENTER,
    ID_CTRL_A
};

// ====================================================================
// A little class to allow us to get rid of the temporary file when the
// viewing app completes.
//
// NOTE:  If the user closes VMAgui before closing the viewer app, the
//        temporary file will not get deleted.  This could be solved
//        by not allowing VMAgui to terminate until all viewers had
//        terminated.
// ====================================================================
class MyProcess : public wxProcess
{
public:
    MyProcess( const wxString& temp )
    {
        m_Temp = temp;
    }

    //
    // Override OnTerminate() to delete temp file when command completes
    //
    void OnTerminate( int WXUNUSED( pid ), int WXUNUSED( status ) )
    {
        wxRemoveFile( m_Temp );
    }

private:
    wxString m_Temp;
};
