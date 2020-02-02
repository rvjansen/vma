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

#include <wx/app.h>
#include <wx/config.h>
#include <wx/cshelp.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/tokenzr.h>

#if defined( _WIN32 )
#include <shlwapi.h>
#include <shlobj.h>
#endif

#include "settings.h"

#include "res/find16.xpm"

// ====================================================================
// Event table
// ====================================================================
BEGIN_EVENT_TABLE( Settings, wxDialog )

// Button handlers
EVT_BUTTON( ID_UPDATE,  Settings::OnUpdate       )
EVT_BUTTON( ID_REMOVE,  Settings::OnRemove       )
EVT_BUTTON( ID_TXTBRO,  Settings::OnTextBrowse   )
EVT_BUTTON( ID_BINBRO,  Settings::OnBinaryBrowse )
EVT_BUTTON( ID_FROMBRO, Settings::OnFromBrowse   )
EVT_BUTTON( ID_TOBRO,   Settings::OnToBrowse     )

END_EVENT_TABLE()

// ====================================================================
// Constructor
// ====================================================================
Settings::Settings( wxWindow *parent, int id, wxString title )
: wxDialog( parent, id, title )
{
    wxNotebook *book;
    wxNotebookPage *page;
    wxBoxSizer *vSizer;
    wxBoxSizer *hSizer;
    wxFlexGridSizer *fSizer;
    wxStaticBoxSizer *sSizer;
    wxStaticText *sText;
    wxBitmapButton *BBtn;
    wxButton *Btn;

    //
    // Base grouping
    //
    book = new wxNotebook( this, wxID_ANY );

    // ----------------------------------------------------------------
    // General page
    // ----------------------------------------------------------------
    page = new wxPanel( book );

    vSizer = new wxBoxSizer( wxVERTICAL );

    //
    // Extensions grouping
    //
    sSizer = new wxStaticBoxSizer( wxVERTICAL,
                                  page,
                                  wxT( "&Extensions" ) );

    //
    // Extensions edit grouping
    //
    hSizer = new wxBoxSizer( wxHORIZONTAL );

    m_Exts = new wxTextCtrl( page,
                            wxID_ANY,
                            wxT( "*.vmarc;*.vma" ) );

    hSizer->Add( m_Exts, 1 );

    sSizer->Add( hSizer, 1, wxEXPAND | wxALL, 3 );

    //
    // Extensions button grouping
    //
    hSizer = new wxBoxSizer( wxHORIZONTAL );

    Btn = new wxButton( page,
                       ID_UPDATE,
                       wxT( "&Update" ) );

    hSizer->Add( Btn, 0 );

    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Separate with ';'" ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             wxALIGN_CENTRE );

    hSizer->Add( sText, 1, wxALIGN_CENTER_VERTICAL | wxALL, 3 );

    Btn = new wxButton( page,
                       ID_REMOVE,
                       wxT( "&Remove" ) );

    hSizer->Add( Btn, 0 );

    sSizer->Add( hSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 3 );

    vSizer->Add( sSizer, 0, wxEXPAND | wxALL, 5 );

    //
    // Conversion grouping
    //
    sSizer = new wxStaticBoxSizer( wxVERTICAL,
                                  page,
                                  wxT( "Conversion UCMs" ) );

    fSizer = new wxFlexGridSizer( 3 );
    fSizer->AddGrowableCol( 1 );

    //
    // From UCM grouping
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "&From" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_FromUCM = new wxTextCtrl( page,
                               wxID_ANY,
                               wxT( "" ) );

    fSizer->Add( m_FromUCM, 0, wxEXPAND | wxALL, 3 );

    BBtn = new wxBitmapButton( page,
                              ID_FROMBRO,
                              wxBitmap( find16_xpm ) );
    BBtn->SetName( wxT( "Browse for file..." ) );
    BBtn->SetToolTip( wxT( "Browse for file..." ) );

    fSizer->Add( BBtn, 0, wxALIGN_CENTER_VERTICAL );

    //
    // To UCM grouping
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "&To" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_ToUCM = new wxTextCtrl( page,
                             wxID_ANY,
                             wxT( "" ) );

    fSizer->Add( m_ToUCM, 0, wxEXPAND | wxALL, 3 );

    BBtn = new wxBitmapButton( page,
                              ID_TOBRO,
                              wxBitmap( find16_xpm ) );
    BBtn->SetName( wxT( "Browse for file..." ) );
    BBtn->SetToolTip( wxT( "Browse for file..." ) );

    fSizer->Add( BBtn, 0, wxALIGN_CENTER_VERTICAL );

    sSizer->Add( fSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 3 );

    vSizer->Add( sSizer, 0, wxEXPAND | wxALL, 5 );

    page->SetSizer( vSizer );

    book->AddPage( page, wxT("General") );

    // ----------------------------------------------------------------
    // Addition page
    // ----------------------------------------------------------------
    page = new wxPanel( book );

    vSizer = new wxBoxSizer( wxVERTICAL );

    //
    // Defaults grouping
    //
    sSizer = new wxStaticBoxSizer( wxVERTICAL,
                                  page,
                                  wxT( "Defaults" ) );

    fSizer = new wxFlexGridSizer( 3 );
    fSizer->AddGrowableCol( 1 );

    //
    // From UCM grouping
    //

    wxString recfms[] = { wxT("Fixed"), wxT("Variable") };
    wxString methods[] = { wxT("ASIS"), wxT("LZW"), wxT("S2") };
    wxString modes[] = { wxT("Auto"), wxT("Text"), wxT("Binary") };

    // Base grouping
    vSizer = new wxBoxSizer( wxVERTICAL );

    fSizer = new wxFlexGridSizer( 2 );
    fSizer->AddGrowableCol( 1 );

    //
    // File Mode
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "File Mode:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_FileMode = new wxTextCtrl( page,
                                wxID_ANY,
                                wxEmptyString );

    fSizer->Add( m_FileMode, 0, wxEXPAND | wxALL, 3 );

    //
    // Record Format
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Record Format:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Recfm = new wxChoice( page,
                           wxID_ANY,
                           wxDefaultPosition,
                           wxDefaultSize,
                           WXSIZEOF( recfms ),
                           recfms );

    fSizer->Add( m_Recfm, 0, wxEXPAND | wxALL, 3 );

    //
    // Record Length
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Record Length:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Lrecl = new wxTextCtrl( page,
                             wxID_ANY,
                             wxEmptyString );
    //    m_Lrecl->SetRange( 1, 65535 );
    //    m_Lrecl->SetValue( 65535 );
    m_Lrecl->SetName( wxT( "Record Length" ) );

    fSizer->Add( m_Lrecl, 0, wxEXPAND | wxALL, 3 );

    //
    // Compression Method
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Compression Method:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Method = new wxChoice( page,
                            wxID_ANY,
                            wxDefaultPosition,
                            wxDefaultSize,
                            WXSIZEOF( methods ),
                            methods );

    fSizer->Add( m_Method, 0, wxEXPAND | wxALL, 3 );

    //
    // Conversion Mode
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Conversion Mode:" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_Mode = new wxChoice( page,
                          wxID_ANY,
                          wxDefaultPosition,
                          wxDefaultSize,
                          WXSIZEOF( modes ),
                          modes );

    fSizer->Add( m_Mode, 0, wxEXPAND | wxALL, 3 );

    sSizer->Add( fSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 3 );

    vSizer->Add( sSizer, 0, wxEXPAND | wxALL, 5 );

    page->SetSizer( vSizer );

    book->AddPage( page, wxT("Addition") );

    // ----------------------------------------------------------------
    // Extraction page
    // ----------------------------------------------------------------
    page = new wxPanel( book );

    vSizer = new wxBoxSizer( wxVERTICAL );

    //
    // Viewers grouping
    //
    sSizer = new wxStaticBoxSizer( wxVERTICAL,
                                  page,
                                  wxT( "Viewers" ) );

    fSizer = new wxFlexGridSizer( 3 );
    fSizer->AddGrowableCol( 1 );

    //
    // Text viewer grouping
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Te&xt" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_TxtViewer = new wxTextCtrl( page,
                                 wxID_ANY,
                                 wxT( "Notepad" ) );

    fSizer->Add( m_TxtViewer, 0, wxEXPAND | wxALL, 3 );

    BBtn = new wxBitmapButton( page,
                              ID_TXTBRO,
                              wxBitmap( find16_xpm ) );
    BBtn->SetName( wxT( "Browse for file..." ) );
    BBtn->SetToolTip( wxT( "Browse for file..." ) );

    fSizer->Add( BBtn, 0, wxALIGN_CENTER_VERTICAL );

    //
    // Binary viewer grouping
    //
    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "Bi&nary" ) );

    fSizer->Add( sText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT );

    m_BinViewer = new wxTextCtrl( page,
                                 wxID_ANY,
                                 wxT( "Notepad" ) );

    fSizer->Add( m_BinViewer, 0, wxEXPAND | wxALL, 3 );

    BBtn = new wxBitmapButton( page,
                              ID_BINBRO,
                              wxBitmap( find16_xpm ) );
    BBtn->SetName( wxT( "Browse for file..." ) );
    BBtn->SetToolTip( wxT( "Browse for file..." ) );

    fSizer->Add( BBtn, 0, wxALIGN_CENTER_VERTICAL );

    sSizer->Add( fSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 3 );

    vSizer->Add( sSizer, 0, wxEXPAND | wxALL, 5 );

    //
    // Default list action
    //
    hSizer = new wxBoxSizer( wxHORIZONTAL );

    sText = new wxStaticText( page,
                             wxID_ANY,
                             wxT( "&List double click/enter action" ) );

    hSizer->Add( sText, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );

    m_Action = new wxComboBox( page,
                              wxID_ANY,
                              wxT( "View" ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0,
                              NULL,
                              wxCB_READONLY );

    m_Action->Append( wxT( "View" ) );
    m_Action->Append( wxT( "Extract" ) );

    hSizer->Add( m_Action, 0, wxCENTER | wxLEFT | wxRIGHT, 5 );

    vSizer->Add( hSizer, 0, wxEXPAND | wxALL, 3 );

    page->SetSizer( vSizer );

    book->AddPage( page, wxT("Extraction") );

    // ----------------------------------------------------------------
    // Dialog grouping
    // ----------------------------------------------------------------
    vSizer = new wxBoxSizer( wxVERTICAL );

    vSizer->Add( book, 0, wxEXPAND | wxALL, 5 );

    //
    // Dialog button grouping
    //
    vSizer->Add( CreateButtonSizer( wxOK | wxCANCEL ),
                0,
                wxALIGN_RIGHT | wxBOTTOM, 5 );

    SetSizerAndFit( vSizer );

    vSizer->SetSizeHints( this );

    CenterOnParent();
}

