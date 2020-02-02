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

#define _ALL_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined( linux ) || defined( __APPLE__ ) || defined(__MVS__)
#include <utime.h>
#else
#include <sys/utime.h>
#endif

#if defined(__MVS__)
#include <sys/utsname.h>
#endif
 
#if !defined( _WIN32 )
#include <unistd.h>
#endif

#if defined( _WIN32 )
#include <io.h>
#define fseek _fseeki64
#endif

#include "vmalib.h"
#include "vmapriv.h"
#include "version.h"

/* --------------------------------------------------------------------
|| General stuff
*/
#if !defined( VERSION )
#define VERSION "\"homegrown\""
#endif

#if !defined( NULL )
#define NULL 0
#endif

#if !defined( TRUE )
#define TRUE 1
#endif

#if !defined( FALSE )
#define FALSE 0
#endif

#if !defined( O_BINARY )
#define O_BINARY 0
#endif

#if defined( _WIN32 )
#define PERMS S_IREAD | S_IWRITE
#else
#define PERMS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#endif

/* --------------------------------------------------------------------
|| EBCDIC and ASCII header IDs
*/
static const uchar hid[ 9 ] = "\x7a\xc3\xc6\xc6\x40\x40\x40\x40";
static const uchar aid[ 9 ] = ":CFF    ";

/* --------------------------------------------------------------------
|| Message text
*/
static const char *
etext[] =
{
    "No error",
    "read error",
    "write error",
    "Internal error - output buffer overflow",
    "caller error - no active subfile",
    "failed to open input file",
    "failed to open output file",
    "failed to open temp file",
    "not enough memory",
    "repositioning within input file failed",
    "caller error - bad argument passed to library function",
    "no more entries",
    "subfile not found",
    "additional bytes expected, possibly corrupt input file",
    "unable to open UCM",
    "only single byte character sets are supported",
    "UCM subchar is invalid",
    "missing or bad character map in UCM",
    "record length too large for z/OS and kin",
    "input file appears to be in ASCII format",
    "invalid compressed input data",
    "file name limited to 8 characters",
    "file type limited to 8 characters",
    "file mode limited to 1 alpha and 1 numeric character",
    "year must be greater than or equal to 1900",
    "month must be within 1-12",
    "day must be within 1-31",
    "hour must be within 0-23",
    "minutes must be within 0-59",
    "seconds must be within 0-59",
    "field not modifiable once written to archive",
    "record format must be F or V",
    "record length must be 1 to 65535",
    "filename pointer is NULL",
    "subfile already retained",
    "subfile not previously retained",
    "rename failed...manual rename required",
    ""
};

/* --------------------------------------------------------------------
|| Conversion stuff
*/
#if defined(__MVS__)
#define TO_E_SYS( c ) ( c )
#define TO_A_SYS( c ) ( c )
#else
#define TO_E_SYS( c ) ( a2e_tab[ (unsigned int) c ] )
#define TO_A_SYS( c ) ( e2a_tab[ (unsigned int) c ] )
#endif
#define TO_E_USR( c ) ( vma->a2e_map[ (unsigned int) c ] )
#define TO_A_USR( c ) ( vma->e2a_map[ (unsigned int) c ] )
#define DLM " \t\n"

typedef struct _ent
{
    int u;
    int b;
} ENT;

#if defined(__MVS__)
/* --------------------------------------------------------------------
|| IBM-1047 -> IBM-1047
*/
static const uchar
e2e_tab[] =
{
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
    "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
    "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"
    "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f"
    "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
    "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f"
    "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
    "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
    "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
    "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
    "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
    "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
    "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
    "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff"
};
#endif
 
/* --------------------------------------------------------------------
|| ISO-8859-1 -> IBM-1047
*/
static const uchar
a2e_tab[] =
{
    "\x00\x01\x02\x03\x37\x2D\x2E\x2F\x16\x05\x15\x0B\x0C\x0D\x0E\x0F"
    "\x10\x11\x12\x13\x3C\x3D\x32\x26\x18\x19\x3F\x27\x1C\x1D\x1E\x1F"
    "\x40\x5A\x7F\x7B\x5B\x6C\x50\x7D\x4D\x5D\x5C\x4E\x6B\x60\x4B\x61"
    "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\x7A\x5E\x4C\x7E\x6E\x6F"
    "\x7C\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xD1\xD2\xD3\xD4\xD5\xD6"
    "\xD7\xD8\xD9\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xAD\xE0\xBD\x5F\x6D"
    "\x79\x81\x82\x83\x84\x85\x86\x87\x88\x89\x91\x92\x93\x94\x95\x96"
    "\x97\x98\x99\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xC0\x4F\xD0\xA1\x07"
    "\x20\x21\x22\x23\x24\x25\x06\x17\x28\x29\x2A\x2B\x2C\x09\x0A\x1B"
    "\x30\x31\x1A\x33\x34\x35\x36\x08\x38\x39\x3A\x3B\x04\x14\x3E\xFF"
    "\x41\xAA\x4A\xB1\x9F\xB2\x6A\xB5\xBB\xB4\x9A\x8A\xB0\xCA\xAF\xBC"
    "\x90\x8F\xEA\xFA\xBE\xA0\xB6\xB3\x9D\xDA\x9B\x8B\xB7\xB8\xB9\xAB"
    "\x64\x65\x62\x66\x63\x67\x9E\x68\x74\x71\x72\x73\x78\x75\x76\x77"
    "\xAC\x69\xED\xEE\xEB\xEF\xEC\xBF\x80\xFD\xFE\xFB\xFC\xBA\xAE\x59"
    "\x44\x45\x42\x46\x43\x47\x9C\x48\x54\x51\x52\x53\x58\x55\x56\x57"
    "\x8C\x49\xCD\xCE\xCB\xCF\xCC\xE1\x70\xDD\xDE\xDB\xDC\x8D\x8E\xDF"
};

/* --------------------------------------------------------------------
|| IBM-1047 -> ISO-8859-1
*/
static const uchar
e2a_tab[] =
{
    "\x00\x01\x02\x03\x9c\x09\x86\x7f\x97\x8d\x8e\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x9d\x85\x08\x87\x18\x19\x92\x8f\x1c\x1d\x1e\x1f"
    "\x80\x81\x82\x83\x84\x0a\x17\x1b\x88\x89\x8a\x8b\x8c\x05\x06\x07"
    "\x90\x91\x16\x93\x94\x95\x96\x04\x98\x99\x9a\x9b\x14\x15\x9e\x1a"
    "\x20\xa0\xe2\xe4\xe0\xe1\xe3\xe5\xe7\xf1\xa2\x2e\x3c\x28\x2b\x7c"
    "\x26\xe9\xea\xeb\xe8\xed\xee\xef\xec\xdf\x21\x24\x2a\x29\x3b\x5e"
    "\x2d\x2f\xc2\xc4\xc0\xc1\xc3\xc5\xc7\xd1\xa6\x2c\x25\x5f\x3e\x3f"
    "\xf8\xc9\xca\xcb\xc8\xcd\xce\xcf\xcc\x60\x3a\x23\x40\x27\x3d\x22"
    "\xd8\x61\x62\x63\x64\x65\x66\x67\x68\x69\xab\xbb\xf0\xfd\xfe\xb1"
    "\xb0\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\xaa\xba\xe6\xb8\xc6\xa4"
    "\xb5\x7e\x73\x74\x75\x76\x77\x78\x79\x7a\xa1\xbf\xd0\x5b\xde\xae"
    "\xac\xa3\xa5\xb7\xa9\xa7\xb6\xbc\xbd\xbe\xdd\xa8\xaf\x5d\xb4\xd7"
    "\x7b\x41\x42\x43\x44\x45\x46\x47\x48\x49\xad\xf4\xf6\xf2\xf3\xf5"
    "\x7d\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\xb9\xfb\xfc\xf9\xfa\xff"
    "\x5c\xf7\x53\x54\x55\x56\x57\x58\x59\x5a\xb2\xd4\xd6\xd2\xd3\xd5"
    "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\xb3\xdb\xdc\xd9\xda\x9f"
};

/* --------------------------------------------------------------------
||
*/
static void
mark_dirty( VMA *vma, PSUBFILE *psf )
{
    vma->f_dirty = TRUE;
    
    if( psf != NULL )
    {
        psf->dirty = TRUE;
    }
    
    return;
}

/* --------------------------------------------------------------------
||
*/
static void
set_active( VMA *vma, PSUBFILE *psf )
{
    vma->active = psf;
    vma->sfretained = NULL;
    
    return;
}

/* --------------------------------------------------------------------
||
*/
static int
validate_name( const char *name )
{
    size_t len = strlen( name );
    size_t i;
    
    if( len < 1 || len > 8 )
    {
        return FALSE;
    }
    
    for( i = 0; i < len; i++ )
    {
        int c = name[ i ];
        
        if( !isalnum( c ) )
        {
            switch( c )
            {
                case '$': case '#': case '@': case '+':
                case '-': case ':': case '_':
                    break;
                    
                default:
                    return FALSE;
                    break;
            }
        }
    }
    
    return TRUE;
}

