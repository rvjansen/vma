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

// ====================================================================
// headers
// ====================================================================

#include <wx/defs.h>

#include <wx/config.h>
#include <wx/cshelp.h>
#include <wx/datectrl.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/tokenzr.h>
#include <wx/valtext.h>

#include "properties.h"

#define ToWX(X) wxString((char *)(X), wxConvISO8859_1)

// ====================================================================
// Event table
// ====================================================================
BEGIN_EVENT_TABLE( Properties, wxDialog )
END_EVENT_TABLE()

// ====================================================================
// Constructor
// ====================================================================
Properties::Properties( wxWindow *parent, void *vma )
:   wxDialog( parent, wxID_ANY, wxString( wxT("Properties") ) ),
m_vma( vma )
{
    wxBoxSizer *vSizer;
    wxBoxSizer *hSizer;
    wxFlexGridSizer *fSizer;
    wxStaticText *sText;
    wxString recfms[] = { wxT("Fixed"), wxT("Variable") };
    wxString methods[] = { wxT("ASIS"), wxT("LZW"), wxT("S2") };

    // Base grouping
    vSizer = new wxBoxSizer( wxVERTICAL );

    fSizer = new wxFlexGridSizer( 2 );
    fSizer->AddGrowableCol( 1 );

    //
    // File Name
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "File Name:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_FileName = new wxTextCtrl( this,
                                wxID_ANY,
                                wxEmptyString );

    fSizer->Add( m_FileName, 0, wxEXPAND | wxALL, 3 );

    //
    // File Type
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "File Type:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_FileType = new wxTextCtrl( this,
                                wxID_ANY,
                                wxEmptyString );

    fSizer->Add( m_FileType, 0, wxEXPAND | wxALL, 3 );

    //
    // File Mode
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "File Mode:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_FileMode = new wxTextCtrl( this,
                                wxID_ANY,
                                wxEmptyString );

    fSizer->Add( m_FileMode, 0, wxEXPAND | wxALL, 3 );

    //
    // Date
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "Date:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Date = new wxDatePickerCtrl( this,
                                  wxID_ANY );
    m_Date->SetRange( wxDateTime( 1, wxDateTime::Jan, 1900 ),
                     wxDateTime( 12, wxDateTime::Dec, 2099 ) );
    m_Date->SetValue( wxDateTime( 1, wxDateTime::Jan, 1900 ) );

    fSizer->Add( m_Date, 0, wxEXPAND | wxALL, 3 );

    //
    // Time
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "Time:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Time = new wxTextCtrl( this,
                            wxID_ANY,
                            wxEmptyString );

    fSizer->Add( m_Time, 0, wxEXPAND | wxALL, 3 );

    //
    // Record Format
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "Record Format:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Recfm = new wxChoice( this,
                           ID_RECFM,
                           wxDefaultPosition,
                           wxDefaultSize,
                           WXSIZEOF( recfms ),
                           recfms );

    fSizer->Add( m_Recfm, 0, wxEXPAND | wxALL, 3 );

    //
    // Record Length
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "Record Length:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Lrecl = new wxTextCtrl( this,
                             ID_LRECL,
                             wxEmptyString,
                             wxDefaultPosition,
                             wxDefaultSize,
                             0,
                             wxTextValidator( wxFILTER_NUMERIC ) );
    m_Lrecl->SetName( wxT( "Record Length" ) );

    fSizer->Add( m_Lrecl, 0, wxEXPAND | wxALL, 3 );

    //
    // Compression Method
    //
    sText = new wxStaticText( this,
                             wxID_ANY,
                             wxT( "Compression Method:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Method = new wxChoice( this,
                            ID_RECFM,
                            wxDefaultPosition,
                            wxDefaultSize,
                            WXSIZEOF( methods ),
                            methods );

    fSizer->Add( m_Method, 0, wxEXPAND | wxALL, 3 );

    //
    // Done with content
    //
    vSizer->Add( fSizer, 0, wxEXPAND | wxALL, 5 );

    //
    // Divider
    //
    hSizer = new wxBoxSizer( wxHORIZONTAL );

    hSizer->Add( new wxStaticLine( this ), 1 );

    vSizer->Add( hSizer, 0, wxEXPAND | wxALL, 5 );

    //
    // Dialog button group
    //
    vSizer->Add( CreateButtonSizer( wxOK | wxCANCEL ),
                0,
                wxALIGN_RIGHT | wxBOTTOM, 5 );

    SetSizerAndFit( vSizer );

    vSizer->SetSizeHints( this );

    CenterOnParent();
}

