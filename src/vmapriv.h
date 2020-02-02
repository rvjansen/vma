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

#if !defined( _VMAPRIV_H )
#define _VMAPRIV_H

/* --------------------------------------------------------------------
|| Some EOU types
*/
typedef unsigned char uchar;

/* --------------------------------------------------------------------
|| General I/O stuff
*/
#define BUFLEN      65536               /* input buffer size         */
#define UNREAD      128                 /* max # of unread() bytes   */

/* --------------------------------------------------------------------
|| Shared compression stuff
*/

/*
||                                                              V1R2P019
||  Internally the EBCDIC codes 00 - FF are remapped to be 001 - 100.
||  Code 000 is used as the record/file endmarker.
||
*/
#define kodendr 0               /* End-of-record compaction code     */
#define kodebcd 1               /* Origin of EBCDIC character codes  */
#define kodmax  kodebcd + 255   /* Last predefined compaction code   */

/* --------------------------------------------------------------------
|| LZW stuff
*/
#define CHAINEND ( (struct lzwdsect *) -1 )

/* --------------------------------------------------------------------
|| String table
*/
#define TABSIZE 4096

typedef struct lzwdsect
{
    struct lzwdsect *right;
    struct lzwdsect *left;
    struct lzwdsect *pred;
    unsigned short  refcnt;
    unsigned short  schar;
} LZWSTRING;

#define ENDCHAIN ( (LZWSTRING *) -1 )

/* --------------------------------------------------------------------
|| Hash table
||
|| Each entry in the hash table contains pointers to the first and
|| last entries in the chain of doubly-linked string table entries.
*/
#define HASHSIZE 5003

typedef struct lzwhash
{
    LZWSTRING *head;
    LZWSTRING *tail;
} LZWHASH;

/* --------------------------------------------------------------------
|| S2 stuff
*/
typedef struct strdsect
{
    struct strdsect *strleft;
    struct strdsect *strright;
    struct strdsect *strsiblg;
    struct strdsect *stroffsp;
    unsigned short strlen;
    unsigned short strcount;
    unsigned short strchar;
    uchar visited;
    uchar align[ 9 ];
} STRDSECT;

/* --------------------------------------------------------------------
|| File header stuff
*/
#define H_VER       0       /* len = 1 */
#define H_REL       1       /* len = 1 */
#define H_FN        2       /* len = 8 */
#define H_FT        10      /* len = 8 */
#define H_FM        18      /* len = 2 */
#define H_LRECL     20      /* len = 2 */
#define H_YEAR      22      /* len = 1 */
#define H_MONTH     23      /* len = 1 */
#define H_DAY       24      /* len = 1 */
#define H_HOUR      25      /* len = 1 */
#define H_MINUTE    26      /* len = 1 */
#define H_SECOND    27      /* len = 1 */
#define H_RECFM     28      /* len = 1 */
#define H_FLAGS     29      /* len = 1 */
#define H_DLEN      30
#define H_XRECL     30      /* len = 4 */
#define H_RSVD      34      /* len = 8 */
#define H_XDLEN     42

#define HF_S2       0x80
#define HF_ASIS     0x40
#define HF_Y2K      0x20
#define HF_EXTH     0x01

typedef struct psubfile
{
    struct psubfile *next;              /* next private subfile      */
    size_t          dataofftmp;         /* offset to file data       */
    size_t          dataoff;            /* offset to file data       */
    unsigned char   flags;              /* flags from subfile header */
    unsigned char   dirty;              /* subfile has been changed  */
    unsigned char   temp;               /* subfile lives in tempfile */
    unsigned char   locked;             /* can't change some fields  */
    SUBFILE         sf;                 /* SUBFILE info              */
} PSUBFILE;

typedef struct vma
{
    /* ----------------------------------------------------------------
    || General
    */
    uchar a2e_map[ 256 ];               /* ascii->ebcdic table       */
    uchar e2a_map[ 256 ];               /* ebcdic->ascii table       */
    int lasterr;                        /* last error code           */
    int mode;                           /* extract/add mode          */
    char f_zos;                         /* system is z/OS            */
    char f_zvm;                         /* system is z/VM            */
    char f_debug;                       /* turn on debugging         */
    char f_text;                        /* append line ends          */
    char f_size;                        /* retrieve subfile sizes    */
    char f_extract;                     /* extract subfiles          */
    char f_scanning;                    /* scanning for data type    */
    char f_dirty;                       /* archive has been changed  */

    /* ----------------------------------------------------------------
    || General I/O stuff
    */
    char *vname;                        /* name of VMA file          */
    FILE *vfile;                        /* VMA file handle           */
    char *tname;                        /* name of temp file         */
    FILE *tfile;                        /* temp file handle          */

    FILE *in;                           /* input file handle         */
    unsigned char ibuf[ BUFLEN + UNREAD ]; /* input buffer           */
    size_t ibufp;                       /* index to next byte in buf */
    size_t ipos;                        /* file pos at last read     */
    size_t icnt;                        /* bytes left in buffer      */
    size_t bytesin;                     /* num subfile bytes read    */

    FILE *out;                          /* output file handle        */
    unsigned char *obuf;                /* output buffer             */
    size_t opos;                        /* index into output buffer  */
    size_t omax;                        /* max size of output record */
    size_t bytesout;                    /* num subfile bytes written */

    /* ----------------------------------------------------------------
    || Shared compression stuff
    */
    char recfm;                 /* RECFM of current subfile          */
    int lrecl;                  /* LRECL of current subfile          */
    size_t recbytes;            /* Bytes in current record (RECFM=F) */
    int eor;                    /* Number of times EOR seen          */
    unsigned int residual;      /* Bits waiting to be written        */

    /* ----------------------------------------------------------------
    || LZW stuff
    */
    LZWHASH lzwhashtab[ HASHSIZE ];         /* lzw hash table        */
    LZWSTRING lzwstrtab[ TABSIZE + 1 ];     /* lzw string table      */
    LZWSTRING *lzwtabs;                     /* First reusable entry  */
    LZWSTRING *lzwtabp;                     /* Last entry of table   */
    LZWSTRING *lzwtabl;                     /* End of string table   */

    /* ----------------------------------------------------------------
    || S2 stuff
    */
    unsigned short s2buf[ 2048 ];           /* s2 input buffer       */
    STRDSECT s2strtab[ TABSIZE ];           /* s2 string table       */
    STRDSECT *s2tabs;                       /* First reusable entry  */
    STRDSECT *s2tabp;                       /* Last entry of table   */
    STRDSECT *s2tabl;                       /* End of string table   */

    /* ----------------------------------------------------------------
    || Subfile stuff
    */
    PSUBFILE *subfiles;                     /* subfile list          */
    PSUBFILE *sflast;                       /* last subfile          */
    PSUBFILE *active;                       /* active subfile        */
    unsigned char head[ H_XDLEN ];          /* header buffer         */
    unsigned char dtype;                    /* apparent data type    */
    SUBFILE sfsave;                         /* retained subfile      */
    PSUBFILE *sfretained;                   /* which was retained    */
    char f_retdirty;                        /* retained dirty flag   */
} VMA;

/* --------------------------------------------------------------------
|| Handy dandy error setter
*/
#define seterr( e ) ( vma->lasterr = (e))   /* set last error        */

#endif
