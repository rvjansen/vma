/* ====================================================================
||
|| vma - A utility to manage contents of VMARC archive files
||
|| Usage: vma -x [options] archive [fn [ft [fm ]]]
||        vma -a [options] archive file[,fn.[ft.[fm]]] ...
||
|| Options:
||   -a        add files to archive
||   -c        convert names to lowercase
||   -h        display usage summary
||   -l        record length...1 to 65535
||   -m fm     replace filemode...0=remove
||   -q        do not list files
||   -r        record format...f=fixed, v=variable
||   -s        store method...asis, lzw, s2
||   -t        translate files to ASCII on extration
||             or to EBCDIC on addition
||   -u f,t    specifies (f)rom and (t) UCM filenames
||   -v        verbose listing
||   -x        extract files
||   -V        display version
||
|| archive:
||   name of the VMARC archive
||
|| fn, ft, fm:
||   filter on file name, type, and/or mode during extraction
||   (case is significant)
||
|| file[,fn.[ft.[fm]]]:
||   one or more file names to be added to the VMARC archive
||
|| To compile with GCC:
||
||   see Makefile
||
==================================================================== */

#define _ALL_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#if !defined(_MSC_VER)
#include <utime.h>
#include <unistd.h>
#endif

#include <ctype.h>

#include "version.h"
#include "vmalib.h"

/* --------------------------------------------------------------------
|| Silly getopt stuff
*/

#if defined(_MSC_VER)
/* getopt.c:
 * a public domain implementation of getopt()
 *
 * The following source code is an adaptation of the public domain getopt()
 * implementation presented at the 1985 UNIFORUM conference in Dallas,
 * Texas. Slight edits have been made to improve readability and the result
 * is released into the public domain like that from which it was derived.
 */

#include <stdio.h>
#include <string.h>

int optind = 1;
int optopt;
char *optarg;