// ====================================================================
// Populate the dialog
// ====================================================================
bool Settings::TransferDataToWindow()
{
    //
    // Get access to our config manager
    //
    m_Config = wxConfig::Get();

    //
    // Load required settings
    //
    m_Config->SetPath( wxT("/Settings") );

    m_TxtViewer->SetValue( m_Config->Read( wxT("Text Viewer"), wxT("Notepad") ) );

    m_BinViewer->SetValue( m_Config->Read( wxT("Binary Viewer"), wxT("Notepad") ) );

    m_FromUCM->SetValue( m_Config->Read( wxT("From UCM"), wxT("") ) );

    m_ToUCM->SetValue( m_Config->Read( wxT("To UCM"), wxT("") ) );

    m_Exts->SetValue( m_Config->Read( wxT("Extensions"), wxT(".vma;.vmarc") ) );

    m_Action->SetValue( m_Config->Read( wxT("Default Action"), wxT("View") ) );

    m_FileMode->SetValue( m_Config->Read( wxT("Default File Mode"), wxT("A1") ) );

    m_Recfm->SetStringSelection( m_Config->Read( wxT("Default RECFM"), wxT("Variable") ) );

    m_Lrecl->SetValue( m_Config->Read( wxT("Default LRECL"), wxT("65535") ) );

    m_Method->SetStringSelection( m_Config->Read( wxT("Default Method"), wxT("LZW") ) );

    return true;
}

