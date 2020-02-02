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

#if !defined( _VMALIB_H )
#define _VMALIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------
|| Subfile information
*/
typedef struct subfile
{
    char   ver;                         /* version of creating VMARC */
    char   rel;                        /* revision of creating VMARC */
    char   meth[ 5 ];               /* storage method: ASIS, LZW, S2 */
    char   fn[ 9 ];                               /* null terminated */
    char   ft[ 9 ];                               /* null terminated */
    char   fm[ 3 ];                               /* null terminated */
    int    lrecl;                                  /* possibly > 64k */
    int    year;
    char   month;
    char   day;
    char   hour;
    char   minute;
    char   second;
    char   recfm;
    size_t compressed;
    size_t uncompressed;
    char   dtype;                           /* data type TRUE = text */
} SUBFILE;

/* --------------------------------------------------------------------
|| SUBFILE::meth
*/
#define VMAM_ASIS       "ASIS"
#define VMAM_LZW        "LZW"
#define VMAM_S2         "S2"

/* --------------------------------------------------------------------
|| SUBFILE::recfm
*/
#define VMAR_FIXED      'F'
#define VMAR_VARIABLE   'V'

/* --------------------------------------------------------------------
|| SUBFILE::dtype is only a best guess and is determined by examining
|| the first 1024 bytes of each subfile and, if any bytes fall below
|| 0x20, the mode value will be set to BINARY.
*/
#define VMAD_UNKNOWN    0
#define VMAD_TEXT       1
#define VMAD_BINARY     2

/* --------------------------------------------------------------------
|| Extraction and Addition mode
*/
#define VMAX_AUTO       0                   /* use SUBFILE:dtype     */
#define VMAX_TEXT       1                   /* convert to text+LF    */
#define VMAX_BINARY     2                   /* no conversion         */
#define VMAX_TRANS      3                   /* translate             */

/* --------------------------------------------------------------------
|| Errors
*/
enum
{
    VMAE_NOERR,                             /* no error              */
    VMAE_RERR,                              /* read error            */
    VMAE_WERR,                              /* write error           */
    VMAE_OOVER,                             /* output overflow       */
    VMAE_INACT,                             /* no active subfile     */
    VMAE_IOPEN,                             /* open input failed     */
    VMAE_OOPEN,                             /* open output failed    */
    VMAE_TOPEN,                             /* open temp failed      */
    VMAE_MEM,                               /* insufficient memory   */
    VMAE_SEEK,                              /* repositioning failed  */
    VMAE_BADARG,                            /* missing parameter     */
    VMAE_NOMORE,                            /* mo more subfiles      */
    VMAE_NOTFOUND,                          /* subfile not found     */
    VMAE_NEEDMORE,                          /* expected more data    */
    VMAE_UCMOPEN,                           /* UCM open failed       */
    VMAE_UCMSBCS,                           /* only SBCS UCMs okay   */
    VMAE_SUBCHAR,                           /* invalid UCM subchar   */
    VMAE_CHARMAP,                           /* missing or bad cmap   */
    VMAE_LRECL,                             /* LRECL exceeds max     */
    VMAE_ASCII,                             /* VMARC appears ASCII   */
    VMAE_BADDATA,                           /* invalid input data    */
    VMAE_BADNAME,                           /* file name invalid     */
    VMAE_BADTYPE,                           /* file type invalid     */
    VMAE_BADMODE,                           /* file mode invalid     */
    VMAE_BADYEAR,                           /* year invalid          */
    VMAE_BADMONTH,                          /* month invalid         */
    VMAE_BADDAY,                            /* day invalid           */
    VMAE_BADHOUR,                           /* hour invalid          */
    VMAE_BADMINS,                           /* minutes invalid       */
    VMAE_BADSECS,                           /* seconds invalid       */
    VMAE_NOTMOD,                            /* field not modifiable  */
    VMAE_BADRECFM,                          /* record format invalid */
    VMAE_BADLRECL,                          /* record length invalid */
    VMAE_BADFILE,                           /* filename is null      */
    VMAE_RETAINED,                          /* already retained      */
    VMAE_NOTRET,                            /* not retained          */
    VMAE_RENAME,                            /* rename failed         */
    VMAE_NUMERRORS                          /* number of errors      */
};

/* --------------------------------------------------------------------
|| Public functions
*/
extern int vma_open( const char *name, void **vvma );
extern void vma_close( void *vvma );

extern int vma_setmode( void *vvma, int mode );

extern int vma_first( void *vvma, SUBFILE **sfp );
extern int vma_next( void *vvma, SUBFILE **sfp );

extern int vma_getactive( void *vvma, SUBFILE **sfp );
extern int vma_setactive( void *vvma, SUBFILE *sf );

extern int vma_extract( void *vvma, const char *name );
extern int vma_setconv( void *vvma, const char *fucm, const char *tucm );

extern const char *vma_strerror( int ec );

extern int vma_new( void *vvma, SUBFILE **sfp );
extern int vma_setname( void *vvma, const char *fn, const char *ft, const char *fm );
extern int vma_setdate( void *vvma, int year, int month, int day );
extern int vma_settime( void *vvma, int hour, int minute, int second );
extern int vma_setrecfm( void *vvma, char recfm );
extern int vma_setlrecl( void *vvma, int lrecl );
extern int vma_setmethod( void *vvma, const char *method );

extern int vma_add( void *vvma, const char *name );
extern int vma_delete( void *vvma );

extern int vma_isdirty( void *vvma, int *dirty );
extern int vma_commit( void *vvma );

extern int vma_retain( void *vvma );
extern int vma_release( void *vvma, int discard );

#ifdef __cplusplus
}
#endif

#endif