int
getopt(int argc, char **argv, char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    if (sp == 1) {

        /* If all args are processed, finish */
        if (optind >= argc) {
            return EOF;
        }
        if (argv[optind][0] != '-' || argv[optind][1] == '\0') {
            return EOF;
        }

    } else if (!strcmp(argv[optind], "--")) {

        /* No more options to be processed after this one */
        optind++;
        return EOF;

    }

    optopt = c = argv[optind][sp];

    /* Check for invalid option */
    if (c == ':' || (cp = strchr(opts, c)) == NULL) {

        fprintf(stderr,
                "%s: illegal option -- %c\n",
                argv[0],
                c);
        if (argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }

        return '?';
    }

    /* Does this option require an argument? */
    if (*++cp == ':') {

        /* If so, get argument; if none provided output error */
        if (argv[optind][sp+1] != '\0') {
            optarg = &argv[optind++][sp+1];
        } else if (++optind >= argc) {
            fprintf(stderr,
                    "%s: option requires an argument -- %c\n",
                    argv[0],
                    c);
            sp = 1;
            return '?';
        } else {
            optarg = argv[optind++];
        }
        sp = 1;

    } else {
        if (argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }

    return c;
}
#elif defined(__MVS__)
#define optind *( __opindf() )
#define optarg *( __opargf() )
#else
#include <getopt.h>
#endif

/* --------------------------------------------------------------------
|| General stuff
*/
#if !defined( NULL )
#define NULL 0
#endif

#if !defined( TRUE )
#define TRUE 1
#endif

#if !defined( FALSE )
#define FALSE 0
#endif

/* --------------------------------------------------------------------
|| (naive) EBCDIC -> ASCII stuff
*/

/* --------------------------------------------------------------------
|| Process control stuff
*/
static char f_add     = FALSE;              /* add files             */
static char f_case    = FALSE;              /* names > lowercase     */
static char f_list    = TRUE;               /* list subfile info     */
static char f_verbose = FALSE;              /* enable verbose output */
static char f_version = FALSE;              /* display version       */
static char f_extract = FALSE;              /* extract subfiles      */
static int  xmode     = VMAX_BINARY;        /* extraction mode       */
static int  lrecl     = 65535;              /* record length         */
static char recfm     = VMAR_VARIABLE;      /* record format         */
static char *s_meth   = VMAM_LZW;           /* store method          */
static char *s_mode   = NULL;               /* convert mode to...    */
static char *s_fn     = "*";                /* file name filter      */
static char *s_ft     = "*";                /* file type filter      */
static char *s_fm     = "*";                /* file mode filter      */
static char *s_filter;                      /* filter string         */
static char *s_fucm   = NULL;               /* from UCM charmap      */
static char *s_tucm   = NULL;               /* to UCM charmap        */
static size_t i_flen;                       /* len of filter         */
static char *s_name;                        /* output name           */
static size_t i_nlen;                       /* len of name           */

#if defined( _WIN32 )
/* --------------------------------------------------------------------
|| Prevent MinGW automatic command line globbing
*/
int _CRT_glob = 0;
#endif

/*
|| Since MinGW doesn't have an fnmatch() function, I'm obliged to
|| include a little globber.  At least it will be consistent across
|| platforms.
||
|| This particular version is in the public domain and was found at:
||     http://www.cs.yorku.ca/~oz
||
|| robust glob pattern matcher
|| ozan s. yigit/dec 1994
|| public domain
||
|| glob patterns:
||      *       matches zero or more characters
||      ?       matches any single character
||      [set]   matches any character in the set
||      [^set]  matches any character NOT in the set
||              where a set is a group of characters or ranges. a range
||              is written as two characters seperated with a hyphen:
||              a-z denotes all characters between a to z inclusive.
||      [-set]  set matches a literal hypen and any character in the set
||      []set]  matches a literal close bracket and any character in the
||              set
||
||      char    matches itself except where char is '*' or '?' or '['
||      \char   matches char, including any pattern character
||
|| examples:
||      a*c             ac abc abbc ...
||      a?c             acc abc aXc ...
||      a[a-z]c         aac abc acc ...
||      a[-a-z]c        a-c aac abc ...
||
|| $Log: glob.c,v $
|| Revision 1.3  1995/09/14  23:24:23  oz
|| removed boring test/main code.
||
|| Revision 1.2  94/12/11  10:38:15  oz
|| cset code fixed. it is now robust and interprets all
|| variations of cset [i think] correctly, including [z-a] etc.
||
|| Revision 1.1  94/12/08  12:45:23  oz
|| Initial revision
*/
#ifndef NEGATE
#define NEGATE  '^'                     /* std cset negation char */
#endif

static int
amatch( char *str, char *p )
{
    int negate;
    int match;
    int c;

    while( *p )
    {
        if( !*str && *p != '*' )
        {
            return FALSE;
        }

        switch( c = *p++ )
        {
            case '*':
                while( *p == '*' )
                {
                    p++;
                }

                if( !*p )
                {
                    return TRUE;
                }

                if( *p != '?' && *p != '[' && *p != '\\' )
                {
                    while( *str && *p != *str )
                    {
                        str++;
                    }
                }

                while( *str )
                {
                    if( amatch( str, p ) )
                    {
                        return TRUE;
                    }
                    str++;
                }

                return FALSE;

            case '?':
                if( *str )
                {
                    break;
                }

                return FALSE;

            /*
            || set specification is inclusive, that is [a-z] is a, z
            || and everything in between. this means [z-a] may be
            || interpreted as a set that contains z, a and nothing in
            || between.
            */
            case '[':
                if( *p != NEGATE )
                {
                    negate = FALSE;
                }
                else
                {
                    negate = TRUE;
                    p++;
                }

                match = FALSE;

                while( !match && ( c = *p++ ) )
                {
                    if( !*p )
                    {
                        return FALSE;
                    }

                    if( *p == '-' )         /* c-c */
                    {
                        if( !*++p )
                        {
                            return FALSE;
                        }

                        if( *p != ']' )
                        {
                            if( *str == c || *str == *p ||
                               (*str > c && *str < *p ) )
                            {
                                match = TRUE;
                            }
                        }
                        else                /* c-] */
                        {
                            if( *str >= c )
                            {
                                match = TRUE;
                            }
                            break;
                        }
                    }
                    else                    /* cc or c] */
                    {
                        if( c == *str )
                        {
                            match = TRUE;
                        }

                        if( *p != ']' )
                        {
                            if( *p == *str )
                            {
                                match = TRUE;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                if( negate == match )
                {
                    return FALSE;
                }

                /*
                || if there is a match, skip past the cset and
                || continue on
                */
                while( *p && *p != ']' )
                {
                    p++;
                }

                if( !*p++ )      /* oops! */
                {
                    return FALSE;
                }
                break;

            case '\\':
                if( *p )
                {
                    c = *p++;
                }

                /* intentional fallthrough */

            default:
                if( c != *str )
                {
                    return FALSE;
                }
                break;
        }
        str++;
    }

    return !*str;
}

static void
make_name( SUBFILE *sf )
{
    char *mode;
    int dot;

    mode = (char *) sf->fm;
    dot = '.';
    if( s_mode )
    {
        mode = s_mode;
        if( strcmp( s_mode, "0" ) == 0 )
        {
            dot = '\0';
        }
    }

    sprintf( s_name,
            "%s.%s%c%s",
            sf->fn,
            sf->ft,
            dot,
            mode );

    if( f_case )
    {
        mode = s_name;
        while( *mode )
        {
            *mode = tolower( *mode );
            mode++;
        }
    }

    return;
}

static void
list_file( SUBFILE *sf )
{
    static int needhead = TRUE;

    /*
    || Only print the header once
    */
    if( needhead )
    {
        printf( "Fn       " );
        printf( "Ft       " );
        printf( "Fm " );
        if( f_verbose )
        {
            printf( "V.R " );
            printf( "Meth " );
        }
        printf( "Date       " );
        printf( "Time     " );
        printf( "R " );
        printf( "Lrecl " );
        printf( " Compressed " );
        printf( "Uncompressed\n" );

        needhead = FALSE;
    }

    /*
    || Print the first few common fields
    */
    printf( "%-8.8s ",
           sf->fn );

    printf( "%-8.8s ",
           sf->ft );

    printf( "%-2.2s ",
           sf->fm );

    /*
    || Print some extra info if being verbose
    */
    if( f_verbose )
    {
        printf( "%1d.%1d ",
               sf->ver,
               sf->rel );

        printf( "%-4.4s ",
               sf->meth );
    }

    /*
    || Print the remaining fields
    */
    printf( "%04d/%02d/%02d %02d:%02d:%02d ",
           sf->year,
           sf->month,
           sf->day,
           sf->hour,
           sf->minute,
           sf->second );

    printf( "%c ",
           sf->recfm );

    printf( "%5d ",
           sf->lrecl );

    printf( "%11d %12d\n",
           (int) sf->compressed,
           (int) sf->uncompressed );

    return;
}

static void
usage( void )
{
    printf( "vma - Manage VMARC archives\n\n" );
    printf( "Usage: vma -x [options] archive [fn [ft [fm ]]]\n\n" );
    printf( "       vma -a [options] archive file[,fn[.ft.[fm]]] ...\n\n" );
    printf( "Options:\n" );
    printf( "  -a        add files to archive\n" );
    printf( "  -c        convert names to lowercase\n" );
    printf( "  -h        display usage summary\n" );
    printf( "  -l        record length...fixed=length, variable=max\n" );
    printf( "  -m fm     replace filemode...0=remove\n" );
    printf( "  -q        do not list files\n" );
    printf( "  -r        record format\n" );
    printf( "  -s        store method...asis, lzw, s2\n" );
    printf( "  -t        translate files to ASCII on extration\n" );
    printf( "            or to EBCDIC on addition\n" );
    printf( "  -u f,t    (f)rom and (t) UCM filenames\n" );
    printf( "  -v        verbose listing\n" );
    printf( "  -x        extract files\n" );
    printf( "  -V        display version\n\n" );
    printf( "input:\n"
           "  name of the VMARC archive\n\n" );
    printf( "fn, ft, fm:\n"
           "  filter on file name, type, and/or mode during extraction\n" );
    printf( "  (case is significant)\n\n" );
    printf( "file[,fn[.ft[.fm]]]:\n"
           "  one or more file names to be added to the VMARC archive\n" );
    printf( "  specify ',' and fn.ft.fm to store file with a different name\n\n" );
    printf( "f,t:\n" );
    printf( "  paths to translateion tables (see README)\n" );

    exit( 1 );
}

int
main( int argc, char *argv[] )
{
    /*          Fn  .   Ft  .   Fm  0 */
    char fname[ 8 + 1 + 8 + 1 + 2 + 1 ];
    void *vma = NULL;
    int rc = VMAE_NOERR;
    int cnt;
    int sfcount = 0;
    int sfproc = 0;
    char *fn = NULL;
    char *ft = NULL;
    char *fm = NULL;
    char *tname = NULL;

    /*
    || Process command flags
    */
    while( ( rc = getopt( argc, argv, "achl:m:qr:s:tu:vxV" ) ) != -1 )
    {
        switch( rc )
        {
            case 'a':
                if( f_extract )
                {
                    printf( "-a and -x are mutually exclusive\n" );
                    usage();
                }
                f_add = TRUE;
                break;

            case 'c':
                f_case = TRUE;
                break;

            case 'l':
            {
                char *endp;

                lrecl = strtol( optarg, &endp, 10 );
                if( *endp || lrecl < 1 || lrecl > 65535 )
                {
                    printf( "invalid record length %s\n", optarg );
                    usage();
                }
            }
                break;

            case 'm':
                s_mode = optarg;
                break;

            case 'q':
                f_list = FALSE;
                break;

            case 'r':
                if( strcmp( optarg, "f" ) == 0 || strcmp( optarg, "fixed" ) == 0 )
                {
                    recfm = VMAR_FIXED;
                }
                else if( strcmp( optarg, "v" ) == 0 || strcmp( optarg, "variable" ) == 0 )
                {
                    recfm = VMAR_VARIABLE;
                }
                else
                {
                    printf( "invalid record format %s\n", optarg );
                    usage();
                }
                break;

            case 's':
                if( strcmp( optarg, "a" ) == 0 || strcmp( optarg, "asis" ) == 0 )
                {
                    s_meth = VMAM_ASIS;
                }
                else if( strcmp( optarg, "l" ) == 0 || strcmp( optarg, "lzw" ) == 0 )
                {
                    s_meth = VMAM_LZW;
                }
                else if( strcmp( optarg, "s" ) == 0 || strcmp( optarg, "s2" ) == 0 )
                {
                    s_meth = VMAM_S2;
                }
                else
                {
                    printf( "invalid store method %s\n", optarg );
                    usage();
                }
                break;

            case 't':
                xmode = VMAX_TEXT;
                break;

            case 'u':
                s_fucm = optarg;
                s_tucm = strchr( optarg, ',' );
                if( !s_tucm )
                {
                    usage();
                }
                (*s_tucm++) = '\0';
                break;

            case 'V':
                f_version = TRUE;
                break;

            case 'v':
                f_verbose = TRUE;
                break;

            case 'x':
                if( f_add )
                {
                    printf( "-a and -x are mutually exclusive\n" );
                    usage();
                }
                f_extract = TRUE;
                break;

            case 'h':
            default:
                usage();
                break;
        }
    }

    if( f_version )
    {
        printf( "%s\nVersion " LIBVER
               " - compiled on " __DATE__
               " at " __TIME__ "\n",
               argv[ 0 ] );

        exit( 0 );
    }

    /*
    || Make sure we have the right number of arguments
    */
    cnt = argc - optind;
    if( f_extract )
    {
        if( cnt < 1 || cnt > 4 )
        {
            usage();
        }
    }
    else if( f_add )
    {
        if( cnt < 1 )
        {
            usage();
        }
    }

    /*
    || Just in case the user forgot what file was being worked on
    */
    if( f_verbose )
    {
        printf( "Processing: %s\n\n", argv[ optind ] );
    }

    /*
    || Open the VMARC file
    */
    rc = vma_open( argv[ optind ], &vma );
    if( rc != VMAE_NOERR )
    {
        printf( "Aborting due to error: %s\n",
               vma_strerror( rc ) );
        exit( 1 );
    }

    if( s_fucm )
    {
        rc = vma_setconv( vma, s_fucm, s_tucm );
        if( rc != VMAE_NOERR )
        {
            printf( "Failed to set conversion: %d\n", rc );
            goto error;
        }
    }

    rc = vma_setmode( vma, xmode );
    if( rc != VMAE_NOERR )
    {
        printf( "Unable to set conversion mode\n" );
        goto error;
    }

    /*
    || Adding files or extracting/listing?
    */
    if( f_add )
    {
        SUBFILE *sf;
        int i;

        for( i = optind + 1 ; i < argc; i++ )
        {
            char *tok = argv[ i ];
            char *tmp;
            struct stat st;
            struct tm *tm;
            int cnt = 0;

#if defined(_WIN32)
            if( *tok && *( tok + 1 ) == ':' )
            {
                tok += 2;
            }
#endif

            tmp = tok;
            while( *tmp && *tmp != ',' )
            {
#if defined(__WIN32)
                if( *tmp == '\\' )
#else
                if( *tmp == '/' )
#endif
                {
                    tok = tmp + 1;
                }

                tmp++;
            }

            if( *tmp == ',' )
            {
                *tmp = '\0';
                tok = tmp + 1;
            }

            tname = strdup( tok );
            if( tname == NULL )
            {
                printf( "no mem\n" );
                goto error;
            }

            tok = strtok( tname, "." );
            while( tok )
            {
                cnt++;

                if( cnt == 1 )
                {
                    fn = strdup( tok );
                    tok = fn;
                    while( tok && *tok )
                    {
                        *tok = toupper( *tok );
                        tok++;
                    }
                }
                else if( cnt == 2 )
                {
                    ft = strdup( tok );
                    tok = ft;
                    while( tok && *tok )
                    {
                        *tok = toupper( *tok );
                        tok++;
                    }
                }
                else if( cnt == 3 )
                {
                    fm = strdup( tok );
                    tok = fm;
                    while( tok && *tok )
                    {
                        *tok = toupper( *tok );
                        tok++;
                    }
                }
                else
                {
                    printf( "filename '%s' has too many tokens\n", argv[ i ] );
                    goto error;
                }

                tok = strtok( NULL, "." );
            }
            free( tname );
            tname = NULL;

            if( stat( argv[ i ], &st ) != 0 )
            {
                printf( "couldn't stat '%s'\n", argv[ i ] );
                goto error;
            }

            tm = localtime( &st.st_mtime );
            if( tm == NULL )
            {
                printf( "couldn't convert time\n" );
                goto error;
            }

            rc = vma_new( vma, &sf );
            if( rc != VMAE_NOERR )
            {
                goto error;
            }

            rc = vma_setname( vma, fn, ft, fm );
            if( rc != VMAE_NOERR )
            {
                goto error;
            }

            if( fn )
            {
                free( fn );
                fn = NULL;
            }

            if( ft )
            {
                free( ft );
                ft = NULL;
            }

            if( fm )
            {
                free( fm );
                fm = NULL;
            }

            rc = vma_setdate( vma, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday );
            if( rc != VMAE_NOERR )
            {
                goto error;
            }

            rc = vma_settime( vma, tm->tm_hour, tm->tm_min, tm->tm_sec );
            if( rc != VMAE_NOERR )
            {
                goto error;
            }

            rc = vma_setrecfm( vma, recfm );
            if( rc != VMAE_NOERR )
            {
                printf( "Unable to set record format\n" );
                goto error;
            }

            rc = vma_setlrecl( vma, lrecl );
            if( rc != VMAE_NOERR )
            {
                printf( "Unable to set record length\n" );
                goto error;
            }

            rc = vma_setmethod( vma, s_meth );
            if( rc != VMAE_NOERR )
            {
                printf( "Unable to set store method\n" );
                goto error;
            }

            rc = vma_add( vma, argv[ i ] );
            if( rc != VMAE_NOERR )
            {
                goto error;
            }

            if( f_list )
            {
                list_file( sf );
            }
        }

        rc = vma_commit( vma );
        if( rc != VMAE_NOERR )
        {
            goto error;
        }
    }
    else
    {
        SUBFILE *sf;

        if( cnt > 1 )
        {
            /*
            || First optional argument becomes the file name filter
            */
            s_fn = argv[ optind + 1 ];

            /*
            || Second optional argument becomes the file type filter
            */
            if( cnt > 2 )
            {
                s_ft = argv[ optind + 2 ];

                /*
                || Third optional argument becomes the file mode filter
                */
                if( cnt > 3 )
                {
                    s_fm = argv[ optind + 3 ];
                }
            }
        }
        /*
        || Calculate filter length
        */
        i_flen = strlen( s_fn ) + 1 +
        strlen( s_ft ) + 1 +
        strlen( s_fm ) + 1;

        /*
        || Allocate it
        */
        s_filter = malloc( i_flen );
        if( s_filter == NULL )
        {
            goto error;
        }

        /*
        || Build the filter string
        */
        sprintf( s_filter,
                "%s.%s.%s",
                s_fn,
                s_ft,
                s_fm );

        /*
        || Display non-default filter
        */
        if( strcmp( s_filter, "*.*.*" ) != 0 )
        {
            printf("Using filter: '%s'\n\n", s_filter );
        }

        /*
        || Calculate output name length
        */
        i_nlen = 8 + 1 +
        8 + 1 +
        ( s_mode ? strlen( s_mode ) : 8 ) + 1;

        /*
        || Allocate it
        */
        s_name = malloc( i_nlen );
        if( s_name == NULL )
        {
            printf( "no mem\n" );
            goto error;
        }

        for( rc = vma_first( vma, &sf );
            rc == VMAE_NOERR;
            rc = vma_next( vma, &sf ) )
        {
            /*
            || Track total subfile count
            */
            sfcount++;

            /*
            || Build a name for filtering
            */
            sprintf( fname,
                    "%s.%s.%s",
                    sf->fn,
                    sf->ft,
                    sf->fm );

            /*
            || Filter it
            */
            if( !amatch( fname, s_filter ) )
            {
                continue;
            }

            /*
            || Extract file
            */
            if( f_extract )
            {
                /*
                || Build the output name
                */
                make_name( sf );

                /*
                || And extract
                */
                rc = vma_extract( vma, s_name );
                if( rc != VMAE_NOERR )
                {
                    if( rc != VMAE_LRECL )
                    {
                        goto error;
                    }

                    printf( "Bypassing next file due "
                           "to LRECL limitations:\n" );
                }
            }

            /*
            || List it ... do after extraction to get byte counts
            */
            if( f_list )
            {
                list_file( sf );
            }

            /*
            || Track processed subfile count
            */
            sfproc++;
        }

        /*
        || This one really isn't an error
        */
        if( rc == VMAE_NOMORE )
        {
            rc = VMAE_NOERR;
        }

        /*
        || Print a little summary
        */
        if( f_verbose )
        {
            printf( "\n%d subfiles",
                   sfcount );

            if( sfcount != sfproc )
            {
                printf( ", %d bypassed due to filtering",
                       sfcount - sfproc );
            }

            printf( "\n" );
        }

    }
error:

    if( fn )
    {
        free( fn );
        fn = NULL;
    }

    if( ft )
    {
        free( ft );
        ft = NULL;
    }

    if( fm )
    {
        free( fm );
        fm = NULL;
    }

    if( tname )
    {
        free( tname );
        tname = NULL;
    }

    if( rc != VMAE_NOERR )
    {
        printf( "Aborting due to error: %s\n",
               vma_strerror( rc ) );
    }

    if( vma )
    {
        vma_close( vma );
    }

    return rc;
}
