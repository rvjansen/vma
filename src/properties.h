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
#include <wx/dialog.h>
#include <wx/window.h>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/textctrl.h>
#include <wx/datectrl.h>
#include <wx/choice.h>

#include "vmalib.h"

class Properties : public wxDialog
{
public:
    Properties( wxWindow *parent, void *vma );

    bool TransferDataToWindow();
    bool TransferDataFromWindow();

    bool SetProperties( SUBFILE *sf, const wxString & path = wxEmptyString );

private:
    void *m_vma;
    SUBFILE *m_Subfile;
    wxString m_Msg;

    bool m_IsNew;

    wxTextCtrl *m_FileName;
    wxTextCtrl *m_FileType;
    wxTextCtrl *m_FileMode;

    wxDatePickerCtrl *m_Date;
    wxTextCtrl *m_Time;

    wxChoice *m_Recfm;
    wxTextCtrl *m_Lrecl;

    wxChoice *m_Method;

    wxConfigBase *m_Config;

    DECLARE_EVENT_TABLE()
};

enum
{
    ID_FOR = 101,
    ID_FILENAME,
    ID_FILETYPE,
    ID_FILEMODE,
    ID_DATE,
    ID_TIME,
    ID_RECFM,
    ID_LRECL,
    ID_METHOD
};
