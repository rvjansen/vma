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
==================================================================== */

#include <wx/defs.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/config.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/spinctrl.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/window.h>

class Settings : public wxDialog
{
public:
    Settings( wxWindow *parent, int id = -1, wxString title = wxT( "Settings" ) ) ;

    bool TransferDataToWindow();
    bool TransferDataFromWindow();

private:
    void OnUpdate( wxCommandEvent& event );
    void OnRemove( wxCommandEvent& event );

    void OnTextBrowse( wxCommandEvent& event );
    void OnBinaryBrowse( wxCommandEvent& event );
    void OnFromBrowse( wxCommandEvent& event );
    void OnToBrowse( wxCommandEvent& event );

    wxTextCtrl      *m_TxtViewer;
    wxTextCtrl      *m_BinViewer;

    wxTextCtrl      *m_Exts;

    wxTextCtrl      *m_FromUCM;
    wxTextCtrl      *m_ToUCM;

    wxTextCtrl      *m_FileMode;
    wxChoice        *m_Recfm;
    wxTextCtrl      *m_Lrecl;
    wxChoice        *m_Method;
    wxChoice        *m_Mode;

    wxComboBox      *m_Action;
    wxCheckBox      *m_Stats;

    wxConfigBase    *m_Config;
    wxLogWindow     *m_Log;

    DECLARE_EVENT_TABLE()
};

enum
{
    ID_TXTBRO = 101,
    ID_BINBRO,
    ID_UPDATE,
    ID_REMOVE,
    ID_FROMBRO,
    ID_TOBRO
};
