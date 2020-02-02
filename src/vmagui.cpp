/* ====================================================================
||
|| VMAgui - GUI viewer/extractor/creqator for VMARC Hives
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
#include <wx/frame.h>

#include <wx/busyinfo.h>
#include <wx/button.h>
#include <wx/config.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/process.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/tokenzr.h>

#include "vmalib.h"
#include "version.h"
#include "vmagui.h"
#include "settings.h"
#include "properties.h"

// ====================================================================
// pixmaps
// ====================================================================
#include "res/vmagui.xpm"
#include "res/unknown16.xpm"
#include "res/text16.xpm"
#include "res/binary16.xpm"
#include "res/uparrow16.xpm"
#include "res/downarrow16.xpm"

// ====================================================================
// These are from the Tango Icon Gallery
// http://tango.freedesktop.org/Tango_Icon_Gallery
// ====================================================================
#include "res/new16.xpm"
#include "res/open16.xpm"
#include "res/save16.xpm"
#include "res/add16.xpm"
#include "res/remove16.xpm"
#include "res/extract16.xpm"
#include "res/extall16.xpm"
#include "res/view16.xpm"
#include "res/props16.xpm"
#include "res/settings16.xpm"

#if defined( DEBUG )
#define log wxLogMessage
#else
#define log
#endif

#if defined(_WIN32)
#define COMPARE stricmp
#else
#define COMPARE strcasecmp
#endif

#define ToWX(X) wxString((char *)(X), wxConvISO8859_1)

#define TITLE "VMAgui - VMARC Viewer/Extractor/Creator - Version " VERSION

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(  BitmapArray )

// ====================================================================
// We are an application (no, really, we are.)
// ====================================================================
IMPLEMENT_APP( MyApp )

// ====================================================================
// Main entry method
// ====================================================================
bool MyApp::OnInit()
{
    frame = new MyFrame;

    // Display the dialog
    frame->Show( true );

    // Make sure our's is on top
    SetTopWindow( frame );

    return true;
}

#if defined(__WXMAC__)
void MyApp::MacOpenFile( const wxString& fileName )
{
    // Just open and display the hive contents
    frame->LoadArchive( fileName );

    return;
}
#endif

// ====================================================================
// Event table
// ====================================================================
BEGIN_EVENT_TABLE( MyFrame, wxFrame )
//
// Window handlers
//
EVT_CLOSE(                              MyFrame::OnClose        )

//
// Button handlers
//
EVT_BUTTON(                 ID_NEW,     MyFrame::OnNew          )
EVT_BUTTON(                 ID_OPEN,    MyFrame::OnOpen         )
EVT_BUTTON(                 ID_SAVE,    MyFrame::OnSave         )
EVT_BUTTON(                 ID_ADD,     MyFrame::OnAdd          )
EVT_BUTTON(                 ID_DELETE,  MyFrame::OnDelete       )
EVT_BUTTON(                 ID_EXTRACT, MyFrame::OnExtract      )
EVT_BUTTON(                 ID_EXTALL,  MyFrame::OnExtAll       )
EVT_BUTTON(                 ID_VIEW,    MyFrame::OnView         )
EVT_BUTTON(                 ID_PROPS,   MyFrame::OnProperties   )
EVT_BUTTON(                 ID_SETTINGS,MyFrame::OnSettings     )

EVT_RADIOBUTTON(            ID_AUTO,    MyFrame::OnMode         )
EVT_RADIOBUTTON(            ID_TEXT,    MyFrame::OnMode         )
EVT_RADIOBUTTON(            ID_BINARY,  MyFrame::OnMode         )

//
// Listview handlers
//
EVT_LIST_COL_CLICK(         ID_LIST,    MyFrame::OnColumnClick  )
EVT_LIST_ITEM_ACTIVATED(    ID_LIST,    MyFrame::OnActivated    )

//
// Accelerator handlers
//
EVT_MENU(                   wxID_EXIT,  MyFrame::OnQuit         )
EVT_MENU(                   ID_CTRL_A,  MyFrame::OnCtrlA        )
END_EVENT_TABLE()

// ====================================================================
// Constructor
// ====================================================================
MyFrame::MyFrame()
:   wxFrame( NULL,
             wxID_ANY,
             wxT( TITLE ),
             wxDefaultPosition,
             wxSize( -1, 150 ),
             wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE )
{
    m_vma = NULL;

    // Get access to our config manager
    m_Config = wxConfig::Get();

    // Set to record all defaults
    m_Config->SetRecordDefaults();

    // Add our cut little teddy icon
    SetIcon( wxICON( vmagui ) );

    // Create a nice little status bar
    {
        wxString v = wxT( "v" VERSION );
        wxCoord w, h;
        int ws[ 2 ];

        GetTextExtent( v, &w, &h );
        ws[ 0 ] = -1;
        ws[ 1 ] = w + 40;

        m_Status = CreateStatusBar( 2 );

        SetStatusWidths( 2, ws );

        SetStatusText( wxT( "" ), 0 );
        SetStatusText( v, 1 );
    }

    // Use a panel to get automatic TAB handling
    m_Panel = new wxPanel( this, wxID_ANY );

    // Create base sizer
    wxBoxSizer *szBase = new wxBoxSizer( wxVERTICAL );

    // Create the top sizer
    wxBoxSizer *szTop = new wxBoxSizer( wxHORIZONTAL );

    // Create the button grouping
    wxBoxSizer *szBtn = new wxBoxSizer( wxHORIZONTAL );

    m_New = AddButton( szBtn, ID_NEW, document_new, wxString( wxT( "New" ) ) );
    m_Open = AddButton( szBtn, ID_OPEN, document_open, wxT( "Open" ) );
    m_Save = AddButton( szBtn, ID_SAVE, document_save, wxT( "Save" ) );

    szBtn->AddSpacer( 15 );

    m_Add = AddButton( szBtn, ID_ADD, list_add, wxT( "Add" ) );
    m_Delete = AddButton( szBtn, ID_DELETE, list_remove, wxT( "Delete" ) );

    szBtn->AddSpacer( 15 );

    m_Extract = AddButton( szBtn, ID_EXTRACT, go_next, wxT( "Extract" ) );
    m_ExtAll = AddButton( szBtn, ID_EXTALL, go_last, wxT( "Extract All" ) );

    szBtn->AddSpacer( 15 );

    m_View = AddButton( szBtn, ID_VIEW, system_search, wxT( "View" ) );

    szBtn->AddSpacer( 15 );

    m_Props = AddButton( szBtn, ID_PROPS, document_properties, wxT( "Properties" ) );

    szBtn->AddSpacer( 15 );

    m_Settings = AddButton( szBtn, ID_SETTINGS, preferences_system, wxT( "Settings" ) );

    szTop->Add( szBtn, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL );

    // Add some cushion
    szTop->AddSpacer( 30 );

    // And make it stretchy
    szTop->AddStretchSpacer();

    // Create the output pattern grouping
    wxBoxSizer *szPattern = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *text = new wxStaticText( m_Panel,
                                          wxID_ANY,
                                          wxT( "&Pattern:" ) );
    szPattern->Add( text, 0, wxALIGN_CENTER_VERTICAL );

    m_Pattern = new wxTextCtrl( m_Panel,
                               ID_OUTPAT,
                               wxT( "%n.%t.%m" ) );
    szPattern->Add( m_Pattern, 0, wxTOP, 1 );

    szTop->Add( szPattern, 0, wxALIGN_CENTER );

    // Add some cushion
    szTop->AddSpacer( 30 );

    // And make it stretchy
    szTop->AddStretchSpacer();

    // Create the extraction type grouping
    wxBoxSizer *szType = new wxBoxSizer( wxHORIZONTAL );

    m_Auto = new wxRadioButton( m_Panel,
                               ID_AUTO,
                               wxT( "A&uto" ) );
    szType->Add( m_Auto );

    m_Text = new wxRadioButton( m_Panel,
                               ID_TEXT,
                               wxT( "&Text" ) );
    szType->Add( m_Text );

    m_Binary = new wxRadioButton( m_Panel,
                                 ID_BINARY,
                                 wxT( "&Binary" ) );
    szType->Add( m_Binary );

    szTop->Add( szType, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL );

    // Top grouping is complete so add it to the base group
    szBase->Add( szTop, 0,
                wxALIGN_LEFT | wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 4 );

    // Create the subfile list
    m_List = new wxListView( m_Panel,
                            ID_LIST,
                            wxDefaultPosition,
                            wxSize( -1, 150 ),
                            wxLC_REPORT | wxLC_SORT_DESCENDING );

    // Add it to the base group
    szBase->Add( m_List, 1, wxEXPAND | wxTOP, 2 );

    // Tell the panel about the base sizer
    m_Panel->SetSizerAndFit( szBase );

    // Set initial sizes and establish minimums
    szBase->SetSizeHints( this );

    // Add the list view images
    m_Images       = new wxImageList( 15, 16, true );
    m_ImageUnknown = m_Images->Add( wxBitmap( unknown16_xpm ) );
    m_ImageText    = m_Images->Add( wxBitmap( text16_xpm ) );
    m_ImageBinary  = m_Images->Add( wxBitmap( binary16_xpm ) );
    m_ImageUp      = m_Images->Add( wxBitmap( uparrow16_xpm ) );
    m_ImageDown    = m_Images->Add( wxBitmap( downarrow16_xpm ) );

    m_List->SetImageList( m_Images, wxIMAGE_LIST_SMALL );

    // Set initial sort order
    m_Dir = -1;
    m_SortBy = Fn;
    memset( m_SortDir, m_Dir, sizeof( m_SortDir ) );

    // Define the list view columns
    m_List->InsertColumn( Fn,       wxT( "Fn" ),        wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Fn );

    m_List->InsertColumn( Ft,       wxT( "Ft" ),        wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Ft );

    m_List->InsertColumn( Fm,       wxT( "Fm" ),        wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Fm );

    m_List->InsertColumn( Date,     wxT( "Date" ),      wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Date );

    m_List->InsertColumn( Recfm,    wxT( "Recfm" ),     wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Recfm );

    m_List->InsertColumn( Lrecl,    wxT( "Lrecl" ),     wxLIST_FORMAT_RIGHT );
    m_List->ClearColumnImage( Lrecl );

    m_List->InsertColumn( Cbytes,   wxT( "Compressed" ),  wxLIST_FORMAT_RIGHT );
    m_List->ClearColumnImage( Cbytes );

    m_List->InsertColumn( Ubytes,   wxT( "Uncompressed" ), wxLIST_FORMAT_RIGHT );
    m_List->ClearColumnImage( Ubytes );

    m_List->InsertColumn( Verrel,   wxT( "V.R" ),       wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Verrel );

    m_List->InsertColumn( Method,   wxT( "Method" ),    wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Method );

    m_List->InsertColumn( Dtype,    wxT( "Type" ),      wxLIST_FORMAT_LEFT  );
    m_List->ClearColumnImage( Dtype );

    // Define accelerators
    wxAcceleratorEntry ae[ 2 ];

    ae[ 0 ].Set( wxACCEL_NORMAL, WXK_ESCAPE, wxID_EXIT );
    ae[ 1 ].Set( wxACCEL_CTRL,   (int) wxT( 'A' ),  ID_CTRL_A );

    wxAcceleratorTable at( 2, ae );

    SetAcceleratorTable( at );

    // Load preferences
    LoadSettings();

    // Process command line arguments
    if( wxTheApp->argc == 2 )
    {
        // Just open and display the hive contents
        LoadArchive( wxString( wxTheApp->argv[ 1 ] ) );
    }
    else if( wxTheApp->argc == 3 )
    {
        // Direct extraction?
        if( wxStrcmp( wxTheApp->argv[ 1 ], wxT( "/x" ) ) == 0 )
        {
            // Open and extract contents
            if( LoadArchive( wxString( wxTheApp->argv[ 2 ] ) ) )
            {
                wxCommandEvent event;
                OnExtAll( event );
            }

            // Display messages in box as we will be exiting
            wxString msg = m_Status->GetStatusText();
            if( !msg.IsEmpty() )
            {
                wxMessageBox( msg, wxT( "Helpful Message" ) );
            }

            // We're done so kill the dialog
            Destroy();

            return;
        }
    }

    Layout();
    CenterOnScreen();

    return;
}

// ====================================================================
//
// ====================================================================
wxButton *
MyFrame::AddButton( wxSizer *sz, int id, const char * const *image, const wxChar *tip )
{
    wxBitmap bm( image );
    wxBitmapButton *bb = new wxBitmapButton( m_Panel, id, bm, wxDefaultPosition, wxDefaultSize );
    bb->SetToolTip( tip );
    sz->Add( bb, 0, wxLEFT | wxRIGHT, 1 );

    return bb;
}

// ====================================================================
// Handle column header clicks
// ====================================================================
void MyFrame::OnColumnClick( wxListEvent& event )
{
    int SortBy = event.GetColumn();

    // Get rid of previous arrow, if any.
    m_List->ClearColumnImage( m_SortBy );

    // Determine direction
    if( m_SortBy == SortBy )
    {
        m_Dir = -m_Dir;
    }
    else
    {
        m_Dir = m_SortDir[ SortBy ];
        m_SortBy = static_cast<Field>( SortBy );
    }

    // Sort the items
    m_List->SortItems( Compare, ( m_SortBy + 1 ) * m_Dir );

    // And set the direction arrow
    m_SortDir[ m_SortBy ] = m_Dir;

#if !defined(__WXMAC__)
    m_List->SetColumnImage( m_SortBy,
                           m_Dir < 0 ? m_ImageDown : m_ImageUp );
#endif

    return;
}

// ====================================================================
// A list item was activated (double clicked or ENTER pressed)
// ====================================================================
void MyFrame::OnActivated( wxListEvent& event )
{

    // Simulate a left button click based on default action
    if( m_Action.Cmp( wxT( "View" ) ) == 0 )
    {
        OnView( event );
    }
    else
    {
        OnExtract( event );
    }

    return;
}

// ====================================================================
// Close our dialog when asked to quit
// ====================================================================
void MyFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
    Close();

    return;
}

// ====================================================================
// CTRL+A was pressed, so select everything for list view and edit box
// ====================================================================
void MyFrame::OnCtrlA( wxCommandEvent& WXUNUSED( event ) )
{
    int nItem = 0;

    // Get ID of currently focused control
    switch( FindFocus()->GetId() )
    {
            // It's the list view
        case ID_LIST:

            // Look through all items
            while( TRUE )
            {
                // Get the next item
                nItem = m_List->GetNextItem( nItem );
                if( nItem == -1 )
                {
                    break;
                }

                // And select it
                m_List->Select( nItem, true );
            }

            break;

            // Pattern edit box
        case ID_OUTPAT:

            // Select entire contents
            m_Pattern->SetSelection( -1, -1 );

            break;
    }

    return;
}

// ====================================================================
// Shutting down so save settings and destroy dialog
// ====================================================================
void MyFrame::OnClose( wxCloseEvent& event )
{
    if( event.CanVeto() && !SaveArchive() )
    {
        event.Veto( true );
        return;
    }

    // Close the archive
    CloseArchive();

    // Save the settings
    SaveSettings();

    // Destroy dialog
    Destroy();

    return;
}

// ====================================================================
// Open button clicked
// ====================================================================
void MyFrame::OnNew( wxCommandEvent& WXUNUSED( event ) )
{
    if( !SaveArchive() )
    {
        return;
    }

    // Close archive
    CloseArchive();

    return;
}

// ====================================================================
// Open button clicked
// ====================================================================
void MyFrame::OnOpen( wxCommandEvent& WXUNUSED( event ) )
{
    if( !SaveArchive() )
    {
        return;
    }

    // Close archive
    CloseArchive();

    // Go open an archive
    LoadArchive();

    return;
}

// ====================================================================
// Save button clicked
// ====================================================================
void MyFrame::OnSave( wxCommandEvent& WXUNUSED( event ) )
{
    // Go save the archive
    SaveArchive();

    return;
}

// ====================================================================
// Add button clicked
// ====================================================================
void MyFrame::OnAdd( wxCommandEvent& WXUNUSED( event ) )
{
    SUBFILE *sf;
    int rc;

    if( m_vma == NULL )
    {
        wxFileName fn;

        // Create an Open dialog
        wxFileDialog fd( this,
                        wxT( "Specify the output archive name" ),
                        m_OpenPath,
                        wxEmptyString,
                        wxT( "All Files|*.*" ),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

        // And Display it
        if( fd.ShowModal() != wxID_OK )
        {
            return;
        }

        m_Filename = fd.GetPath();

        if( !OpenArchive() )
        {
            return;
        }
    }

    // Create an Open dialog
    wxFileDialog fd( this,
                    wxT( "Select the files to add" ),
                    m_OpenPath,
                    wxEmptyString,
                    wxT( "All Files|*.*" ),
                    wxFD_OPEN | wxFD_MULTIPLE );

    // And Display it
    if( fd.ShowModal() != wxID_OK )
    {
        return;
    }

    wxArrayString paths;
    fd.GetPaths( paths );

    if( paths.Count() == 0 )
    {
        return;
    }

    rc = vma_setmode( m_vma, m_Format );
    if( rc != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( "Unable to set conversion mode - rc: %d, %s\n" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

        return;
    }

    for( size_t i = 0; i < paths.Count(); i++ )
    {
        wxFileName name( paths[ i ] );
        rc = vma_new( m_vma, &sf );
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Creating new subfile failed - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

            SetStatusText( m_Msg );

            break;
        }

        Properties props( this, m_vma );
        if( !props.SetProperties( sf, name.GetFullPath() ) )
        {
            vma_delete( m_vma );
            break;
        }

        rc = vma_add( m_vma, name.GetFullPath().mb_str() );
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Unable to add %s - rc: %d, %s\n" ),
                         name.GetFullName().c_str(),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

            SetStatusText( m_Msg );

            vma_delete( m_vma );

            break;
        }
    }

    ReadArchive();

    return;
}

// ====================================================================
// Delete button clicked
// ====================================================================
void MyFrame::OnDelete( wxCommandEvent& WXUNUSED( event ) )
{
    SUBFILE *sf;
    wxFileName fn;
    wxString dest;
    int nItem;
    int cnt = m_List->GetSelectedItemCount();

    // Can't do anything without entries
    if( m_List->GetItemCount() == 0 )
    {
        wxMessageBox( wxT( "Might be a good idea if you opened a VMARC " )
                     wxT( "before trying to delete anything." ),
                     wxT( "Here's you sign..." ),
                     wxICON_EXCLAMATION );

        return;
    }

    // Can't do anything without selections
    if( cnt == 0 )
    {
        wxMessageBox( wxT( "I'm just thinkin' here, but maybe, just maybe, " )
                     wxT( "selecting something might work a little better?" ),
                     wxT( "Here's your sign..." ) );

        return;
    }


    if( wxMessageBox( wxT( "Are you sure you want to delete the selected subfiles?" ),
                     wxT( "Delete subfiles..." ),
                     wxYES_NO ) == wxNO )
    {
        return;
    }

    // Retrieve the first selected item
    nItem = -1;
    while( ( nItem = m_List->GetFirstSelected() ) != -1 )
    {
        int rc;

        // And the SUBFILE ptr
        sf = (SUBFILE *) wxUIntToPtr( m_List->GetItemData( nItem ) );

        // Make this the active subfile
        vma_setactive( m_vma, sf );

        // Delete it
        rc = vma_delete( m_vma );

        // Complain if there was an error
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( " Deletion of %s.%s.%s failed with %d, %s" ),
                         ToWX( sf->fn ).c_str(),
                         ToWX( sf->ft ).c_str(),
                         ToWX( sf->fm ).c_str(),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

            SetStatusText( m_Msg );

            return;
        }

        // Refresh stats for this entry since they are now available
        m_List->DeleteItem( nItem );
    }

    // Provide a little info
    m_Msg.Printf( wxT( " %d subfiles deleted" ), cnt );
    SetStatusText( m_Msg );

    return;
}

// ====================================================================
// Extract button clicked
// ====================================================================
void MyFrame::OnExtract( wxCommandEvent& WXUNUSED( event ) )
{
    SUBFILE *sf;
    wxFileName fn;
    wxString dest;
    int nItem;
    int cnt = m_List->GetSelectedItemCount();

    // Can't do anything without entries
    if( m_List->GetItemCount() == 0 )
    {
        wxMessageBox( wxT( "Might be a good idea if you opened a VMARC " )
                     wxT( "before trying to extract anything." ),
                     wxT( "Here's you sign..." ),
                     wxICON_EXCLAMATION );

        return;
    }

    // Can't do anything without selections
    if( cnt == 0 )
    {
        wxMessageBox( wxT( "I'm just thinkin' here, but maybe, just maybe, " )
                     wxT( "selecting something might work a little better?" ),
                     wxT( "Here's your sign..." ) );

        return;
    }

    // Only one file selected...handle it
    if( cnt == 1 )
    {
        // Get the first (only) selected item
        nItem = m_List->GetFirstSelected();

        // And the SUBFILE ptr
        sf = (SUBFILE *) wxUIntToPtr( m_List->GetItemData( nItem ) );

        // Generate an output name and append to previous folder path
        fn = m_SavePath;
        fn.AppendDir( OutputName( sf ) );

        // Create an Open dialog
        wxFileDialog fd( this,
                        wxT( "Specify the output file name" ),
                        m_SavePath,
                        OutputName( sf ),
                        wxT( "All Files|*.*" ),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

        // And Display it
        if( fd.ShowModal() != wxID_OK )
        {
            return;
        }

        // Retrieve the (possibly updated) file name
        fn = fd.GetPath();

        // Remember the folder part
        m_SavePath = fn.GetPath();

        // Extract it
        Extract( sf, fn.GetFullPath(), nItem );

        return;
    }

    // Create the "Browse for Folder" dialog
    wxDirDialog dd( this,
                   wxT( "Extract to..." ),
                   m_SavePath,
                   wxDD_DEFAULT_STYLE );

    // And display it
    if( dd.ShowModal() != wxID_OK )
    {
        return;
    }

    // Remember the path
    m_SavePath = dd.GetPath();

    // Keep this as a separate block for the wxBusyInfo
    {
        wxBusyInfo wait( wxT( "Extracting files..." ) );

        // Retrieve the first selected item
        cnt = 0;
        nItem = -1;
        while( TRUE )
        {
            nItem = m_List->GetNextSelected( nItem );
            if( nItem == -1 )
            {
                break;
            }

            // And the SUBFILE ptr
            sf = (SUBFILE *) wxUIntToPtr( m_List->GetItemData( nItem ) );

            // Generate an output name and append to previous folder path
            fn.Assign( m_SavePath, OutputName( sf ) );

            // Extract it
            if( !Extract( sf, fn.GetFullPath(), nItem ) )
            {
                break;
            }

            cnt++;
        }
    }

    // Provide a little info
    m_Msg.Printf( wxT( " %d subfiles extracted" ), cnt );
    SetStatusText( m_Msg );

    return;
}

// ====================================================================
// Extract All button clicked
// ====================================================================
void MyFrame::OnExtAll( wxCommandEvent& WXUNUSED( event ) )
{
    SUBFILE *sf;
    wxFileName fn;
    wxString dest;
    int nItem;
    int cnt;

    // Can't do anything without entries
    if( m_List->GetItemCount() == 0 )
    {
        wxMessageBox( wxT( "Might be a good idea if you opened a VMARC " )
                     wxT( "before trying to extract anything." ),
                     wxT( "Here's you sign..." ),
                     wxICON_EXCLAMATION );

        return;
    }

    // Create the "Browse for Folder" dialog
    wxDirDialog dd( this,
                   wxT( "Extract to..." ),
                   m_SavePath,
                   wxDD_DEFAULT_STYLE );

    // And display it
    if( dd.ShowModal() != wxID_OK )
    {
        return;
    }

    // Remember the path
    m_SavePath = dd.GetPath();

    // Keep this as a separate block for the wxBusyInfo
    {
        wxBusyInfo wait( wxT( "Extracting files..." ) );

        // Retrieve the first selected item
        cnt = 0;
        nItem = -1;
        while( TRUE )
        {
            nItem = m_List->GetNextItem( nItem );
            if( nItem == -1 )
            {
                break;
            }

            // And the SUBFILE ptr
            sf = (SUBFILE *) wxUIntToPtr( m_List->GetItemData( nItem ) );

            // Generate an output name and append to previous folder path
            fn.Assign( m_SavePath, OutputName( sf ) );

            // Extract it
            if( !Extract( sf, fn.GetFullPath(), nItem ) )
            {
                break;
            }

            cnt++;
        }
    }

    // Provide a little info
    m_Msg.Printf( wxT( " %d subfiles extracted" ), cnt );
    SetStatusText( m_Msg );

    return;
}

// ====================================================================
// View button clicked
// ====================================================================
void MyFrame::OnView( wxCommandEvent& WXUNUSED( event ) )
{
    SUBFILE *sf;
    wxFileName fn;
    wxString dest;
    wxString cmd;
    int nItem;
    int cnt = m_List->GetSelectedItemCount();

    // Can't do anything without entries
    if( m_List->GetItemCount() == 0 )
    {
        wxMessageBox( wxT( "Might be a good idea if you opened a VMARC " )
                     wxT( "before trying to view anything." ),
                     wxT( "Here's you sign..." ),
                     wxICON_EXCLAMATION );

        return;
    }

    // Can't do anything without selections
    if( cnt == 0 )
    {
        wxMessageBox( wxT( "I'm just thinkin' here, but maybe, just maybe, " )
                     wxT( "selecting something might work a little better?" ),
                     wxT( "Here's your sign..." ) );

        return;
    }

    // Limit to one file at a time
    if( cnt > 1 )
    {
        wxMessageBox( wxT( "I could show you more than 1 file, but I don't feel like it." ),
                     wxT( "Helpful Message" ) );
        return;
    }

    // Get the first (only) selected item
    nItem = m_List->GetFirstSelected();

    // And the SUBFILE ptr
    sf = (SUBFILE *) wxUIntToPtr( m_List->GetItemData( nItem ) );

    // Generate an output name and append to previous folder path
    dest = wxFileName::CreateTempFileName( wxT( "VMA" ) );

    // Extract it
    if( !Extract( sf, dest, nItem ) )
    {
        return;
    }

    // Determine how to view the file
    cmd = m_BinViewer;
    if( m_Text->GetValue() || ( m_Auto->GetValue() && sf->dtype == VMAD_TEXT ) )
    {
        cmd = m_TxtViewer;
    }

    // Invoke the viewer app
    cmd += wxT( " " ) + dest;
    wxProcess *process = new MyProcess( dest );
    if( wxExecute( cmd, wxEXEC_ASYNC | wxEXEC_NOHIDE, process ) == 0 )
    {
        wxLogError( wxT( "Execution of '%s' failed." ), cmd.c_str() );

        delete process;
    }

    return;
}

// ====================================================================
// Properties button clicked
// ====================================================================
void MyFrame::OnProperties( wxCommandEvent& WXUNUSED( event ) )
{
    SUBFILE *sf;
    int nItem;

    // Can't do anything without entries
    if( m_List->GetItemCount() == 0 )
    {
        wxMessageBox( wxT( "Might be a good idea if you opened a VMARC " )
                     wxT( "before trying to set properties." ),
                     wxT( "Here's you sign..." ),
                     wxICON_EXCLAMATION );

        return;
    }

    // Can't do anything without selections
    if( m_List->GetSelectedItemCount() == 0 )
    {
        wxMessageBox( wxT( "I'm just thinkin' here, but maybe, just maybe, " )
                     wxT( "selecting something might work a little better?" ),
                     wxT( "Here's your sign..." ) );

        return;
    }

    // Retrieve the first selected item
    for( nItem = m_List->GetFirstSelected();
        nItem != wxNOT_FOUND;
        nItem = m_List->GetNextSelected( nItem ) )
    {
        // And the SUBFILE ptr
        sf = (SUBFILE *) wxUIntToPtr( m_List->GetItemData( nItem ) );

        // Make this the active subfile
        vma_setactive( m_vma, sf );

        // Display the properties dialog
        Properties props( this, m_vma );
        if( !props.SetProperties( sf ) )
        {
            break;
        }
    }

    ReadArchive();

    return;
}

// ====================================================================
// Settings button clicked
// ====================================================================
void MyFrame::OnSettings( wxCommandEvent& WXUNUSED( event ) )
{
    Settings settings( this );
    wxString exts;
    wxString ext;
    int rc;

    // Display Settings dialog
    settings.ShowModal();

    // Refresh (possibly changed) settings
    m_Action = m_Config->Read( wxT( "Default Action" ), wxT( "View" ) );

#if defined(__WXMAC__)
    m_TxtViewer = m_Config->Read( wxT( "Text Viewer" ), wxT( "/Applications/TextEdit.app" ) );
    m_BinViewer = m_Config->Read( wxT( "Binary Viewer" ), wxT( "/Applications/TextEdit.app" ) );
#elif defined(__WXMSW__)
    m_TxtViewer = m_Config->Read( wxT( "Text Viewer" ), wxT( "Notepad" ) );
    m_BinViewer = m_Config->Read( wxT( "Binary Viewer" ), wxT( "Notepad" ) );
#else
    m_TxtViewer = m_Config->Read( wxT( "Text Viewer" ), wxT( "gedit" ) );
    m_BinViewer = m_Config->Read( wxT( "Binary Viewer" ), wxT( "gedit" ) );
#endif

    m_FromUCM = m_Config->Read( wxT( "From UCM" ), wxT( "" ) );

    m_ToUCM = m_Config->Read( wxT( "To UCM" ), wxT( "" ) );

    exts = m_Config->Read( wxT( "Extensions" ), wxT( ".vma;.vmarc" ) );

    // Rebuild Open dialog filter
    m_Filter.Empty();

    wxStringTokenizer st( exts, wxT( ";" ) );

    // Make sure extension have proper format
    while( st.HasMoreTokens() )
    {
        ext = st.GetNextToken().Strip( wxString::both );
        if( ext.Length() == 0 )
        {
            continue;
        }
        if( ext[ 0 ] == wxT( '*' ) )
        {
            ext.Remove( 0, 1 );
        }

        if( ext[ 0 ] == wxT( '.' ) )
        {
            ext.Remove( 0, 1 );
        }

        m_Filter += wxT( "*." ) + ext + wxT( ';' );
    }

    // Complete filter if extensions provided
    if( !m_Filter.IsEmpty() )
    {
        m_Filter = wxT( "VMARC Hives|" ) + m_Filter + wxT( '|' );
    }

    // Reset the conversion tables
    if( m_vma )
    {
        // Both UCMs must be specified to
        if( !m_FromUCM.IsEmpty() && !m_ToUCM.IsEmpty() )
        {
            rc = vma_setconv( m_vma,
                             m_FromUCM.mb_str(),
                             m_ToUCM.mb_str() );

            if( rc != VMAE_NOERR )
            {
                exts.Printf( wxT( "Conversion table load failed: %d, %s\n" ),
                            rc,
                            ToWX( vma_strerror( rc ) ).c_str() );

                wxMessageBox( exts, wxT( "Not So Helpful Message" ) );
            }
        }
        else
        {
            // Reset to internal conversion
            vma_setconv( m_vma, NULL, NULL );
        }
    }

    return;
}

// ====================================================================
// Settings button clicked
// ====================================================================
void MyFrame::OnMode( wxCommandEvent& WXUNUSED( event ) )
{
    int rc;

    m_Format = VMAX_AUTO;

    if( m_Text->GetValue() )
    {
        m_Format = VMAX_TEXT;
    }
    else if( m_Binary->GetValue() )
    {
        m_Format = VMAX_BINARY;
    }

    m_Config->Write( wxT( "/Settings/Format" ),
                    m_Format );

    rc = vma_setmode( m_vma, m_Format );
    if( rc != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( "Unable to set conversion mode - rc: %d, %s\n" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

        return;
    }

    return;
}

// ====================================================================
// MyFrame private methods
// ====================================================================

// ====================================================================
// Compare function for column sorting
// ====================================================================
int wxCALLBACK
MyFrame::Compare( long item1,
                 long item2,
                 long sortData )
{
    SUBFILE *sf1 = reinterpret_cast<SUBFILE *>(item1);
    SUBFILE *sf2 = reinterpret_cast<SUBFILE *>(item2);
    bool    dir = false;
    int     result = 0;

    // Descending?
    if( sortData < 0 )
    {
        sortData = -sortData;
        dir = true;
    }

    // Compare entries based on column
    switch( sortData - 1 )
    {
        case Fn:
            result = COMPARE( (char *)sf1->fn, (char *)sf2->fn );
            break;

        case Ft:
            result = COMPARE( (char *)sf1->ft, (char *)sf2->ft );
            break;

        case Fm:
            result = COMPARE( (char *)sf1->fm, (char *)sf2->fm );
            break;

        case Date:
        {
            struct tm bt;
            time_t ct1;
            time_t ct2;

            // Convert time of first entry
            bt.tm_sec = sf1->second;
            bt.tm_min = sf1->minute;
            bt.tm_hour = sf1->hour;
            bt.tm_mday = sf1->day;
            bt.tm_mon = sf1->month;
            bt.tm_year = sf1->year - 1900;
            bt.tm_wday = 0;
            bt.tm_yday = 0;
            bt.tm_isdst = 0;

            ct1 = mktime( &bt );

            // Convert time of second entry
            bt.tm_sec = sf2->second;
            bt.tm_min = sf2->minute;
            bt.tm_hour = sf2->hour;
            bt.tm_mday = sf2->day;
            bt.tm_mon = sf2->month;
            bt.tm_year = sf2->year - 1900;
            bt.tm_wday = 0;
            bt.tm_yday = 0;
            bt.tm_isdst = 0;

            ct2 = mktime( &bt );

            result = static_cast<int>(ct1 - ct2);
        }
            break;

        case Recfm:
            result = sf1->recfm - sf2->recfm;
            break;

        case Lrecl:
            result = sf1->lrecl - sf2->lrecl;
            break;

        case Cbytes:
            result = (int) ( sf1->compressed - sf2->compressed );
            break;

        case Ubytes:
            result = (int) ( sf1->uncompressed - sf2->uncompressed );
            break;

        case Verrel:
            result =  ( ( sf1->ver << 8 ) + sf1->rel ) -
            ( ( sf2->ver << 8 ) + sf2->rel );
            break;

        case Method:
            result = COMPARE( (char *)sf1->meth, (char *)sf2->meth );
            break;

        case Dtype:
            result = COMPARE( (char *)sf1->meth, (char *)sf2->meth );
            break;
    }

    return dir ? -result : result;
}

// ====================================================================
// Format function for list values
// ====================================================================
wxString MyFrame::Format( SUBFILE *sf, int nField )
{
    wxString szResult;

    // Populate szResult based on column
    switch( nField )
    {
        case Fn:
            szResult = ToWX( sf->fn );
            break;

        case Ft:
            szResult = ToWX( sf->ft );
            break;

        case Fm:
            szResult = ToWX( sf->fm );
            break;

        case Date:
            szResult.Printf( wxT( "%04d/%02d/%02d %02d:%02d:%02d" ),
                            sf->year,
                            sf->month,
                            sf->day,
                            sf->hour,
                            sf->minute,
                            sf->second );
            break;

        case Recfm:
            szResult.Printf( wxT( "%c" ), sf->recfm );
            break;

        case Lrecl:
            szResult.Printf( wxT( "%d" ), sf->lrecl );
            break;

        case Cbytes:
            szResult.Printf( wxT( "%d" ), sf->compressed );
            break;

        case Ubytes:
            szResult.Printf( wxT( "%d" ), sf->uncompressed );
            break;

        case Verrel:
            szResult.Printf( wxT( "%d.%d" ), sf->ver, sf->rel );
            break;

        case Method:
            szResult = ToWX( sf->meth );
            break;

        case Dtype:
            switch( sf->dtype )
        {
            case VMAD_UNKNOWN:
                szResult = wxT( "Unknown" );
                break;

            case VMAD_TEXT:
                szResult = wxT( "Text" );
                break;

            case VMAD_BINARY:
                szResult = wxT( "Binary" );
                break;
        }
            break;
    }

    return szResult;
}

// ====================================================================
// Auto size columns base on largest of header and entry contents
// ====================================================================
void MyFrame::AutoSizeColumns( void )
{
    int numcols = m_List->GetColumnCount();
    int col;
    int cw1;
    int cw2;

    // Turn off display updating
    m_List->Freeze();

    // Process all columns
    for( col = 0; col < numcols; col++ )
    {
        // Set width based on entry contents
        m_List->SetColumnWidth( col, wxLIST_AUTOSIZE );

        // Retrieve calculated width
        int height;
        m_Images->GetSize( 0, cw1, height );
        cw1 += m_List->GetColumnWidth( col );

        // Set width base on header contents
#if defined(__WXMAC__)
        wxListItem info;
        m_List->GetColumn( col, info );

        const wxFont font = info.GetFont();

        int w = 0;
        if( font.IsOk() )
        {
            m_List->GetTextExtent( info.GetText(), &w, NULL, NULL, NULL, &font );
        }
        else
        {
            m_List->GetTextExtent( info.GetText(), &w, NULL );
        }

        // 8 = left and right padding, 16 = sort icon size
        cw2 = w + 8 + 16;
#else
        m_List->SetColumnWidth( col, wxLIST_AUTOSIZE_USEHEADER );

        // Retrieve calculated width
        cw2 = m_List->GetColumnWidth( col );
#endif

        // Set final width using maximum of the 2 (40 pixel minimum)
        m_List->SetColumnWidth( col, wxMax( 40, wxMax( cw1, cw2 ) ) );
    }

    // Re-enable display updates
    m_List->Thaw();

    return;
}

// ====================================================================
// Generate output file name using pattern
// ====================================================================
wxString MyFrame::OutputName( SUBFILE *sf )
{
    wxString pat;
    wxString name;
    wxString temp;
    const wxChar *p;

    // Get current pattern
    pat = m_Pattern->GetValue();

    // Build name while examining pattern
    p = pat;
    while( *p )
    {
        // Conversion?
        if( *p == wxT( '%' ) )
        {
            // Is it one we understand?
            p++;
            switch( *p )
            {
                    // "%" as end of string
                case wxT( '\0' ):
                    name += wxT( '%' );
                    break;

                    // Append lowercase file name
                case wxT( 'n' ):
                    temp = ToWX( sf->fn );
                    temp.MakeLower();
                    name += temp;
                    break;

                    // Append uppercase file name
                case wxT( 'N' ):
                    temp = ToWX( sf->fn );
                    temp.MakeUpper();
                    name += temp;
                    break;

                    // Append lowercase file type
                case wxT( 't' ):
                    temp = ToWX( sf->ft );
                    temp.MakeLower();
                    name += temp;
                    break;

                    // Append uppercase file type
                case wxT( 'T' ):
                    temp = ToWX( sf->ft );
                    temp.MakeUpper();
                    name += temp;
                    break;

                    // Append lowercase file mode
                case wxT( 'm' ):
                    temp = ToWX( sf->fm );
                    temp.MakeLower();
                    name += temp;
                    break;

                    // Append uppercase file mode
                case wxT( 'M' ):
                    temp = ToWX( sf->fm );
                    temp.MakeUpper();
                    name += temp;
                    break;

                    // Just pass through unknowns
                default:
                    name += wxT( '%') ;
                    name += *p;
                    break;
            }
        }
        else
        {
            // Append literal character
            name += *p;
        }

        p++;
    }

    return name;
}

// ====================================================================
// Extract subfile
// ====================================================================
bool MyFrame::Extract( SUBFILE *sf, wxString dest, int nItem )
{
    int rc;

    // Users don't like sitting in the dark
    m_Msg.Printf( wxT( " Extracting %s.%s.%s" ),
                 ToWX( sf->fn ).c_str(),
                 ToWX( sf->ft ).c_str(),
                 ToWX( sf->fm ).c_str() );
    SetStatusText( m_Msg );

    // Make this the active subfile
    vma_setactive( m_vma, sf );

    // Extract it
    rc = vma_extract( m_vma, dest.mb_str() );

    // Complain if there was an error
    if( rc != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( " Extraction of %s.%s.%s failed with %d, %s" ),
                     ToWX( sf->fn ).c_str(),
                     ToWX( sf->ft ).c_str(),
                     ToWX( sf->fm ).c_str(),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

        SetStatusText( m_Msg );

        return false;
    }

    // Refresh stats for this entry since they are now available
    m_List->SetItem( nItem, Cbytes, Format( sf, Cbytes ) );
    m_List->SetItem( nItem, Ubytes, Format( sf, Ubytes ) );
    m_List->SetItem( nItem, Dtype,  Format( sf, Dtype  ) );
    m_List->SetItemImage( nItem, sf->dtype );

    // Unselect the list item
    m_List->SetItemState( nItem, 0, wxLIST_STATE_SELECTED );

    // A silly little message
    m_Msg.Printf( wxT( " %s.%s.%s extracted" ),
                 ToWX( sf->fn ).c_str(),
                 ToWX( sf->ft ).c_str(),
                 ToWX( sf->fm ).c_str() );
    SetStatusText( m_Msg );

    return true;
}

// ====================================================================
// Load an archive
// ====================================================================
bool MyFrame::LoadArchive( wxString name )
{
    // Ask user for filename if one wasn't specified
    if( name.IsEmpty() )
    {
        wxFileDialog fd( this,
                        wxT( "Choose a VMARC Hive" ),
                        m_OpenPath,
                        wxT( "" ),
                        m_Filter + wxT( "All Files|*.*" ) );

        if( fd.ShowModal() != wxID_OK )
        {
            return false;
        }

        m_Filename = fd.GetPath();
    }
    else
    {
        m_Filename = name;
    }

    if( OpenArchive() )
    {
        if( !ReadArchive() )
        {
            CloseArchive();
        }
    }

    return true;
}

// ====================================================================
// Open an archive
// ====================================================================
bool MyFrame::OpenArchive()
{
    wxFileName fn;
    int rc;

    // Close previous archive (if any)
    if( m_vma )
    {
        CloseArchive();
    }

    // Get rid of any previous list items
    m_List->DeleteAllItems();

    // Normalize path and extract components
    fn = m_Filename;
    fn.Normalize();
    m_Filename = fn.GetFullPath();
    m_OpenPath = fn.GetPath();

    // Keep this as a separate block for the wxBusyInfo
    {
        wxBusyInfo wait( wxT( "Opening VMARC Hive..." ) );

        // Open the archive
        rc = vma_open( m_Filename.mb_str(), &m_vma );
        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Open failed - rc: %d, %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

            return false;
        }

        // Set the window title
        m_Msg.Printf( wxT( "VMAgui - %s" ), m_Filename.c_str() );
        SetTitle( m_Msg );
    }

    // Set the conversion tables
    if( !m_FromUCM.IsEmpty() && !m_ToUCM.IsEmpty() )
    {
        rc = vma_setconv( m_vma,
                         m_FromUCM.mb_str(),
                         m_ToUCM.mb_str() );

        if( rc != VMAE_NOERR )
        {
            m_Msg.Printf( wxT( "Conversion table load failed: %d - %s\n" ),
                         rc,
                         ToWX( vma_strerror( rc ) ).c_str() );

            wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

            CloseArchive();

            return false;
        }
    }
    else
    {
        // Reset to internal conversion
        vma_setconv( m_vma, NULL, NULL );
    }

    // Determine the conversion mode
    int mode = VMAX_AUTO;
    if( m_Text->GetValue() )
    {
        mode = VMAX_TEXT;
    }
    else if( m_Binary->GetValue() )
    {
        mode = VMAX_BINARY;
    }

    return true;
}

// ====================================================================
// Close the archive
// ====================================================================
void MyFrame::CloseArchive()
{
    if( !m_vma )
    {
        return;
    }

    // Close archive
    vma_close( m_vma );
    m_vma = NULL;

    // Reset the file name
    m_Filename.Clear();

    // Get rid of any previous list items
    m_List->DeleteAllItems();

    // Finally, set the window title
    SetTitle( wxT( TITLE ) );
}

// ====================================================================
// Read the archive
// ====================================================================
bool MyFrame::ReadArchive()
{
    SUBFILE *sf;
    int rc;

    wxBusyInfo wait( wxT( "Reading VMARC hive..." ) );

    // Get rid of any previous list items
    m_List->DeleteAllItems();

    // Add each subfile to the list
    for( rc = vma_first( m_vma, &sf );
        rc == VMAE_NOERR;
        rc = vma_next( m_vma, &sf ) )
    {
        int nIndex;
        wxListItem li;

        li.Clear();
        li.SetId( m_List->GetItemCount() );
        li.SetText( ToWX( sf->fn ) );
        li.SetData( sf );
        li.SetImage( sf->dtype );

        nIndex = m_List->InsertItem( li );
        if( nIndex >= 0 )
        {
            m_List->SetItemData( nIndex, wxPtrToUInt( sf ) );

            m_List->SetItem( nIndex, Ft,     Format( sf, Ft     ) );
            m_List->SetItem( nIndex, Fm,     Format( sf, Fm     ) );
            m_List->SetItem( nIndex, Date,   Format( sf, Date   ) );
            m_List->SetItem( nIndex, Recfm,  Format( sf, Recfm  ) );
            m_List->SetItem( nIndex, Lrecl,  Format( sf, Lrecl  ) );
            m_List->SetItem( nIndex, Cbytes, Format( sf, Cbytes ) );
            m_List->SetItem( nIndex, Ubytes, Format( sf, Ubytes ) );
            m_List->SetItem( nIndex, Verrel, Format( sf, Verrel ) );
            m_List->SetItem( nIndex, Method, Format( sf, Method ) );
            m_List->SetItem( nIndex, Dtype,  Format( sf, Dtype  ) );
        }
    }

    //
    // Check final status
    //
    if( rc != VMAE_NOMORE )
    {
        m_Msg.Printf( wxT( "Subfile processing failed - rc: %d, %s\n" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

        CloseArchive();

        return false;
    }

    m_List->SortItems( Compare, ( m_SortBy + 1 ) * m_Dir );
    AutoSizeColumns();

    return true;
}

// ====================================================================
// Save changes
// ====================================================================
bool MyFrame::SaveArchive()
{
    int dirty;

    if( m_vma == NULL )
    {
        return true;
    }

    vma_isdirty( m_vma, &dirty );

    if( !dirty )
    {
        return true;
    }

    int ans;
    ans = wxMessageBox( wxT( "Do you want to commit the changes?" ),
                       wxT( "Archive changed..." ),
                       wxYES_NO | wxCANCEL );
    if( ans == wxNO )
    {
        return true;
    }
    else if( ans == wxCANCEL )
    {
        return false;
    }

    int rc = vma_commit( m_vma );
    if( rc != VMAE_NOERR )
    {
        m_Msg.Printf( wxT( " Committing changes failed with %d, %s" ),
                     rc,
                     ToWX( vma_strerror( rc ) ).c_str() );

        wxMessageBox( m_Msg, wxT( "Not So Helpful Message" ) );

        SetStatusText( m_Msg );

        return false;
    }

    SetStatusText( wxT( " Changes cmmitted" ) );

    return true;
}

// ====================================================================
// Save profile settings
// ====================================================================
void MyFrame::SaveSettings( void )
{
    wxRect rect;

    // Set active subkey
    m_Config->SetPath( wxT( "/Settings" ) );

    // Write all the settings
    m_Config->Write( wxT( "Open Path" ),
                    m_OpenPath );

    m_Config->Write( wxT( "Save Path" ),
                    m_SavePath );

    m_Format = VMAX_AUTO;
    if( m_Text->GetValue() )
    {
        m_Format = VMAX_TEXT;
    }
    else if( m_Binary->GetValue() )
    {
        m_Format = VMAX_BINARY;
    }

    m_Config->Write( wxT( "Format" ),
                    m_Format );

    m_Config->Write( wxT( "Pattern" ),
                    m_Pattern->GetValue() );

    m_Config->Write( wxT( "Maximized" ),
                    IsMaximized() );

    // Must "restore" window to get actual dimensions
    if( IsMaximized() )
    {
        // Prevent visual updating
        Freeze();

        // "Restore" window
        Iconize( false );

        // Get dimensions
        rect = GetRect();

        // Put it back the way it was
        Maximize( true );

        // Allow visual updating
        Thaw();
    }
    else
    {
        // Get Dimensions
        rect = GetRect();
    }

    // Write the dimensions
    m_Config->Write( wxT( "Window X" ),
                    rect.GetX() );
    m_Config->Write( wxT( "Window Y" ),
                    rect.GetY() );
    m_Config->Write( wxT( "Window W" ),
                    rect.GetWidth() );
    m_Config->Write( wxT( "Window H" ),
                    rect.GetHeight() );

    return;
}

// ====================================================================
// Load profile settings
// ====================================================================
void MyFrame::LoadSettings( void )
{
    wxString str;
    wxString exts;
    wxString ext;

    // Set active subkey
    m_Config->SetPath( wxT( "/Settings" ) );

    // Display a little first time info
    if( m_Config->Read( wxT( "First Time" ), 1 ) == 1 )
    {
#if defined( _WIN32 )
        wxMessageBox( wxT( "You may want to use the Settings dialog to select the file\n" )
                     wxT( "extensions you'd like to have associated with VMAgui." ),
                     wxT( "Welcome to VMAgui" ) );
#else
        wxMessageBox( wxT( "You may want to setup your environment to automatically\n"
                          "execute VMAgui when attempting to launch VMARC hives." ),
                     wxT( "Welcome to VMAgui" ) );
#endif
        m_Config->Write( wxT( "First Time" ), 0l );
    }

    // Read all the settings
    m_Format = m_Config->Read( wxT( "Format" ), 0l );
    if( m_Format == VMAX_TEXT )
    {
        m_Text->SetValue( true );
    }
    else if( m_Format == VMAX_BINARY )
    {
        m_Binary->SetValue( true );
    }
    else
    {
        m_Auto->SetValue( true );
    }

    str = m_Config->Read( wxT( "Pattern" ), wxT( "%n.%t.%m" ) );
    m_Pattern->SetValue( str );

    m_OpenPath = m_Config->Read( wxT( "Open Path" ), wxT( "." ) );

    m_SavePath = m_Config->Read( wxT( "Save Path" ), wxT( "." ) );

    m_Action = m_Config->Read( wxT( "Default Action" ), wxT( "View" ) );

#if defined(__WXMAC__)
    m_TxtViewer = m_Config->Read( wxT( "Text Viewer" ), wxT( "/Applications/TextEdit.app" ) );
    m_BinViewer = m_Config->Read( wxT( "Binary Viewer" ), wxT( "/Applications/TextEdit.app" ) );
#elif defined(__WXMSW__)
    m_TxtViewer = m_Config->Read( wxT( "Text Viewer" ), wxT( "Notepad" ) );
    m_BinViewer = m_Config->Read( wxT( "Binary Viewer" ), wxT( "Notepad" ) );
#else
    m_TxtViewer = m_Config->Read( wxT( "Text Viewer" ), wxT( "gedit" ) );
    m_BinViewer = m_Config->Read( wxT( "Binary Viewer" ), wxT( "gedit" ) );
#endif

    m_FromUCM = m_Config->Read( wxT( "From UCM" ), wxT( "" ) );

    m_ToUCM = m_Config->Read( wxT( "To UCM" ), wxT( "" ) );

    SetSize( m_Config->Read( wxT( "Window X" ), -1l ),
            m_Config->Read( wxT( "Window Y" ), -1l ),
            m_Config->Read( wxT( "Window W" ), -1l ),
            m_Config->Read( wxT( "Window H" ), -1l ) );

    Maximize( ( m_Config->Read( wxT( "Maximized" ), 0l ) != 0 ) );

    exts = m_Config->Read( wxT( "Extensions" ), wxT( ".vma;.vmarc" ) );

    // Rebuild Open dialog filter
    m_Filter.Empty();

    wxStringTokenizer st( exts, wxT( ";" ) );

    // Make sure extension have proper format
    while( st.HasMoreTokens() )
    {
        ext = st.GetNextToken().Strip( wxString::both );
        if( ext.Length() == 0 )
        {
            continue;
        }
        if( ext[ 0 ] == wxT( '*' ) )
        {
            ext.Remove( 0, 1 );
        }

        if( ext[ 0 ] == wxT( '.' ) )
        {
            ext.Remove( 0, 1 );
        }

        m_Filter += wxT( "*." ) + ext + wxT( ';' );
    }

    // Complete filter if extensions provided
    if( !m_Filter.IsEmpty() )
    {
        m_Filter = wxT( "VMARC Hives|" ) + m_Filter + wxT( '|' );
    }

    return;
}