// ====================================================================
// Get the users choices
// ====================================================================
bool Settings::TransferDataFromWindow()
{
    //
    // Get access to our config manager
    //
    m_Config = wxConfig::Get();

    //
    // Write all settings
    //
    m_Config->SetPath( wxT("/Settings") );

    m_Config->Write( wxT("Text Viewer"),
                    m_TxtViewer->GetValue() );

    m_Config->Write( wxT("Binary Viewer"),
                    m_BinViewer->GetValue() );

    m_Config->Write( wxT("From UCM"),
                    m_FromUCM->GetValue() );

    m_Config->Write( wxT("To UCM"),
                    m_ToUCM->GetValue() );

    m_Config->Write( wxT("Default Action"),
                    m_Action->GetValue() );

    m_Config->Write( wxT("Default File Mode"),
                    m_FileMode->GetValue() );

    m_Config->Write( wxT("Default RECFM"),
                    m_Recfm->GetStringSelection() );

    m_Config->Write( wxT("Default LRECL"),
                    m_Lrecl->GetValue() );

    m_Config->Write( wxT("Default Method"),
                    m_Method->GetStringSelection() );

    return true;
}

// ====================================================================
// Update button clicked
// ====================================================================
void Settings::OnUpdate( wxCommandEvent& event )
{

#if defined( _WIN32 )

    wxString exts = m_Exts->GetValue();
    wxStringTokenizer st( exts, wxT( ";" ) );
    wxString ext;
    wxString app;
    wxString val;
    int cnt;

    // Remove existing extensions
    OnRemove( event );

    // Nothing left to do if no extensions specified
    if( exts.IsEmpty() )
    {
        return;
    }

    // Get a fully qualified path to our app
    ::PathCanonicalize( app.GetWriteBuf( MAX_PATH ), wxTheApp->argv[ 0 ] );
    app.UngetWriteBuf();

    // Build the application entry
    ::SHSetValue( HKEY_CLASSES_ROOT,
                 wxT( "VMARC_Hive" ),
                 wxT( "" ),
                 REG_SZ,
                 wxT( "VMARC Hive" ),
                 sizeof( wxT( "VMARC Hive" ) ) - 1 );

    ::SHSetValue( HKEY_CLASSES_ROOT,
                 wxT( "VMARC_Hive\\shell" ),
                 wxT( "" ),
                 REG_SZ,
                 wxT( "open" ),
                 sizeof( wxT( "open" ) ) - 1 );

    ::SHSetValue( HKEY_CLASSES_ROOT,
                 wxT( "VMARC_Hive\\shell\\open" ),
                 wxT( "" ),
                 REG_SZ,
                 wxT( "Open" ),
                 sizeof( wxT( "Open" ) ) - 1 );

    val = app + wxT( " \"%1\"" );
    ::SHRegSetPath( HKEY_CLASSES_ROOT,
                   wxT( "VMARC_Hive\\shell\\open\\command" ),
                   wxT( "" ),
                   val,
                   0 );

    ::SHSetValue( HKEY_CLASSES_ROOT,
                 wxT( "VMARC_Hive\\shell\\extract" ),
                 wxT( "" ),
                 REG_SZ,
                 wxT( "Extract to..." ),
                 sizeof( wxT( "Extract to..." ) ) - 1 );

    val = app + wxT( " /x \"%1\"" );
    ::SHRegSetPath( HKEY_CLASSES_ROOT,
                   wxT( "VMARC_Hive\\shell\\extract\\command" ),
                   wxT( "" ),
                   val,
                   0 );

    val = app + wxT( ",0" );
    ::SHRegSetPath( HKEY_CLASSES_ROOT,
                   wxT( "VMARC_Hive\\DefaultIcon" ),
                   wxT( "" ),
                   val,
                   0 );

    // Add each extension
    cnt = 0;
    while( st.HasMoreTokens() )
    {
        // Extract the next ";" delimited token
        ext = st.GetNextToken().Strip( wxString::both );
        if( ext.Length() == 0 )
        {
            continue;
        }

        // Remove leading "*", if any
        if( ext[ 0 ] == wxT( '*' ) )
        {
            ext.Remove( 0, 1 );
        }

        // Remove leading ".", if any
        if( ext[ 0 ] == wxT( '.' ) )
        {
            ext.Remove( 0, 1 );
        }

        // Associate the extension with our application
        ::SHSetValue( HKEY_CLASSES_ROOT,
                     wxT( '.' ) + ext,
                     wxT( "" ),
                     REG_SZ,
                     wxT( "VMARC_Hive" ),
                     sizeof( wxT( "VMARC_Hive" ) ) - 1 );

        // Track number associated
        cnt++;
    }

    // Tell everyone the associations changed
    SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0 );

    // Store extensions in registry
    //
    // NOTE:  Shouldn't need to do this if we scanned the registry for
    //        currently associated.
    m_Config->Write( wxT( "Extensions" ),
                    exts );

    // Tell 'em what happened
    exts.Printf( wxT( "%d extensions registered" ), cnt );
    wxMessageBox( exts, wxT( "Helpful Message" ) );