/* --------------------------------------------------------------------
|| Loads character mappings from UCM format files
*/
static int
loaducm( VMA *vma, const char *name, ENT *map, int *subchar )
{
    FILE *f;
    char *token;
    char line[ 256 ];
    int f_cmap = FALSE;
    int f_emap = FALSE;
    int f_sbcs = FALSE;
    int u;
    int b;
    
    /*
    || Set output to default state
    */
    memset( map, '\0', sizeof( ENT ) * 256 );
    *subchar = -1;
    
    /*
    || Open the UCM
    */
    f = fopen( name, "r" );
    if( f == NULL )
    {
        seterr( VMAE_UCMOPEN );
        return FALSE;
    }
    
    /*
    || Read all of the lines
    */
    while( fgets( line, sizeof( line ), f )  )
    {
        /*
        || Get the first word
        */
        token = strtok( line, DLM );
        if( token == NULL )
        {
            continue;
        }
        
        /*
        || Ignore comments and empty lines
        */
        if( token[ 0 ] == '#' || token[ 0 ] == '\0' )
        {
            continue;
        }
        
        /*
        || Start of the character mapping?
        */
        if( strcmp( token, "CHARMAP" ) == 0 )
        {
            /*
            || Ensure we have an SBCS before allowing CHARMAP
            */
            if( !f_sbcs )
            {
                seterr( VMAE_UCMSBCS );
                break;
            }
            
            /*
            || Indicate that we're now processing the mappings
            */
            f_cmap = TRUE;
            continue;
        }
        
        /*
        || End of the character mapping?
        */
        if( strcmp( token, "END" ) == 0 )
        {
            /*
            || Get the next word
            */
            token = strtok( NULL, DLM );
            if( token == NULL )
            {
                continue;
            }
            
            /*
            || And ensure it's the end of the table
            */
            if( strcmp( token, "CHARMAP" ) != 0 )
            {
                continue;
            }
            
            /*
            || Not process the table...tsk, tsk
            */
            if( !f_cmap )
            {
                seterr( VMAE_CHARMAP );
                break;
            }
            
            /*
            || All done, indicate possible success
            */
            f_emap = TRUE;
            break;
        }
        
        /*
        || Check to make sure we have an SBCS mapping
        */
        if( strcmp( token, "<uconv_class>" ) == 0 )
        {
            /*
            || Get the next word
            */
            token = strtok( NULL, DLM );
            if( token == NULL )
            {
                continue;
            }
            
            /*
            || Ensure we have an SBCS mapping
            */
            if( strcmp( token, "\"SBCS\"" ) != 0 )
            {
                seterr( VMAE_UCMSBCS );
                break;
            }
            
            /*
            || Indicate such
            */
            f_sbcs = TRUE;
            continue;
        }
        
        /*
        || Remember the substitution character
        */
        if( strcmp( token, "<subchar>" ) == 0 )
        {
            /*
            || Get the next word
            */
            token = strtok( NULL, DLM );
            if( token == NULL )
            {
                continue;
            }
            
            /*
            || Look like a hex value?
            */
            if( token[ 0 ] != '\\' || token[ 1 ] != 'x' )
            {
                seterr( VMAE_SUBCHAR );
                break;
            }
            
            /*
            || Convert it
            */
            *subchar = (int) strtoul( &token[ 2 ], NULL, 16 );
            continue;
        }
        
        /*
        || Ignore anything else if we're not processing mapping entries
        */
        if( !f_cmap )
        {
            continue;
        }
        
        /*
        || Make sure it looks like a unicode scalar
        */
        if( token[ 0 ] != '<' || token[ 1 ] != 'U' )
        {
            seterr( VMAE_CHARMAP );
            break;
        }
        
        /*
        || Get the value
        */
        u = (int) strtoul( &token[ 2 ], &token, 16 );
        
        /*
        || Terminated correctly?
        */
        if( token[ 0 ] != '>' )
        {
            seterr( VMAE_CHARMAP );
            break;
        }
        
        /*
        || Get the next word
        */
        token = strtok( NULL, DLM );
        if( token == NULL )
        {
            continue;
        }
        
        /*
        || Look like a hex value?
        */
        if( token[ 0 ] != '\\' || token[ 1 ] != 'x' )
        {
            seterr( VMAE_CHARMAP );
            break;
        }
        
        /*
        || Convert it
        */
        b = (int) strtoul( &token[ 2 ], &token, 16 );
        
        /*
        || Nothing strange following it?
        */
        if( token[ 0 ] != '\0' )
        {
            seterr( VMAE_CHARMAP );
            break;
        }
        
        /*
        || Get the next word
        */
        token = strtok( NULL, DLM );
        if( token == NULL )
        {
            continue;
        }
        
        
        /*
        || Only allow direct mappings
        */
        if( strcmp( token, "|0" ) != 0 )
        {
            continue;
        }
        
        /*
        || Save the value
        */
        map[ b ].u = u;
        map[ b ].b = b;
        
        /*
        || Just ignore the rest of the line
        */
        continue;
    }
    
    /*
    || Done with the UCM
    */
    fclose( f );
    
    /*
    || Something happened...
    */
    if( !f_emap )
    {
        return FALSE;
    }
    
    /*
    || Looks like we were successful
    */
    return TRUE;
}

/* --------------------------------------------------------------------
|| Compares entries during sorting and searching
*/
static int
cmp( const void *e1, const void *e2 )
{
    return ( (ENT *)e1 )->u - ( (ENT *)e2 )->u;
}

/* --------------------------------------------------------------------
|| General I/O stuff
*/
static void
unget( VMA *vma, uchar *buf, int len )
{
    while( --len > 0 )
    {
        vma->ibuf[ --vma->ibufp ] = buf[ len ];
        vma->icnt++;
    }
    
    return;
}

/* --------------------------------------------------------------------
|| Get a character from the VMARC
*/
static unsigned int
get( VMA *vma )
{
    /*
    || Need to refill the buffer?
    */
    if( vma->icnt == 0 )
    {
        /*
        || Check for EOF
        */
        if( feof( vma->in ) )
        {
            return (unsigned int) EOF;
        }
        
        /*
        || Remember file position corresponding to start of buffer
        */
        vma->ipos = ftell( vma->in );
        
        /*
        || Read a buffers worth
        */
        vma->icnt = fread( &vma->ibuf[ UNREAD ], 1, BUFLEN, vma->in );
        
        /*
        || Check for errors...defer EOF check for later
        */
        if( ferror( vma->in ) )
        {
            seterr( VMAE_RERR );
            
            return (unsigned int) EOF;
        }
        
        /*
        || Check for EOF
        */
        if( vma->icnt == 0 && feof( vma->in ) )
        {
            return (unsigned int) EOF;
        }
        
        /*
        || Reset buffer position
        */
        vma->ibufp = UNREAD;
    }
    
    /*
    || Track # of bytes read...resets at start of subfile extraction
    */
    vma->bytesin++;
    
    /*
    || One less byte in the buffer
    */
    vma->icnt--;
    
    return (unsigned int) vma->ibuf[ vma->ibufp++ ];
}

/* --------------------------------------------------------------------
|| Returns current input position in the VMARC
*/
static size_t
mytell( VMA *vma )
{
    /*
    || Return the current input position
    ||
    || Accounts for any "unget()" bytes too.
    */
    return vma->ipos + ( vma->ibufp - UNREAD );
}

/* --------------------------------------------------------------------
|| Writes a byte to the extract file
*/
static int
put( VMA *vma, unsigned int c )
{
    /*
    || Track the number of bytes we've written (for display)
    */
    if( c != UINT_MAX )
    {
        vma->bytesout++;
        
        if( vma->bytesout <= 1024 )
        {
            if( c < 0x20 )
            {
                if( c != '\t' && c != '\r' && c != '\n' )
                {
                    vma->dtype = VMAD_BINARY;
                }
            }
        }
    }
    
    /*
    || Nothing else to do if we're not extracting
    */
    if( !vma->f_extract )
    {
        return TRUE;
    }
    
    /*
    || Flush the output buffer
    */
    if( c == UINT_MAX )
    {
        if( fwrite( vma->obuf, 1, vma->opos, vma->out ) != vma->opos )
        {
            seterr( VMAE_WERR );
            return FALSE;
        }
        
        vma->opos = 0;
        
        return TRUE;
    }
    
    /*
    || Make sure we don't overflow
    */
    if( vma->opos == vma->omax )
    {
        seterr( VMAE_OOVER );
        return FALSE;
    }
    
    /*
    || Store the character
    */
    vma->obuf[ vma->opos++ ] = c;
    
    return TRUE;
}

/* --------------------------------------------------------------------
|| Shared compression I/O
*/
static int
putbyte( VMA *vma, unsigned short c )
{
    int code;
    
    /*
    || Convert back to EBCDIC character
    */
    code = c - kodebcd;
    
    /*
    || Handle special codes (only EOR)
    */
    if( code < 0 )
    {
        /*
        || Track number of times we've seen the EOR code
        */
        vma->eor++;
        
        /*
        || Variable records:
        || First means end-of-record...second one means end-of-file
        ||
        || Fixed records:
        || First means end-of-file
        */
        if( vma->eor > 1 || vma->recfm == 'F' )
        {
            if( !put( vma, UINT_MAX ) )
            {
                return -1;
            }
            
            return 0;
        }
        
        /*
        || Append a line end if converting
        */
        if( vma->f_text )
        {
#if defined( _WIN32 )
            if( !put( vma, '\r' ) )
            {
                return -1;
            }
#endif
            if( !put( vma, '\n' ) )
            {
                return -1;
            }
        }
        
        if( !put( vma, UINT_MAX ) )
        {
            return -1;
        }
        
        return 1;
    }
    
    /*
    || Reset EOR counter
    */
    vma->eor = 0;
    
    /*
    || Bail out if we're scanning and have hit our limit
    */
    if( vma->f_scanning && vma->bytesin >= 1024 )
    {
        return 0;
    }
    
    /*
    || Fixed files need record length tracking to output newline since
    || we don't receive an EOR code for them.
    */
    if( vma->recfm == 'F' )
    {
        if( vma->recbytes == vma->lrecl )
        {
            vma->recbytes = 0;
            
            if( vma->f_text )
            {
#if defined( _WIN32 )
                if( !put( vma, '\r' ) )
                {
                    return -1;
                }
#endif
                if( !put( vma, '\n' ) )
                {
                    return -1;
                }
            }
            
            if( !put( vma, UINT_MAX ) )
            {
                return -1;
            }
        }
        
        vma->recbytes++;
    }
    
    /*
    || Translate
    */
    if( vma->f_text )
    {
        code = TO_A_USR( code );
    }
    
    /*
    || Put the byte
    */
    if( !put( vma, code ) )
    {
        return -1;
    }
    
    return 1;
}

static unsigned short
getcode( VMA *vma )
{
    unsigned int code;
    
    /*
    || Always need to read as least one byte
    */
    code = get( vma );
    if( code == EOF )
    {
        return USHRT_MAX;
    }
    
    /*
    || Use what's left over from previous call
    */
    if( vma->residual != UINT_MAX )
    {
        code |= ( vma->residual << 8 );
        vma->residual = UINT_MAX;
    }
    else
    {
        /*
        || Get the low 4 bits of the 12 bit code
        */
        vma->residual = get( vma );
        if( vma->residual == EOF )
        {
            return USHRT_MAX;
        }
        
        /*
        || And put it where it belongs
        */
        code = ( code << 4 ) | ( vma->residual >> 4 );
    }
    
    /*
    || Always returns a 12 bit code
    */
    return code & 0xfff;
}

/* --------------------------------------------------------------------
|| LZW Decompression
*/
static int
lookup( VMA *vma, LZWSTRING *lastpred, unsigned short nextchar, LZWSTRING **ent )
{
    unsigned int offset;
    LZWSTRING *bucket;
    LZWSTRING *origin;
    LZWSTRING *left;
    LZWSTRING *right;
    int wrapped;
    
    /*
    || Hash function.
    */
    offset = ( ( ( (size_t) lastpred ) / sizeof(LZWHASH) ) ^ nextchar )
    & 0xffff;
    offset *= offset;
    offset %= HASHSIZE;
    
    /*
    || Locate start of hash chain
    */
    bucket = (LZWSTRING *) &vma->lzwhashtab[ offset ];
    origin = bucket;
    
    /*
    || Search hash table for (predecessor,character) combination.
    */
    while( bucket->right != origin )
    {
        bucket = bucket->right;
        if( bucket->pred == lastpred && bucket->schar == nextchar )
        {
            /* found it*/
            *ent = bucket;
            return TRUE;
        }
    }
    
    /*
    || Desired entry not found.  Add it if possible.
    */
    
    /*
    || Bump reference count of previous predecessor
    */
    if( lastpred != ENDCHAIN )
    {
        lastpred->refcnt++;
    }
    
    /*
    || Search for free entry until end of table is reached
    */
    wrapped = 0;
    do
    {
        /*
        || Bump to next table entry
        */
        ++vma->lzwtabp;
        
        /*
        || Reached the end of the table?
        */
        if( vma->lzwtabp > vma->lzwtabl )
        {
            /*
            || Get out if we've been here before
            */
            if( wrapped )
            {
                break;
            }
            
            wrapped = 1;
            vma->lzwtabp = vma->lzwtabs;
        }
    } while( vma->lzwtabp->refcnt != 0 );
    
    /*
    || If we hit the KwKwK case, then don't add this entry
    || to the table. (big flower box in VMARC source)
    */
    if( vma->lzwtabp > vma->lzwtabl )
    {
        lastpred->refcnt--;
        
        /*
        || Indicate it wasn't found
        */
        *ent = lastpred;
        return FALSE;
    }
    
    /*
    || Remove entry from previous location in hash and prefix chains
    */
    if( vma->lzwtabp->right != 0 )
    {
        right = vma->lzwtabp->right;
        left = vma->lzwtabp->left;
        right->left = left;
        left->right = right;
        if( vma->lzwtabp->pred != 0 )
        {
            vma->lzwtabp->pred->refcnt--;
        }
    }
    
    /*
    || Add it to its new home in the prefix chain
    */
    vma->lzwtabp->pred = lastpred;
    vma->lzwtabp->schar = nextchar;
    vma->lzwtabp->refcnt = 0;
    
    /*
    || And in the hash chain
    */
    right = origin->right;
    vma->lzwtabp->right = right;
    vma->lzwtabp->left = origin;
    origin->right = vma->lzwtabp;
    right->left = vma->lzwtabp;
    
    /*
    || Return entry
    */
    *ent = vma->lzwtabp;
    return FALSE;
}