// ====================================================================
// Set subfile properties
// ====================================================================
bool Properties::SetProperties( SUBFILE *sf, const wxString & path )
{
    int rc = wxID_OK;

    rc = vma_retain( m_vma );
    if( rc  != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( "Unable to retain subfile - rc: %d, %s\n" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

        return false;
    }

    rc = wxID_OK;

    m_Subfile = sf;

    if( path.IsEmpty() )
    {
        m_Recfm->Enable( false );
        m_Lrecl->Enable( false );
        m_Method->Enable( false );

        rc = ShowModal();
    }
    else
    {
        m_IsNew = true;

        m_Config = wxConfig::Get();
        m_Config->SetPath( wxT("/Settings") );

        m_FileMode->SetValue( m_Config->Read( wxT("Default File Mode"), wxT("A1") ) );
        m_Recfm->SetStringSelection( m_Config->Read( wxT("Default RECFM"), wxT("Variable") ) );
        m_Lrecl->SetValue( m_Config->Read( wxT("Default LRECL"), wxT("65535") ) );
        m_Method->SetStringSelection( m_Config->Read( wxT("Default Method"), wxT("LZW") ) );

        wxFileName name( path );

        m_FileName->SetValue( ToWX( m_Subfile->fn ) );
        m_FileType->SetValue( ToWX( m_Subfile->ft ) );

        int cnt = 0;

        wxStringTokenizer tokenz(name.GetFullName() + wxT("."), wxT(".") );
        while( tokenz.HasMoreTokens() )
        {
            wxString tok = tokenz.GetNextToken();

            cnt++;

            if( cnt == 1 )
            {
                m_FileName->SetValue( tok.Upper() );
            }
            else if( cnt == 2 )
            {
                m_FileType->SetValue( tok.Upper() );
            }
            else if( cnt == 3 )
            {
                m_FileMode->SetValue( tok.Upper() );
            }
        }

        wxDateTime dt( (time_t ) 0 );
        name.GetTimes( NULL, NULL, &dt );

        if( dt.IsValid() )
        {
            m_Date->SetValue( dt );
            m_Time->SetValue( dt.FormatTime() );
        }

        if( !TransferDataFromWindow() )
        {
            wxString title;
            title.Printf( wxT("Properties for %s"), name.GetFullName().c_str() );
            SetTitle( title );
            rc = ShowModal();
        }
    }

    vma_release( m_vma, rc != wxID_OK );

    return ( rc == wxID_OK );
}

// ====================================================================
// Populate dialog with subfile contents
// ====================================================================
bool Properties::TransferDataToWindow()
{
    m_FileName->SetValue( ToWX( m_Subfile->fn ) );
    m_FileType->SetValue( ToWX( m_Subfile->ft ) );
    m_FileMode->SetValue( ToWX( m_Subfile->fm ) );

    wxDateTime dt( m_Subfile->day,
                  (wxDateTime::Month) ( m_Subfile->month - 1 ),
                  m_Subfile->year,
                  m_Subfile->hour,
                  m_Subfile->minute,
                  m_Subfile->second );

    m_Date->SetValue( dt );
    m_Time->SetValue( dt.FormatTime() );

    if( m_Subfile->recfm == VMAR_FIXED )
    {
        m_Recfm->SetStringSelection( wxT("Fixed") );
    }
    else
    {
        m_Recfm->SetStringSelection( wxT("Variable") );
    }

    m_Lrecl->SetValue( wxString::Format( wxT("%d"), m_Subfile->lrecl ) );

    m_Method->SetStringSelection( ToWX( m_Subfile->meth ) );

    return true;
}

// ====================================================================
// Populate subfile with dialog contents
// ====================================================================
bool Properties::TransferDataFromWindow()
{
    wxString val;
    int rc;

    val = m_FileName->GetValue();
    if( !val.IsEmpty() )
    {
        rc = vma_setname( m_vma,
                         val.mb_str(),
                         m_Subfile->ft,
                         m_Subfile->fm );

        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to set file name - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

            return false;
        }
    }

    val = m_FileType->GetValue();
    if( !val.IsEmpty() )
    {
        rc = vma_setname( m_vma,
                         m_Subfile->fn,
                         val.mb_str(),
                         m_Subfile->fm );

        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to set file type - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

            return false;
        }
    }

    val = m_FileMode->GetValue();
    if( !val.IsEmpty() )
    {
        rc = vma_setname( m_vma,
                         m_Subfile->fn,
                         m_Subfile->ft,
                         val.mb_str() );

        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to set file mode - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

            return false;
        }
    }

    wxDateTime dt = m_Date->GetValue();

    rc = vma_setdate( m_vma,
                     dt.GetYear(),
                     dt.GetMonth() + 1,
                     dt.GetDay() );

    if( rc != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( "Unable to set date - rc: %d, %s\n" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

        return false;
    }

    dt.ParseTime( m_Time->GetValue() );

    rc = vma_settime( m_vma,
                     dt.GetHour(),
                     dt.GetMinute(),
                     dt.GetSecond() );

    if( rc != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( "Unable to set time - rc: %d, %s\n" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

        return false;
    }

    if( m_IsNew )
    {
        long lrecl = 65535;
        char recfm;

        if( m_Recfm->GetStringSelection() == wxT("Fixed") )
        {
            recfm = VMAR_FIXED;
        }
        else
        {
            recfm = VMAR_VARIABLE;
        }

        rc = vma_setrecfm( m_vma, recfm );
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to set record format - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

            return false;
        }

        m_Lrecl->GetValue().ToLong( &lrecl );
        rc = vma_setlrecl( m_vma, lrecl );
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to set record length - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

            return false;
        }

        rc = vma_setmethod( m_vma, m_Method->GetStringSelection().mb_str() );
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to set compression method - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ), wxOK, this );

            return false;
        }
    }

    return true;
}