#endif

    return;
}

// ====================================================================
// Remove button clicked
// ====================================================================
void Settings::OnRemove( wxCommandEvent& event )
{
#if defined( _WIN32 )

    wxChar buf[ MAX_PATH ];
    wxChar val[ MAX_PATH ];
    DWORD len;
    DWORD ndx;
    DWORD type;
    bool ffirst = 0;

    // Get rid of the program entry
    ::SHDeleteKey( HKEY_CLASSES_ROOT,
                  wxT( "VMARC_Hive" ) );

    // Scan all HKCR entries beginning with "."
    ndx = 0;
    len = sizeof( buf );
    while( ::SHEnumKeyEx( HKEY_CLASSES_ROOT, ndx, buf, &len ) == ERROR_SUCCESS )
    {
        // Ignore until first extension, then stop when last is processed
        if( buf[ 0 ] != '.' )
        {
            if( ffirst )
            {
                break;
            }
        }
        else
        {
            ffirst = 1;
        }

        // Get the default value
        len = sizeof( val );
        type = REG_SZ;
        SHGetValue( HKEY_CLASSES_ROOT,
                   buf,
                   wxT( "" ),
                   &type,
                   val,
                   &len );

        // Delete it if it's ours
        if( wxStrcmp( val, wxT( "VMARC_Hive" ) ) == 0 )
        {
            ::SHDeleteKey( HKEY_CLASSES_ROOT,
                          buf );
        }

        // Prepare for next
        ndx++;
        len = sizeof( buf ) - 1;
    }

    // Tell everyone the associations changed
    ::SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0 );

    // Bypass if we were called from OnUpdate()
    if( event.GetId() == ID_REMOVE )
    {
        // Clear edit box
        m_Exts->SetValue( wxT( "" ) );

        // And registry setting
        m_Config->Write( wxT( "Extensions" ),
                        wxT( "" ) );

        // Let 'em know what happened
        wxMessageBox( wxT( "All extensions removed.\n" )
                     wxT( "You must click 'Update' to re-associate." ),
                     wxT( "Helpful Message" ) );
    }