static void
lzwinit( VMA *vma )
{
    LZWSTRING *ent;
    LZWHASH *hash;
    unsigned short ndx;
    
    /*
    || Clear the prefix table
    */
    memset( vma->lzwstrtab, 0, sizeof( vma->lzwstrtab ) );
    
    /*
    || Initialize the hash table
    */
    hash = &vma->lzwhashtab[ 0 ];
    for( ndx = 0; ndx < HASHSIZE; ndx++ )
    {
        hash->head = (LZWSTRING *) hash;
        hash->tail = (LZWSTRING *) hash;
        hash++;
    }
    
    /*
    || Preload the specials plus 256 EBCDIC characters
    */
    vma->lzwtabp = &vma->lzwstrtab[ 0 ] - 1; /* -1 for lookup preinc*/
    vma->lzwtabl = &vma->lzwstrtab[ TABSIZE - 1 ];
    
    for( ndx = 0; ndx < kodmax + 1; ndx++ )
    {
        /*
        || Force addition of entry and set reference count
        */
        lookup( vma, ENDCHAIN, ndx, &ent );
        ent->refcnt = 1;
    }
    
    /*
    || Set wrap-around point
    */
    vma->lzwtabs = ent + 1;
    
    return;
}

static int
extract_lzw( VMA *vma )
{
    LZWSTRING *prev;
    LZWSTRING *last;
    LZWSTRING *ent;
    LZWSTRING *tabptr;
    int wrapped;
    unsigned short code;
    int rc;
    
    /*
    || Initialize
    */
    lzwinit( vma );
    tabptr = vma->lzwtabs;
    tabptr->pred = ENDCHAIN;
    
    while( TRUE )
    {
        /*
        || Get the next code
        */
        code = getcode( vma );
        if( code == USHRT_MAX )
        {
            return seterr( VMAE_BADDATA );
        }
        
        /*
        || Protect against corrupt hives
        */
        ent = &vma->lzwstrtab[ code ];
        if( ent->pred == NULL )
        {
            return seterr( VMAE_BADDATA );
        }
        
        /*
        || Traverse down to get to the start of the string
        */
        prev = ENDCHAIN;
        do
        {
            last = ent->pred;
            ent->pred = prev;
            prev = ent;
            ent = last;
        } while( ent != ENDCHAIN );
        
        /*
        ||
        */
        tabptr->schar = prev->schar;
        
        /*
        || Go back up while outputing the characters in correct order
        */
        do
        {
            rc = putbyte( vma, prev->schar );
            if( rc == 0 )
            {
                return seterr( VMAE_NOERR );
            }
            if( rc < 0 )
            {
                return seterr( VMAE_BADDATA );
            }
            
            last = prev->pred;
            prev->pred = ent;
            ent = prev;
            prev = last;
        } while( prev != ENDCHAIN );
        
        /*
        || Start building follow-on entry for this code.
        */
        ent->refcnt++;
        
        /*
        || Search for free entry until end of table is reached
        */
        wrapped = FALSE;
        tabptr = vma->lzwtabp;
        do
        {
            /*
            || Bump to next table entry
            */
            ++tabptr;
            
            /*
            || Reached the end of the table?
            */
            if( tabptr > vma->lzwtabl )
            {
                /*
                || Get out if we've been here before
                */
                if( wrapped )
                {
                    break;
                }
                
                wrapped = TRUE;
                tabptr = vma->lzwtabs;
            }
        } while( tabptr->refcnt );
        
        /*
        || If we hit the KwKwK case, then don't add this entry
        || to the table. (big flower box in VMARC source)
        */
        if( tabptr > vma->lzwtabl )
        {
            ent->refcnt--;
            continue;
        }
        
        /*
        || Add the new entry
        */
        if( tabptr->pred != ENDCHAIN && tabptr->pred != NULL )
        {
            tabptr->pred->refcnt--;
        }
        
        /*
        || Remember for next round
        */
        tabptr->pred = ent;
        vma->lzwtabp = tabptr;
    }
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
|| S2 Decompression
*/
static void
s2addnew( VMA *vma, unsigned short slastcode, unsigned short lastcode  )
{
    STRDSECT *left;
    STRDSECT *right;
    STRDSECT *probe;
    STRDSECT *ostr;
    STRDSECT **psib;
    STRDSECT **opsib;
    
    /*
    || Find string table entries for the left and right substrings
    */
    left = &vma->s2strtab[ slastcode ];
    right = &vma->s2strtab[ lastcode ];
    
    /*
    || Would string be longer with buffer?
    */
    if( ( left->strlen + right->strlen ) > ( TABSIZE - 2 ) )
    {
        return;
    }
    
    left->strcount++;
    right->strcount++;
    
    /*
    || Find a place for the new string
    */
    probe = vma->s2tabp;
    do
    {
        probe++;
        if( probe > vma->s2tabl )
        {
            probe = vma->s2tabs;
        }
    } while( probe->strcount != 0 );
    
    /*
    || Replace an old string entry with this new one
    */
    ostr = probe->strleft;
    if( ostr )
    {
        ostr->strcount--;
        
        psib = &ostr->stroffsp;
        while( *psib != probe )
        {
            psib = &( *psib )->strsiblg;
        }
        *psib = probe->strsiblg;
        
        ostr = probe->strright;
        ostr->strcount--;
    }
    
    /*
    || Now construct a new entry for this string
    */
    if( right == vma->s2tabp )
    {
        if( left == right->strleft )
        {
            ostr = right;
            right = left;
            left = ostr;
        }
    }
    
    probe->strleft = left;
    probe->strright = right;
    probe->strlen = 2 + left->strlen + right->strlen;
    
    /*
    || Insert new node into offspring/sibling list of left substring
    */
    psib = &left->stroffsp;
    do
    {
        opsib = psib;
        left = *psib;
        psib = &left->strsiblg;
    } while( left && ( left != vma->s2tabp ) );
    
    probe->strsiblg = left;
    *opsib = probe;
    vma->s2tabp = probe;
    
    return;
}

static void
s2init( VMA *vma )
{
    unsigned short ndx;
    
    memset( &vma->s2buf, 0, sizeof( vma->s2buf ) );
    memset( &vma->s2strtab, 0, sizeof( vma->s2strtab ) );
    
    vma->s2tabs = &vma->s2strtab[ kodmax + 1 ];
    vma->s2tabl = &vma->s2strtab[ TABSIZE ];
    vma->s2tabp = vma->s2tabl;
    
    for( ndx = 0; ndx < kodmax + 1; ndx++ )
    {
        vma->s2strtab[ ndx ].strchar = ndx;
        vma->s2strtab[ ndx ].strcount = 1;
    }
    
    return;
}

static int
extract_s2( VMA *vma )
{
    STRDSECT *prev;
    STRDSECT *curr;
    STRDSECT *next;
    unsigned short slastcode;
    unsigned short lastcode;
    int rc;
    
    /*
    || Intialize tables
    */
    s2init( vma );
    
    /*
    || Retrieve and remember the first code
    */
    slastcode = getcode( vma );
    if( slastcode == USHRT_MAX )
    {
        return seterr( VMAE_NEEDMORE );
    }
    
    /*
    || Output the first code
    */
    rc = putbyte( vma, slastcode );
    if( rc < 0 )
    {
        return seterr( VMAE_BADDATA );
    }
    
    /*
    || EOF is okay at this point
    */
    if( rc == 0 )
    {
        return seterr( VMAE_NOERR );
        return TRUE;
    }
    
    while( TRUE )
    {
        lastcode = getcode( vma );
        if( slastcode == USHRT_MAX )
        {
            return seterr( VMAE_BADDATA );
        }
        
        curr = &vma->s2strtab[ lastcode ];
        prev = NULL;
        do
        {
            /*
            || Drop down as far as possible along the leftmost path
            */
            do
            {
                next = curr->strleft;
                curr->strleft = prev;
                prev = curr;
                curr = next;
            } while( curr );
            
            /*
            || We have hit the bottom
            */
            curr = prev;
            prev = curr->strleft;
            curr->strleft = next;
            rc = putbyte( vma, curr->strchar );
            if( rc < 0 )
            {
                return seterr( VMAE_BADDATA );
            }
            
            /*
            || Done...
            */
            if( rc == 0 )
            {
                return seterr( VMAE_NOERR );
            }
            
            /*
            || Retrace path up looking for the first unexplored right
            || path
            */
            while( TRUE )
            {
                next = curr;
                curr = prev;
                
                /*
                || Reached the root
                */
                if( !curr )
                {
                    break;
                }
                
                /*
                || Have another right side to traverse
                */
                if( !curr->visited )
                {
                    prev = curr->strleft;
                    curr->strleft = next;
                    
                    next = curr->strright;
                    curr->strright = prev;
                    
                    curr->visited = TRUE;
                    
                    prev = curr;
                    curr = next;
                    break;
                }
                
                /*
                || Move back up a level
                */
                curr->visited = FALSE;
                prev = curr->strright;
                curr->strright = next;
            }
        } while( curr );
        
        /*
        || Extend string table with new entry
        */
        s2addnew( vma, slastcode, lastcode );
        slastcode = lastcode;
    }
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
|| ASIS stuff
*/
static int
extract_asis( VMA *vma )
{
    int lrecl;
    unsigned int h;
    unsigned int l;
    
    while( TRUE )
    {
        /*
        || Get the reccord length
        */
        h = get( vma );
        if( h == EOF )
        {
            break;
        }
        
        l = get( vma );
        if( l == EOF )
        {
            return seterr( VMAE_NEEDMORE );
        }
        
        /*
        || Reconstruct lrecl
        */
        lrecl = ( h << 8 ) + l;
        
        /*
        || Done when we get a zero LRECL
        */
        if( lrecl == 0 )
        {
            break;
        }
        
        /*
        || Copy input to output
        */
        while( lrecl-- )
        {
            l = get( vma );
            if( l == EOF )
            {
                return seterr( VMAE_NEEDMORE );
            }
            
            if( putbyte( vma, l + kodebcd ) < 0 )
            {
                return seterr( VMAE_WERR );
            }
        }
        
        /*
        || Force EOR for variable files only
        */
        if( vma->recfm == 'V' )
        {
            if( putbyte( vma, 0 ) < 0 )
            {
                return seterr( VMAE_WERR );
            }
        }
    }
    
    /*
    || Force EOF
    */
    if( putbyte( vma, 0 ) < 0 )
    {
        return seterr( VMAE_WERR );
    }
    
    return seterr( VMAE_NOERR );;
}

static int
extract( VMA *vma )
{
    int rc;
    
    /*
    || Get properly positioned
    */
    if( fseek( vma->in, vma->active->dataoff, SEEK_SET ) != 0 )
    {
        seterr( VMAE_SEEK );
        return FALSE;
    }
    
    /*
    || Cache RECFM and LRECL
    */
    vma->recfm = vma->active->sf.recfm;
    vma->lrecl = vma->active->sf.lrecl;
    
    /*
    || Reset various counters and I/O controls
    */
    vma->ibufp = 0;
    vma->icnt = 0;
    vma->bytesin = 0;
    vma->bytesout = 0;
    vma->residual = UINT_MAX;
    vma->recbytes = 0;
    vma->eor = 0;
    vma->dtype = VMAD_TEXT;    /* assume text for now*/
    
    /*
    || Extract based on storage type
    */
    if( vma->active->flags & HF_ASIS )
    {
        rc = ( extract_asis( vma ) == VMAE_NOERR );
    }
    else if ( vma->active->flags & HF_S2 )
    {
        rc = ( extract_s2( vma ) == VMAE_NOERR );
    }
    else
    {
        rc = ( extract_lzw( vma ) == VMAE_NOERR );
    }
    
    if( rc )
    {
        if( vma->active->sf.dtype == VMAD_UNKNOWN )
        {
            vma->active->sf.dtype = vma->dtype;
        }
        
        if( !vma->f_scanning )
        {
            vma->active->sf.compressed = vma->bytesin;
            vma->active->sf.uncompressed = vma->bytesout;
        }
    }
    
    return rc;
}

/* --------------------------------------------------------------------
|| Convert decimal byte to binary
*/
static int
cvb( int dec )
{
    int bin;
    
    /*
    || Mask out any unwanteds...shouldn't be any
    */
    dec &= 0xff;
    
    /*
    || Get the "tens" digit
    */
    bin = ( dec >> 4 ) * 10;
    
    /*
    || And add in the "ones" digit
    */
    bin += ( dec & 0x0f );
    
    return bin;
}

/* --------------------------------------------------------------------
|| Convert binary to decimal byte
*/
static int
cvd( int bin )
{
    int dec;
    
    /*
    || Mask out any unwanteds...shouldn't be any
    */
    bin &= 0xff;
    
    /*
    || Set the "tens" digit
    */
    dec = ( bin / 10 ) << 4;
    
    /*
    || Set the "ones" digit
    */
    dec |= bin % 10;
    
    return dec;
}

/* --------------------------------------------------------------------
||
*/
static FILE *
open_exclusive( VMA *vma, const char *name, int mode )
{
    FILE *file;
    struct stat st;
    int fd;
    
    /*
    || Set default mode bits
    */
    if( mode == 0 )
    {
        st.st_mode = PERMS;
    }
    
    /*
    || Open the file for exclusive use
    */
    fd = open( name, O_CREAT | O_EXCL | O_RDWR | O_BINARY, mode );
    if( fd < 0 )
    {
        return NULL;
    }
    
    /*
    || Associate with a stream
    */
    file = fdopen( fd, "r+b" );
    if( file == NULL )
    {
        close( fd );
        return NULL;
    }
    
    return file;
}

/* --------------------------------------------------------------------
||
*/
static int
open_temp( VMA *vma )
{
    char sfx[] = ".XXXXXX" ;
    struct stat st;
   
    /*
    || Nothing to do if it's already open
    */
    if( vma->tfile != NULL )
    {
        return seterr( VMAE_NOERR );
    }
    
    /*
    || Allocate memory for the name
    */
    vma->tname = malloc( strlen( vma->vname ) + sizeof( sfx ) );
    if( vma->tname == NULL )
    {
        return seterr( VMAE_MEM );
    }
    
    /*
    || Build the name
    */
    strcpy( vma->tname, vma->vname );
    strcat( vma->tname, sfx );
    if( mktemp( vma->tname ) == NULL )
    {
        return seterr( VMAE_MEM );
    }
    
    /*
    || Build the name
    */
    if( !vma->vfile || fstat( fileno( vma->vfile ), &st ) != 0 )
    {
        st.st_mode = PERMS;
    }
    
    /*
    || And open the file for exclusive use
    */
    vma->tfile = open_exclusive( vma, vma->tname, st.st_mode );
    if( vma->tfile == NULL )
    {
        return seterr( VMAE_TOPEN );
    }
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
||
*/
static int
write_trailer( VMA *vma, FILE *f )
{
    long pos;
    
    /*
    || Get the current position
    */
    pos = ftell( f );
    if( pos == -1 )
    {
        return seterr( VMAE_WERR );
    }
    
    /*
    || And pad to a multiple of 80, but don't fall into the classic
    || trap of writing 80 nulls when we have a complete card.
    */
    pos %= 80;
    for( pos = (pos ? 80 - pos : 0 ) ; pos > 0; pos-- )
    {
        if( fputc( 0, f ) == EOF )
        {
            return seterr( VMAE_WERR );
        }
    }
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
||
*/
static int
write_header( VMA *vma, FILE *f, PSUBFILE *psf )
{
    char *p;
    int i;
    
    /*
    || Build the subfile header
    */
    vma->head[ H_VER ] = psf->sf.ver;
    vma->head[ H_REL ] = psf->sf.rel;
    
    for( p = psf->sf.fn, i = 0; i < 8; i++ )
    {
        vma->head[ H_FN + i ] = *p ? TO_E_SYS( *p++ ) : TO_E_SYS( ' ' );
    }
    
    for( p = psf->sf.ft, i = 0; i < 8; i++ )
    {
        vma->head[ H_FT + i ] = *p ? TO_E_SYS( *p++ ) : TO_E_SYS( ' ' );
    }
    
    for( p = psf->sf.fm, i = 0; i < 2; i++ )
    {
        vma->head[ H_FM + i ] = *p ? TO_E_SYS( *p++ ) : TO_E_SYS( ' ' );
    }
    
    vma->head[ H_FLAGS ] = psf->flags;
    
    vma->head[ H_LRECL ] = ( psf->sf.lrecl >> 8 ) & 0xff;
    vma->head[ H_LRECL + 1 ] = psf->sf.lrecl & 0xff;
    
    i = psf->sf.year - 1900;
    if( i >= 100 )
    {
        vma->head[ H_FLAGS ] |= HF_Y2K;
        i -= 100;
    }
    vma->head[ H_YEAR ] = cvd( i );
    
    vma->head[ H_MONTH ] = cvd( psf->sf.month );
    vma->head[ H_DAY ] = cvd( psf->sf.day );
    vma->head[ H_HOUR ] = cvd( psf->sf.hour );
    vma->head[ H_MINUTE ] = cvd( psf->sf.minute );
    vma->head[ H_SECOND ] = cvd( psf->sf.second );
    vma->head[ H_RECFM ] = TO_E_SYS( psf->sf.recfm );
    
    /*
    || Write the header ID
    */
    if( fwrite( hid, 1, 8, f ) != 8 || ferror( f ) )
    {
        return seterr( VMAE_WERR );
    }
    
    /*
    || Write the header
    */
    if( fwrite( vma->head, 1, H_DLEN, f ) != H_DLEN || ferror( f ) )
    {
        return seterr( VMAE_WERR );
    }
    
    /*
    || Remember the data offset
    */
    psf->dataofftmp = ftell( f );
    if( psf->dataofftmp < 0 )
    {
        return seterr( VMAE_WERR );
    }

    /*
    || Prevent modification of certain header fields
    */
    psf->locked = TRUE;
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
||
*/
static int
add_asis( VMA *vma, FILE *f, int mode )
{
    unsigned char *buf = vma->ibuf;
    size_t len;
    size_t i;
    
    if( vma->recfm == VMAR_FIXED )
    {
        len = vma->lrecl;
        
        while( !feof( f ) && !ferror( f ) )
        {
            i = fread( buf, 1, len, f );
            if( i == 0 )
            {
                break;
            }
            
            vma->bytesin += i;
            
            if( fputc( ( i >> 8 ) & 0xff, vma->tfile ) == EOF )
            {
                seterr( VMAE_WERR );
                goto error;
            }
            
            vma->bytesout++;
            
            if( fputc( i & 0xff, vma->tfile ) == EOF )
            {
                seterr( VMAE_WERR );
                goto error;
            }
            
            vma->bytesout++;
            
            if( mode == VMAX_BINARY )
            {
                if( fwrite( buf, 1, i, vma->tfile ) != i )
                {
                    seterr( VMAE_WERR );
                    goto error;
                }
            }
            else
            {
                size_t j;
                for( j = 0; j < i; j++ )
                {
                    if( fputc( TO_E_USR( buf[ j ] ), vma->tfile ) == EOF )
                    {
                        seterr( VMAE_WERR );
                        goto error;
                    }
                }
            }
            
            vma->bytesout += i;
        }
    }
    else
    {
        len = ( mode == VMAX_BINARY ? vma->lrecl : 0 );
        i = 0;
        while( !feof( f ) && !ferror( f ) )
        {
            int c = fgetc( f );
            
            if( c == EOF )
            {
                break;
            }
            
            vma->bytesin++;
            
            if( mode == VMAX_BINARY )
            {
                if( i == len )
                {
                    if( fputc( ( i >> 8 ) & 0xff, vma->tfile ) == EOF )
                    {
                        seterr( VMAE_WERR );
                        goto error;
                    }
                    
                    vma->bytesout++;
                    
                    if( fputc( i & 0xff, vma->tfile ) == EOF )
                    {
                        seterr( VMAE_WERR );
                        goto error;
                    }
                    
                    vma->bytesout++;
                    
                    if( fwrite( buf, 1, i, vma->tfile ) != i )
                    {
                        seterr( VMAE_WERR );
                        goto error;
                    }
                    
                    vma->bytesout += i;
                    
                    i = 0;
                }
                
                buf[ i++ ] = c;
            }
            else
            {
                switch( c )
                {
                    case '\r':
                        break;
                        
                    case '\n':
                        /*
                        || CMS doesn't like zero length records, so insert
                        || a blank
                        */
                        if( i == 0 )
                        {
                            buf[ i++ ] = TO_E_USR( ' ' );
                        }
                        
                        if( fputc( ( i >> 8 ) & 0xff, vma->tfile ) == EOF )
                        {
                            seterr( VMAE_WERR );
                            goto error;
                        }
                        
                        vma->bytesout++;
                        
                        if( fputc( i & 0xff, vma->tfile ) == EOF )
                        {
                            seterr( VMAE_WERR );
                            goto error;
                        }
                        
                        vma->bytesout++;
                        
                        if( fwrite( buf, 1, i, vma->tfile ) != i )
                        {
                            seterr( VMAE_WERR );
                            goto error;
                        }
                        
                        vma->bytesout += i;
                        
                        if( i > len )
                        {
                            len = i;
                        }
                        
                        i = 0;
                        break;
                        
                    default:
                        if( i > (size_t) vma->lrecl )
                        {
                            seterr( VMAE_LRECL );
                            goto error;
                        }
                        
                        buf[ i++ ] = TO_E_USR( c );
                        break;
                }
            }
        }
        
        if( i > 0 )
        {
            if( fputc( ( i >> 8 ) & 0xff, vma->tfile ) == EOF )
            {
                seterr( VMAE_WERR );
                goto error;
            }
            
            vma->bytesout++;
            
            if( fputc( i & 0xff, vma->tfile ) == EOF )
            {
                seterr( VMAE_WERR );
                goto error;
            }
            
            vma->bytesout++;
            
            if( fwrite( buf, 1, i, vma->tfile ) != i )
            {
                seterr( VMAE_WERR );
                goto error;
            }
            
            vma->bytesout += i;
        }
        
        if( i > len )
        {
            len = i;
        }
    }
    
    if( ferror( f ) )
    {
        seterr( VMAE_RERR );
        goto error;
    }
    
    /*
    || Write end of file...0 record length
    */
    if( fputc( 0, vma->tfile ) == EOF )
    {
        seterr( VMAE_WERR );
        goto error;
    }
    
    vma->bytesout++;
    
    if( fputc( 0, vma->tfile ) == EOF )
    {
        seterr( VMAE_WERR );
        goto error;
    }
    
    vma->bytesout++;
    
    vma->omax = len;
    
    seterr( VMAE_NOERR );
    
error:
    
    return vma->lasterr;
}

/* --------------------------------------------------------------------
||
*/
static int
getbyte( VMA *vma, FILE *f, int *pc )
{
    int c;
    
    if( vma->eor )
    {
        vma->eor = 0;
        vma->recbytes = 0;
        
        *pc = kodendr;
        
        return seterr( VMAE_NOERR );
    }
    
    while( TRUE )
    {
        /*
        || Need to refill the buffer?
        */
        if( vma->icnt == 0 )
        {
            /*
            || Check for EOF
            */
            if( feof( f ) )
            {
                *pc = EOF;
                
                return seterr( VMAE_NOERR );
            }
            
            /*
            || Remember file position corresponding to start of buffer
            */
            vma->ipos = ftell( f );
            
            /*
            || Read a buffers worth
            */
            vma->icnt = fread( &vma->ibuf[ UNREAD ], 1, BUFLEN, f );
            
            /*
            || Check for errors...defer EOF check for later
            */
            if( ferror( f ) )
            {
                *pc = EOF;
                return seterr( VMAE_RERR );
            }
            
            /*
            || Reset buffer position
            */
            vma->ibufp = UNREAD;
        }
        
        /*
        || Get next character
        */
        c = vma->ibuf[ vma->ibufp++ ];
        
        /*
        || Track # of bytes read...resets at start of subfile extraction
        */
        vma->bytesin++;
        
        /*
        || One less byte in the buffer
        */
        vma->icnt--;
        
        /*
        || Handle text conversion
        */
        if( vma->f_text )
        {
            /*
            || Ignore
            */
            if( c == '\r' )
            {
                continue;
            }
            
            if( c == '\n' )
            {
                /*
                || CMS doesn't like zero length records, so insert
                || a blank and trick the next getbyte() call into
                || returning an end of record code.
                */
                if( vma->recbytes == 0 )
                {
                    c = ' ';
                    vma->eor = 1;
                }
                else
                {
                    vma->recbytes = 0;
                    
                    *pc = kodendr;
                    
                    return seterr( VMAE_NOERR );
                }
            }
            
            c = TO_E_USR( c );
        }
        
        /*
        || Always exit loop
        */
        break;
    }
    
    /*
    || Track # of bytes in the record
    */
    vma->recbytes++;
    
    /*
    || And maximum record length
    */
    if( vma->recbytes > vma->omax )
    {
        vma->omax = vma->recbytes;
    }
    
    /*
    || Convert to coded EBCDIC
    */
    *pc = c + kodebcd;
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
||
*/
static int
putcode( VMA *vma, unsigned short code )
{
    if( code == USHRT_MAX )
    {
        if( vma->residual != UINT_MAX )
        {
            if( fputc( vma->residual, vma->tfile ) == EOF )
            {
                return seterr( VMAE_WERR );
            }
            
            vma->bytesout++;
        }
        
        return seterr( VMAE_NOERR );
    }
    
    code &= 0xfff;
    
    if( vma->residual == UINT_MAX )
    {
        vma->residual = ( code & 0x0f ) << 12;
        if( fputc( code >> 4, vma->tfile ) == EOF )
        {
            return seterr( VMAE_WERR );
        }
        
        vma->bytesout++;
    }
    else
    {
        code |= vma->residual;
        vma->residual = UINT_MAX;
        
        if( fputc( code >> 8, vma->tfile ) == EOF )
        {
            return seterr( VMAE_WERR );
        }
        
        vma->bytesout++;
        
        if( fputc( code & 0xff, vma->tfile ) == EOF )
        {
            return seterr( VMAE_WERR );
        }
        
        vma->bytesout++;
    }
    
    return seterr( VMAE_NOERR );
}

/* --------------------------------------------------------------------
||
*/
static int
add_lzw( VMA *vma, FILE *f, int mode )
{
    LZWSTRING *lastpred;
    LZWSTRING *ent;
    unsigned short code;
    int c;
    
    /*
    || Initialize
    */
    lzwinit( vma );
    
    lastpred = ENDCHAIN;
    if( getbyte( vma, f, &c ) != VMAE_NOERR )
    {
        return vma->lasterr;
    }
    
    while( c != EOF )
    {
        if( lookup( vma, lastpred, c, &ent ) )
        {
            lastpred = ent;
            if( getbyte( vma, f, &c ) != VMAE_NOERR )
            {
                return vma->lasterr;
            }
            continue;
        }
        
        code = (unsigned short) ( lastpred - vma->lzwstrtab );
        if( putcode( vma, code ) != VMAE_NOERR )
        {
            return vma->lasterr;
        }
        lastpred = ENDCHAIN;
    }
    
    if( ferror( f ) )
    {
        return seterr( VMAE_RERR );
    }
    
    code = (unsigned short) ( lastpred - vma->lzwstrtab );
    if( putcode( vma, code ) != VMAE_NOERR )
    {
        return vma->lasterr;
    }
    
    if( putcode( vma, kodendr ) != VMAE_NOERR )
    {
        return vma->lasterr;
    }
    
    if( putcode( vma, USHRT_MAX ) != VMAE_NOERR )
    {
        return vma->lasterr;
    }
    
    return seterr( VMAE_NOERR );
}


/* --------------------------------------------------------------------
|| Searches for the next header (There HAS to be a better way!)
||
|| If you can think of way to do this better, PLEASE let me know.
*/
static int
locate_file( VMA *vma )
{
    unsigned int byte;
    int cnt;
    int ndx;
    int isascii = FALSE;
    
    while( TRUE )
    {
        ndx = 0;
        while( ndx < sizeof( hid ) - 1 )
        {
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            
            if( byte == hid[ ndx ] )
            {
                ndx++;
            }
            else if( byte == aid[ ndx ] )
            {
                ndx++;
                isascii = TRUE;
            }
            else if( byte == hid[ 0 ] )
            {
                ndx = 1;
            }
            else if( byte == aid[ 0 ] )
            {
                ndx = 1;
                isascii = TRUE;
            }
            else
            {
                isascii = FALSE;
                ndx = 0;
            }
        }
        
        if( isascii )
        {
            seterr( VMAE_ASCII );
            return FALSE;
        }
        
        cnt = 0;
        do
        {
            /*
            || Retrieve the creation version
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte != 1 )
            {
                break;
            }
            
            /*
            || Retrieve the creation release
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return 0;
            }
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte > 2 )
            {
                break;
            }
            
            /*
            || Retrieve the file name
            */
            for( ndx = 0; ndx < 8; ndx++ )
            {
                byte = get( vma );
                if( byte == EOF )
                {
                    return FALSE;
                }
                
                vma->head[ cnt ] = TO_A_SYS( byte );
                if( !isprint( vma->head[ cnt++ ] ) )
                {
                    break;
                }
            }
            
            /*
            || Verify
            */
            if( ndx != 8 )
            {
                break;
            }
            
            /*
            || Retrieve the file type
            */
            for( ndx = 0; ndx < 8; ndx++ )
            {
                byte = get( vma );
                if( byte == EOF )
                {
                    return FALSE;
                }
                
                vma->head[ cnt ] = TO_A_SYS( byte );
                if( !isprint( vma->head[ cnt++ ] ) )
                {
                    break;
                }
            }
            
            /*
            || Verify
            */
            if( ndx != 8 )
            {
                break;
            }
            
            /*
            || Retrieve the file mode
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = TO_A_SYS( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( !isalpha( byte ) )
            {
                break;
            }
            
            /*
            || Retrieve the file mode number
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = TO_A_SYS( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte < '0' || byte > '9' )
            {
                break;
            }
            
            /*
            || Retrieve the record length
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            vma->head[ cnt++ ] = byte;
            
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            vma->head[ cnt++ ] = byte;
            
            /*
            || Retrieve the year
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            vma->head[ cnt++ ] = cvb( byte );
            
            /*
            || Retrieve the month
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = cvb( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte < 1 || byte > 12 )
            {
                break;
            }
            
            /*
            || Retrieve the day
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = cvb( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte < 1 || byte > 31 )
            {
                break;
            }
            
            /*
            || Retrieve the hour
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = cvb( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte > 23 )
            {
                break;
            }
            
            /*
            || Retrieve the minute
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = cvb( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte > 59 )
            {
                break;
            }

            /*
            || Retrieve the second
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = cvb( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte > 59 )
            {
                break;
            }
            
            /*
            || Retrieve the record format
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            byte = TO_A_SYS( byte );
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte != 'F' && byte != 'V' )
            {
                break;
            }
            
            /*
            || Retrieve the flags
            */
            byte = get( vma );
            if( byte == EOF )
            {
                return FALSE;
            }
            vma->head[ cnt++ ] = byte;
            
            /*
            || Verify
            */
            if( byte & ~( HF_S2 | HF_ASIS | HF_Y2K | HF_EXTH ) )
            {
                break;
            }
            
            /*
            || Get the extended header
            */
            if( byte & HF_EXTH )
            {
                for( ndx = 0; ndx < 4; ndx++ )
                {
                    byte = get( vma );
                    if( byte == EOF )
                    {
                        return FALSE;
                    }
                    vma->head[ cnt++ ] = byte;
                }
                
                for( ndx = 0; ndx < 8; ndx++ )
                {
                    byte = get( vma );
                    if( byte == EOF )
                    {
                        return FALSE;
                    }
                    vma->head[ cnt++ ] = byte;
                }
            }
            
            /*
            || Looks like we've found a new header
            */
            return TRUE;
            
        } while( FALSE );
        
        unget( vma, vma->head, cnt );
    }
    
    return FALSE;
}

/* --------------------------------------------------------------------
|| Determines if we're running under z/VM or z/OS
*/
static void
systype( VMA *vma )
{
#if defined( __MVS__ )
    /*
    || The utsname structures on z/OS and z/VM differ in size, so to
    || be able to run a z/OS compiled module under z/VM, we add some
    || padding.
    */
    union
    {
        struct utsname un;
        char dummy[ 256 ];
    } u;
 
    memset( &u.un, 0, sizeof( u ) );
 
    uname( &u.un );
 
    if( strcmp( u.un.sysname, "OS/390" ) == 0 )
    {
        vma->f_zos = TRUE;
    }
    else if( strcmp( u.un.sysname, "z/VM" ) == 0 )
    {
        vma->f_zvm = TRUE;
    }
 
#endif
    return;
}
 
/* ********************************************************************
|| Public functions
******************************************************************** */

/* ====================================================================
|| Returns ptr to error text
*/
const char *
vma_strerror( int ec )
{
    if( ec < 0 || ec > VMAE_NUMERRORS )
    {
        return "Invalid error code passed to vma_strerror()";
    }
    
    return etext[ ec ];
}

/* ====================================================================
|| Builds conversion table based on UCMs
*/
int
vma_setconv( void *vvma, const char *fucm, const char *tucm )
{
    VMA *vma = (VMA *)vvma;
    int i;
    int subchar;
    ENT tmap[ 256 ];
    ENT fmap[ 256 ];
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify "from" UCM
    */
    if( fucm == NULL && tucm != NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Verify "to" UCM
    */
    if( tucm == NULL && fucm != NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || If both are null, then reset to default tables
    */
    if( tucm == NULL && fucm == NULL )
    {
#if defined(__MVS__)
        memcpy( vma->e2a_map, e2e_tab, 256 );
        memcpy( vma->a2e_map, a2e_tab, 256 );
#else
        memcpy( vma->e2a_map, e2a_tab, 256 );
        memcpy( vma->a2e_map, a2e_tab, 256 );
#endif
        
        return VMAE_NOERR;
    }
    
    /*
    || Load "from" UCM
    */
    if( !loaducm( vma, fucm, fmap, &subchar ) )
    {
        return vma->lasterr;
    }
    
    /*
    || Load "to" UCM
    */
    if( !loaducm( vma, tucm, tmap, &subchar ) )
    {
        return vma->lasterr;
    }
    
    /*
    || Make sure the maps are sorted
    */
    qsort( &fmap, 256, sizeof( ENT ), cmp );
    qsort( &tmap, 256, sizeof( ENT ), cmp );
    
    /*
    || Transfer to caller's mapping
    */
    for( i = 0; i < 256; i++ )
    {
        ENT *ent;
        
        ent = (ENT *) bsearch( &fmap[ i ],
                              tmap,
                              256,
                              sizeof( ENT ),
                              cmp );
        if( ent == NULL )
        {
            vma->e2a_map[ i ] = subchar;
        }
        else
        {
            vma->e2a_map[ i ] = ent->b;
        }
    }
    
    /*
    || Transfer to caller's reverse mapping
    */
    for( i = 0; i < 256; i++ )
    {
        ENT *ent;
        
        ent = (ENT *) bsearch( &tmap[ i ],
                              fmap,
                              256,
                              sizeof( ENT ),
                              cmp );
        if( ent == NULL )
        {
            vma->a2e_map[ i ] = subchar;
        }
        else
        {
            vma->a2e_map[ i ] = ent->b;
        }
    }
    
    return VMAE_NOERR;
}

static const int days[] =
{
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

/* ====================================================================
|| Extract currently active subfile
*/
int
vma_extract( void *vvma, const char *name )
{
    VMA *vma = (VMA *)vvma;
    PSUBFILE *psf;
    char openflags[ 64 ];
    struct tm bt;
    struct utimbuf ut;
    time_t ct;
    int rc;
    int mode = vma->mode;
#if defined( __MVS__ )
    fldata_t fd;
#endif
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify name
    */
    if( name == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Reset error
    */
    seterr( VMAE_NOERR );
    
    /*
    || Determine which file the subfile resides in.
    */
    vma->in = ( psf->temp ? vma->tfile : vma->vfile );
    
    /*
    || If user requested AUTO and we haven't previously scanned the
    || subfile, then we must do so now.
    */
    if( mode == VMAX_AUTO )
    {
        if( psf->sf.dtype == VMAD_UNKNOWN )
        {
            /*
            || Don't know what type it is yet.
            */
            vma->f_text = FALSE;
            
            /*
            || Turn extraction off
            */
            vma->f_extract = FALSE;
            
            /*
            || And indicate we only want to scan first 1024 bytes
            */
            vma->f_scanning = TRUE;
            
            /*
            || Perform a simulated extraction
            */
            rc = extract( vma );
            
            /*
            || Disable scanning mode
            */
            vma->f_scanning = FALSE;
            
            /*
            || Bail if the extraction wasn't successful
            */
            if( !rc )
            {
                return vma->lasterr;
            }
            
            /*
            || Remember it
            */
            psf->sf.dtype = vma->dtype;
        }
        
        /*
        || Set the extraction type
        */
        mode = psf->sf.dtype;
    }
    
    /*
    || Turn extraction on
    */
    vma->f_extract = TRUE;
    
    /*
    || Convert to ASCII?
    */
    vma->f_text = FALSE;
    if( mode == VMAX_TEXT )
    {
        vma->f_text = TRUE;
    }
    
    /*
    || Cache RECFM and LRECL
    */
    vma->recfm = psf->sf.recfm;
    vma->lrecl = psf->sf.lrecl;
    
    /*
    || Calculate the maximum size of the output buffer
    */
    vma->omax = ( vma->recfm == 'F' ? vma->lrecl : vma->lrecl + 4 ) +
#if defined( _WIN32 )
    ( vma->f_text ? 2 : 0 );
#else
    ( vma->f_text ? 1 : 0 );
#endif
    
#if defined(__MVS__)
    /*
    || Ensure LRECL is in range
    */
    if( vma->f_zos )
    {
        if( vma->recfm == 'V' && vma->omax + 4 > 32760 )
        {
            return seterr( VMAE_LRECL );
        }
    }
    else if( vma->f_zvm )
    {
        if( vma->omax > 65535 )
        {
            return seterr( VMAE_LRECL );
        }
    }
#endif
    
    /*
    || Build the open flags
    */
    sprintf( openflags,
             "wb"
#if defined( __MVS__ )
             ",type=record,noseek,recfm=%c,lrecl=%d,blksize=0",
             vma->recfm,
             vma->omax
#endif
           );
 
    
    /*
    || And open the output file
    */
    vma->out = fopen( name, openflags );
    if( vma->out == NULL )
    {
        return seterr( VMAE_OOPEN );
    }
    
    /*
    || (Re)Alocate output buffer
    */
    vma->obuf = (uchar *) malloc( vma->omax + 1 );
    if( vma->obuf == NULL )
    {
        fclose( vma->out );
        vma->out = NULL;
        
        return seterr( VMAE_MEM );
    }
    vma->opos = 0;
    
    /*
    || Extract the file
    */
    rc = extract( vma );
    
    /*
    || Get rid of the buffer
    */
    free( vma->obuf );
    vma->obuf = NULL;
    
#if defined( __MVS__ )
    /*
    || Retrieve the real filename before closing the file
    */
    if( rc && vma->f_zvm )
    {
        fldata( vma->out, NULL, &fd );
    }
#endif
 
    /*
    || Close the file
    */
    fclose( vma->out );
    vma->out = NULL;
    
    /*
    || Attempt to set the file times
    */
    if( rc )
    {
#if defined( __MVS__ )
        if( vma->f_zos )
        {
            /* not much we can do here */
        }
        else if( vma->f_zvm )
        {
            char cmd[ 256 ];
 
            sprintf( cmd,
                     "DMSPLU "
                     "%s "
                     "%02d/%02d/%04d "
                     "%02d:%02d:%02d",
                     fd.__dsname,
                     vma->active->sf.month,
                     vma->active->sf.day,
                     vma->active->sf.year,
                     vma->active->sf.hour,
                     vma->active->sf.minute,
                     vma->active->sf.second );
            system( cmd );
        }
        else
#endif
        {
        /*
        || Set the last access and modification times
        */
        memset( &bt, 0, sizeof( bt ) );
        bt.tm_sec = psf->sf.second;
        bt.tm_min = psf->sf.minute;
        bt.tm_hour = psf->sf.hour;
        bt.tm_mday = psf->sf.day;
        bt.tm_mon = psf->sf.month - 1;
        bt.tm_year = psf->sf.year - 1900;
        bt.tm_wday = 0;
        bt.tm_yday = 0;
        bt.tm_isdst = -1;
        
        ct = mktime( &bt );
        if( ct != (time_t) -1 )
        {
            ut.actime = ct;
            ut.modtime = ct;
            
            utime( name, &ut );
        }
    }
    }
    
    return vma->lasterr;
}

/* ====================================================================
||
*/
int
vma_getactive( void *vvma, SUBFILE **sfp )
{
    VMA *vma = (VMA *) vvma;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify subfile
    */
    if( sfp == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Get the active subfile
    */
    *sfp = &vma->active->sf;
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_setactive( void *vvma, SUBFILE *sf )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify subfile
    */
    if( sf == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Make this subfile the active one
    */
    for( psf = vma->subfiles; psf != NULL; psf = psf->next )
    {
        if( &psf->sf == sf )
        {
            break;
        }
    }
    
    /*
    || Was the subfile in the list?
    */
    if( psf == NULL )
    {
        return seterr( VMAE_NOTFOUND );
    }
    
    /*
    || Set the active subfile
    */
    set_active( vma, psf );
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_first( void *vvma, SUBFILE **sfp )
{
    VMA *vma = (VMA *) vvma;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify subfile
    */
    if( sfp == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Make this subfile the active one
    */
    set_active( vma, vma->subfiles );
    
    /*
    || Processed all subfiles
    */
    if( vma->active == NULL )
    {
        return seterr( VMAE_NOMORE );
    }
    
    /*
    || Store subfile ptr (possibly NULL)
    */
    *sfp = &vma->active->sf;
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_next( void *vvma, SUBFILE **sfp )
{
    VMA *vma = (VMA *) vvma;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify subfile
    */
    if( sfp == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Reset error
    */
    seterr( VMAE_NOERR );
    
    if( vma->active == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Set the next active file
    */
    set_active( vma, vma->active->next );
    
    /*
    || Processed all subfiles
    */
    if( vma->active == NULL )
    {
        return seterr( VMAE_NOMORE );
    }
    
    /*
    || Store subfile ptr (possibly NULL)
    */
    *sfp = &vma->active->sf;
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_isdirty( void *vvma, int *dirty )
{
    VMA *vma = (VMA *) vvma;
    
    /*
    || Bail if we weren't passed a VMA
    */
    if( vma == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Don't do any good if we don't have a "dirty" arg
    */
    if( dirty == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Return the current cleaniness of the archive
    */
    *dirty = vma->f_dirty;
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_commit( void *vvma )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    char *mname = NULL;
    FILE *mfile = NULL;
    char sfx[] = ".merged.XXXXXX";
    struct stat st;

    /*
    || Bail if we weren't passed a VMA
    */
    if( vma == NULL )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Nothing to do if not dirty
    */
    if( !vma->f_dirty )
    {
        return seterr( VMAE_NOERR );
    }
    
    /*
    || Allocate memory for new name
    */
    mname = malloc( strlen( vma->vname ) + sizeof( sfx ) );
    if( mname == NULL )
    {
        seterr( VMAE_MEM );
        goto error;
    }
        
    /*
    || Build new name
    */
    strcpy( mname, vma->vname );
    strcat( mname, sfx );
    if( mktemp( mname ) == NULL )
    {
        seterr( VMAE_WERR );
        goto error;
    }
        
    /*
    || Get the current file permissions
    */
    if( vma->vfile == NULL || fstat( fileno( vma->vfile ), &st ) != 0 )
    {
        st.st_mode = PERMS;
    }
    
    /*
    || Open with exclusive access
    */
    mfile = open_exclusive( vma, mname, st.st_mode );
    if( mfile == NULL )
    {
        seterr( VMAE_TOPEN );
        goto error;
    }

    /*
    || Copy subfiles from original and temp archives
    */
    for( psf = vma->subfiles; psf != NULL; psf = psf->next )
    {
        FILE *from;
        size_t bytes;
        size_t len;

        /*
        || Determine where the subfile currently lives
        */
        if( psf->temp )
        {
            from = vma->tfile;
        }
        else
        {
            from = vma->vfile;
        }

        /*
        || Position to start of subfile data
        */
        if( fseek( from, psf->dataoff, SEEK_SET ) != 0 )
        {
            seterr( VMAE_RERR );
            goto error;
        }
        
        /*
        || Write the subfile header
        */
        if( write_header( vma, mfile, psf ) != VMAE_NOERR )
        {
            goto error;
        }

        /*
        || Copy the subfile data to the merged archive
        */
        bytes = psf->sf.compressed;
        for( bytes = psf->sf.compressed; bytes > 0; bytes -= len )
        {
            len = bytes < BUFLEN ? bytes : BUFLEN;
            len = fread( vma->ibuf, 1, len, from );
            if( ferror( from ) || feof( from ) )
            {
                seterr( VMAE_RERR );
                goto error;
            }
            
            if( len != 0 )
            {
                if( fwrite( vma->ibuf, 1, len, mfile ) != len )
                {
                    seterr( VMAE_WERR );
                    goto error;
                }
            }
        }
        
        /*
        || Write the trailer
        */
        if( write_trailer( vma, mfile ) != VMAE_NOERR )
        {
            goto error;
        }
    }

    if( fclose( mfile ) != 0 )
    {
        mfile = NULL;
        seterr( VMAE_WERR );
        goto error;
    }
    mfile = NULL;

    /*
    || All changes have been successfully written to the final archive.
    || Any errors from this point on are non-fatal, but the user may
    || have to manually rename the "<archive name>.merged.XXXXXX" to
    || the desired name.
    */
    vma->f_dirty = FALSE;

    /*
    || Clean up the subfile entries
    */
    for( psf = vma->subfiles; psf != NULL; psf = psf->next )
    {
        psf->dataoff = psf->dataofftmp;
        psf->temp = FALSE;
        psf->dirty = FALSE;
    }

    /*
    || Temp archive is no longer needed
    */
    if( vma->tfile )
    {
        fclose( vma->tfile );
        vma->tfile = NULL;

        unlink( vma->tname );

        free( vma->tname );
        vma->tname = NULL;
    }

    /*
    || Original archive is no longer needed
    */
    if( vma->vfile )
    {
        fclose( vma->vfile );
        vma->vfile = NULL;

        unlink( vma->vname );
    }

    /*
    || If the rename fails, we still want to have a valid open file
    || handle, so forget about the original archive name and start
    || using the new (merged) archive name.
    */
    if( rename( mname, vma->vname ) != 0 )
    {
        free( vma->vname );
        vma->vname = mname;
        mname = NULL;

        seterr( VMAE_RENAME );
    }
    else
    {
        free( mname );
    }
    mname = NULL;

    /*
    || Open the new VMARC file
    */
    vma->vfile = fopen( vma->vname, "rb" );
    if( vma->vfile == NULL )
    {
        return seterr( VMAE_IOPEN );
    }
    
    return seterr( VMAE_NOERR );

error:

    if( mfile )
    {
        fclose( mfile );
    }

    if( mname )
    {
        unlink( mname );
        free( mname );
    }

    return vma->lasterr;
}

/* ====================================================================
||
*/
int
vma_setmode( void *vvma, int mode )
{
    VMA *vma = (VMA *) vvma;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Verify mode
    */
    switch( mode )
    {
        case VMAX_AUTO:
        case VMAX_TEXT:
        case VMAX_TRANS:
        case VMAX_BINARY:
            break;
            
        default:
            return seterr( VMAE_BADARG );
    }
    
    /*
    || Set the mode
    */
    vma->mode = mode;
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
void
vma_close( void *vvma )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Bail if we weren't passed a VMA
    */
    if( vma == NULL )
    {
        return;
    }
    
    /*
    || Clean up the temp file
    */
    if( vma->tfile != NULL )
    {
        fclose( vma->tfile );
        unlink( vma->tname );
    }
    
    /*
    || Free the temp file name
    */
    if( vma->tname != NULL )
    {
        free( vma->tname );
    }
    
    /*
    || Close the input file
    */
    if( vma->vfile != NULL )
    {
        fclose( vma->vfile );
    }
    
    /*
    || Close the output file
    */
    if( vma->out != NULL )
    {
        fclose( vma->out );
    }
    
    /*
    || Free all of the SUBFILEs
    */
    while( vma->subfiles != NULL )
    {
        psf = vma->subfiles;
        vma->subfiles = psf->next;
        
        free( psf );
    }
    
    /*
    || Free the output buffer
    */
    if( vma->obuf != NULL )
    {
        free( vma->obuf );
    }
    
    /*
    || Free the file name
    */
    if( vma->vname != NULL )
    {
        free( vma->vname );
    }
    
    /*
    || And, finally, free the VMA itself
    */
    free( vma );
    
    return;
}

/* ====================================================================
||
*/
int
vma_open( const char *name, void **vvma )
{
    VMA *vma = NULL;
    PSUBFILE *psf;
    PSUBFILE *lpsf;
    int i;
    int ec = VMAE_NOERR;
    
    /*
    || Verify args
    */
    if( name == NULL || vvma == NULL )
    {
        ec = VMAE_BADARG;
        goto error;
    }
    
    /*
    || Initialize vvma
    */
    *vvma = NULL;
    
    /*
    || Allocate our handle
    */
    vma = (VMA *) calloc( 1, sizeof( VMA ) );
    if( vma == NULL )
    {
        ec = VMAE_MEM;
        goto error;
    }
    
    /*
    || Remember the file name
    */
    vma->vname = strdup( name );
    if( vma->vname == NULL )
    {
        ec = VMAE_MEM;
        goto error;
    }
    
    /*
    || Set default conversion table
    */
    vma_setconv( vma, NULL, NULL );
    
    /*
    || Get the system type
    */
    systype( vma );

    /*
    || Open the VMARC file
    */
    vma->vfile = fopen( name, "rb" );
    if( vma->vfile == NULL )
    {
        /*
        || Just assume they're going to create a new file
        */
        *( (VMA **) vvma ) = vma;
        
        return seterr( VMAE_NOERR );
    }
    
    /*
    || Set active input file
    */
    vma->in = vma->vfile;
    
    /*
    || Turn off extraction
    */
    vma->f_extract = FALSE;
    
    /*
    || Build list of subfiles
    */
    lpsf = NULL;
    while( locate_file( vma ) )
    {
        /*
        || Allocate a new subfile
        */
        if( vma_new( vma, NULL ) != VMAE_NOERR )
        {
            ec = VMAE_MEM;
            goto error;
        }
        
        /*
        || Get the newly allocated subfile
        */
        psf = vma->active;
        
        /*
        || Copy header to subfile
        */
        psf->sf.ver     = vma->head[ H_VER ];
        psf->sf.rel     = vma->head[ H_REL ];
        
        sprintf( (char *) psf->sf.meth,
                "%s",
                ( vma->head[ H_FLAGS ] & HF_ASIS ? "ASIS" :
                 ( vma->head[ H_FLAGS ] & HF_S2 ? "S2" :
                  "LZW" ) ) );
        for( i = 0; i < 8 && vma->head[ H_FN + i ] != ' '; i++ )
        {
            psf->sf.fn[ i ] = vma->head[ H_FN + i ];
        }
        psf->sf.fn[ i ] = '\0';
        
        for( i = 0; i < 8 && vma->head[ H_FT + i ] != ' '; i++ )
        {
            psf->sf.ft[ i ] = vma->head[ H_FT + i ];
        }
        psf->sf.ft[ i ] = '\0';
        
        for( i = 0; i < 2 && vma->head[ H_FM + i ] != ' '; i++ )
        {
            psf->sf.fm[ i ] = vma->head[ H_FM + i ];
        }
        psf->sf.fm[ i ] = '\0';
        
        psf->sf.year = vma->head[ H_YEAR ] + 1900 +
        ( ( vma->head[ H_FLAGS ] & HF_Y2K ) ? 100 : 0 );
        psf->sf.month   = vma->head[ H_MONTH ];
        psf->sf.day     = vma->head[ H_DAY ];
        psf->sf.hour    = vma->head[ H_HOUR ];
        psf->sf.minute  = vma->head[ H_MINUTE ];
        psf->sf.second  = vma->head[ H_SECOND ];
        psf->sf.recfm   = vma->head[ H_RECFM ];
        psf->flags      = vma->head[ H_FLAGS ] & ( HF_S2 | HF_ASIS );
        
        /*
        || Hack for non-Y2K compliant files.  Yes, they ARE still being
        || created!  Come on folks, upgrade your VMARC to at least
        || V1R2P021.  :-)
        */
        if( psf->sf.year < 1960 )
        {
            psf->sf.year += 100;
        }
        
        /*
        || Construct the LRECL
        */
        if( vma->head[ H_FLAGS ] & HF_EXTH )
        {
            psf->sf.lrecl = ( vma->head[ H_XRECL + 0 ] << 24 ) |
            ( vma->head[ H_XRECL + 1 ] << 16 ) |
            ( vma->head[ H_XRECL + 2 ] << 8  ) |
            ( vma->head[ H_XRECL + 3 ]       );
        }
        else
        {
            psf->sf.lrecl = ( vma->head[ H_LRECL + 0 ] << 8  ) |
            ( vma->head[ H_LRECL + 1 ]       );
        }
        
        /*
        || Remember where the data starts
        */
        psf->dataoff = mytell( vma );

        /*
        || Retrieve the sizes
        */
        set_active( vma, psf );
        if( !extract( vma ) )
        {
            ec = vma->lasterr;
            goto error;
        }
        set_active( vma, NULL );
        
        /*
        || Prevent modification of certain header fields
        */
        psf->locked = TRUE;
    }
    
    /*
    || Locate file doesn't check for errors, just EOF
    */
    if( ferror( vma->vfile ) )
    {
        ec = VMAE_RERR;
        goto error;
    }
    
    /*
    || See if any errors are outstanding
    */
    if( vma->lasterr )
    {
        ec = vma->lasterr;
        goto error;
    }
    
    /*
    || Store the VMA ptr
    */
    *( (VMA **) vvma ) = vma;
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
    
error:
    
    /*
    || Use vma_close() to cleanup
    */
    vma_close( vma );
    
    return ec;
}

/* ====================================================================
||
*/
int
vma_new( void *vvma, SUBFILE **sfp )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Allocate a new subfile
    */
    psf = (PSUBFILE *) calloc( 1, sizeof( PSUBFILE ) );
    if( psf == NULL )
    {
        return seterr( VMAE_MEM );
    }
    
    /*
    || Link it in
    */
    if( vma->sflast == NULL )
    {
        vma->subfiles = psf;
    }
    else
    {
        vma->sflast->next = psf;
    }
    vma->sflast = psf;
    
    /*
    || Make this subfile the active one
    */
    set_active( vma, psf );
    
    /*
    || Set defaults
    */
    psf->sf.ver = 1;
    psf->sf.rel = 0;
    strcpy( (char *) psf->sf.meth, VMAM_ASIS );
    strcpy( (char *) psf->sf.fn, "TEMPNAME" );
    strcpy( (char *) psf->sf.ft, "$DEFAULT" );
    strcpy( (char *) psf->sf.fm, "A1" );
    psf->sf.lrecl = 65535;
    psf->sf.year = 1900;
    psf->sf.month = 1;
    psf->sf.day = 1;
    psf->sf.hour = 0;
    psf->sf.minute = 0;
    psf->sf.second = 0;
    psf->sf.recfm = VMAR_VARIABLE;
    psf->sf.compressed = 0;
    psf->sf.uncompressed = 0;
    psf->sf.dtype = VMAD_UNKNOWN;
    
    /*
    || Store subfile ptr
    */
    if( sfp != NULL )
    {
        *sfp = &vma->active->sf;
    }
    
    /*
    || Success
    */
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_setname( void *vvma, const char *fn, const char *ft, const char *fm )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Verify the file name
    */
    if( fn != NULL && !validate_name( fn ) )
    {
        return seterr( VMAE_BADNAME );
    }
    
    /*
    || Verify the file type
    */
    if( ft != NULL && !validate_name( ft ) )
    {
        return seterr( VMAE_BADTYPE );
    }
    
    /*
    || Verify the file mode
    */
    if( fm != NULL )
    {
        size_t len = strlen( fm );
        
        if( len != 2 )
        {
            return seterr( VMAE_BADMODE );
        }
        
        switch( fm[ 0 ] )
        {
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'Y':
            case 'Z':
                if( isdigit( fm[ 1 ] ) )
                {
                    break;
                }
                /* fall through*/
                
            default:
                return seterr( VMAE_BADMODE );
                break;
        }
    }
    
    /*
    || Set the file name
    */
    if( fn != NULL )
    {
        strcpy( (char *) psf->sf.fn, fn );
        mark_dirty( vma, psf );
    }
    
    /*
    || Set the file type
    */
    if( ft != NULL )
    {
        strcpy( (char *) psf->sf.ft, ft );
        mark_dirty( vma, psf );
    }
    
    /*
    || Set the file mode
    */
    if( fm != NULL )
    {
        strcpy( (char *) psf->sf.fm, fm );
        mark_dirty( vma, psf );
    }
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_setdate( void *vvma, int year, int month, int day )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Verify the year
    */
    if( year < 1900 )
    {
        return seterr( VMAE_BADYEAR );
    }
    
    /*
    || Verify the month
    */
    if( month < 1 || month > 12 )
    {
        return seterr( VMAE_BADMONTH );
    }
    
    /*
    || Verify the day (I know, I know...lazy)
    */
    if( day < 1 || day > 31 )
    {
        return seterr( VMAE_BADDAY );
    }
    
    psf->sf.year = year;
    psf->sf.month = month;
    psf->sf.day = day;
    
    mark_dirty( vma, psf );
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_settime( void *vvma, int hour, int minute, int second )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Verify the hour
    */
    if( hour < 0 || hour > 23 )
    {
        return seterr( VMAE_BADHOUR );
    }
    
    /*
    || Verify the minute
    */
    if( minute < 0 || minute > 59 )
    {
        return seterr( VMAE_BADMINS );
    }
    
    /*
    || Verify the second
    */
    if( second < 0 || second > 59 )
    {
        return seterr( VMAE_BADSECS );
    }
    
    psf->sf.hour = hour;
    psf->sf.minute = minute;
    psf->sf.second = second;
    
    mark_dirty( vma, psf );
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_setrecfm( void *vvma, char recfm )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Verify the record format
    */
    if( recfm != VMAR_FIXED && recfm != VMAR_VARIABLE )
    {
        return seterr( VMAE_BADRECFM );
    }
    
    /*
    || Can only set recfm on new subfiles, not existing ones.
    */
    if( psf->locked )
    {
        return seterr( VMAE_NOTMOD );
    }
    
    psf->sf.recfm = recfm;
    
    mark_dirty( vma, psf );
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_setlrecl( void *vvma, int lrecl )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Verify the record format (yes I know, VMARCs support longer)
    */
    if( lrecl < 1 || lrecl > 65535 )
    {
        return seterr( VMAE_BADLRECL );
    }
    
    /*
    || Can only set lrecl on new subfiles, not existing ones.
    */
    /*
    || Can only set recfm on new subfiles, not existing ones.
    */
    if( psf->locked )
    {
        return seterr( VMAE_NOTMOD );
    }
    
    psf->sf.lrecl = lrecl;
    
    mark_dirty( vma, psf );
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_setmethod( void *vvma, const char *method )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Verify method
    */
    if( strcmp( method, VMAM_ASIS ) != 0 &&
       strcmp( method, VMAM_LZW ) != 0 &&
       strcmp( method, VMAM_S2 ) != 0 )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Can only set recfm on new subfiles, not existing ones.
    */
    /*
    || Can only set recfm on new subfiles, not existing ones.
    */
    if( psf->locked )
    {
        return seterr( VMAE_NOTMOD );
    }
    
    strcpy( psf->sf.meth, method );
    
    /*
    || Set subfile flags
    */
    psf->sf.rel = 0;
    if( strcmp( method, VMAM_ASIS ) == 0 )
    {
        psf->flags = HF_ASIS;
        psf->sf.rel = 1;
    }
    else if( strcmp( method, VMAM_S2 ) == 0 )
    {
        psf->flags = HF_S2;
    }
    
    mark_dirty( vma, psf );
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_add( void *vvma, const char *name )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    FILE *f;
    int mode = vma->mode;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Ensure the subfile is fresh (maybe remove to do subfile replaces)
    */
    if( psf->dataoff != 0 )
    {
        return seterr( VMAE_BADARG );
    }
    
    /*
    || Verify name
    */
    if( name == NULL )
    {
        return seterr( VMAE_BADFILE );
    }
    
    /*
    || Open input file
    */
    f = fopen( name, "rb" );
    if( f == NULL )
    {
        return seterr( VMAE_IOPEN );
    }
    
    /*
    || Try to determine the file type
    */
    if( mode == VMAX_AUTO )
    {
        char buf[ 1024 ];
        size_t actual = fread( buf, 1, 1024, f );
        size_t i;
        
        if( ferror( f ) )
        {
            return seterr( VMAE_RERR );
        }
        
        mode = VMAX_TEXT;
        for( i = 0; i < actual; i++ )
        {
            if( buf[ i ] < 0x20 )
            {
                if( buf[ i ] != '\t' && buf[ i ] != '\r' && buf[ i ] != '\n' )
                {
                    mode = VMAX_BINARY;
                }
            }
        }
        
        rewind( f );
    }
    
    /*
    || Convert to ASCII?
    */
    vma->f_text = FALSE;
    if( mode == VMAX_TEXT )
    {
        vma->f_text = TRUE;
    }
    
    /*
    || Cache RECFM and LRECL
    */
    vma->recfm = psf->sf.recfm;
    vma->lrecl = psf->sf.lrecl;
    
    /*
    || Reset various counters and I/O controls
    */
    vma->ibufp = 0;
    vma->icnt = 0;
    vma->bytesin = 0;
    vma->bytesout = 0;
    vma->residual = UINT_MAX;
    vma->recbytes = 0;
    vma->eor = 0;
    vma->omax = 0;
    
    /*
    || Make sure the temp file is open and positioned properly
    */
    if( open_temp( vma ) != VMAE_NOERR )
    {
        return vma->lasterr;
    }
    
    /*
    || Remember data offset
    */
    psf->dataoff = ftell( vma->tfile );
    
    /*
    || Store based on type
    */
    if( strcmp( psf->sf.meth, VMAM_ASIS ) == 0 )
    {
        add_asis( vma, f, mode );
    }
    else if( strcmp( psf->sf.meth, VMAM_LZW ) == 0 )
    {
        add_lzw( vma, f, mode );
    }
    else if( strcmp( psf->sf.meth, VMAM_S2 ) == 0 )
    {
        /* force lzw until s2 is added */
        strcpy( psf->sf.meth, VMAM_LZW );
        add_lzw( vma, f, mode );
    }

    /*
    || Finalize subfile fields
    */
    psf->sf.dtype = mode;
    psf->sf.lrecl = (int) ( vma->recfm == VMAR_FIXED ? vma->lrecl : vma->omax );
    psf->sf.compressed = vma->bytesout;
    psf->sf.uncompressed = vma->bytesin;
    psf->temp = TRUE;
    
    return VMAE_NOERR;
}

/* ====================================================================
||
*/
int
vma_delete( void *vvma )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    PSUBFILE *lpsf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    if( vma->active == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Find the subfile preceeding the active one
    */
    lpsf = NULL;
    for( psf = vma->subfiles; psf != NULL; psf = psf->next )
    {
        if( psf == vma->active )
        {
            break;
        }
        
        lpsf = psf;
    }
    
    /*
    || Unlink it
    */
    if( lpsf == NULL )
    {
        vma->subfiles = psf->next;
    }
    else
    {
        lpsf->next = psf->next;
    }
    
    if( psf == vma->sflast )
    {
        vma->sflast = lpsf;
    }
    
    /*
    || No active subfile now
    */
    set_active( vma, NULL );
    
    /*
    || Free the memory
    */
    free( psf );
    
    /*
    || Mark archive dirty
    */
    mark_dirty( vma, NULL );
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_retain( void *vvma )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Make sure something is not already retained
    */
    if( vma->sfretained != NULL )
    {
        return seterr( VMAE_RETAINED );
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Retain the active subfile
    */
    memcpy( &vma->sfsave, psf, sizeof( vma->sfsave ) );
    
    /*
    || Remember which subfile was retained
    */
    vma->sfretained = psf;
    
    /*
    || And remember if the archive was dirty.
    */
    vma->f_retdirty = vma->f_dirty;
    
    return seterr( VMAE_NOERR );
}

/* ====================================================================
||
*/
int
vma_release( void *vvma, int discard )
{
    VMA *vma = (VMA *) vvma;
    PSUBFILE *psf;
    
    /*
    || Verify VMA
    */
    if( vma == NULL )
    {
        return VMAE_BADARG;
    }
    
    /*
    || Ensure an active subfile
    */
    psf = vma->active;
    if( psf == NULL )
    {
        return seterr( VMAE_INACT );
    }
    
    /*
    || Make sure we're releasing the same subfile that was retained
    */
    if( vma->sfretained != psf )
    {
        return seterr( VMAE_NOTRET );
    }
    
    /*
    || Restore the subfile if the changes are to be discarded
    */
    if( discard )
    {
        memcpy( psf, &vma->sfsave, sizeof( *psf ) );
        vma->f_dirty = vma->f_retdirty;
    }
    
    /*
    || No longer retained
    */
    vma->sfretained = NULL;
    
    return seterr( VMAE_NOERR );
}