#endif

    return;
}

// ====================================================================
// Browse for text viewer clicked
// ====================================================================
void Settings::OnTextBrowse( wxCommandEvent& WXUNUSED( event ) )
{
    wxFileDialog fd( this,
                    wxT( "Choose your favorite text viewing program" ),
                    m_TxtViewer->GetValue(),
                    wxT( "" ),
                    wxT( "All Files|*.*" ) );

    // Show the dialog
    if( fd.ShowModal() != wxID_OK )
    {
        return;
    }

    // Store path in edit control
    m_TxtViewer->SetValue( fd.GetPath() );

    return;
}

// ====================================================================
// Browse for binary viewer clicked
// ====================================================================
void Settings::OnBinaryBrowse( wxCommandEvent& WXUNUSED( event ) )
{
    wxFileDialog fd( this,
                    wxT( "Choose your favorite binary viewing program" ),
                    m_BinViewer->GetValue(),
                    wxT( "" ),
                    wxT( "All Files|*.*" ) );

    // Show the dialog
    if( fd.ShowModal() != wxID_OK )
    {
        return;
    }

    // Store path in edit control
    m_BinViewer->SetValue( fd.GetPath() );

    return;
}

// ====================================================================
// Browse for source UCM clicked
// ====================================================================
void Settings::OnFromBrowse( wxCommandEvent& WXUNUSED( event ) )
{
    wxFileDialog fd( this,
                    wxT( "Select the source UCM" ),
                    m_FromUCM->GetValue(),
                    wxT( "" ),
                    wxT( "All Files|*.*" ) );

    // Show the dialog
    if( fd.ShowModal() != wxID_OK )
    {
        return;
    }

    // Store path in edit control
    m_FromUCM->SetValue( fd.GetPath() );

    return;
}

// ====================================================================
// Browse for destination UCM clicked
// ====================================================================
void Settings::OnToBrowse( wxCommandEvent& WXUNUSED( event ) )
{
    wxFileDialog fd( this,
                    wxT( "Select the target UCM" ),
                    m_ToUCM->GetValue(),
                    wxT( "" ),
                    wxT( "All Files|*.*" ) );

    // Show the dialog
    if( fd.ShowModal() != wxID_OK )
    {
        return;
    }

    // Store path in edit control
    m_ToUCM->SetValue( fd.GetPath() );

    return;
}
