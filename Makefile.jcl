//*
//* -------------------------------------------------------------------
//* To rebuild under z/OS:
//*
//* 1)  Create the following datasets:
//*     <pfx>.VMA.C,    RECFM=FB, LRECL=255, BLKSIZE=0
//*     <pfx>.VMA.H,    RECFM=FB, LRECL=255, BLKSIZE=0
//*     <pfx>.VMA.LOAD, RECFM=U,  LRECL=0,   BLKSIZE=32760
//* 2)  Upload the *.c files into the <pfx>.VMA.C PDS
//* 3)  Upload the *.h files into the <pfx>.VMA.H PDS
//* 4)  Upload the Makefile.jcl file wherever you like
//* 5)  Change the "SET PFX=" line to the correct HLQ
//* 6)  Run the Makefile.jcl job
//*
//* -------------------------------------------------------------------
//  SET PFX=<pfx>                                     *** CHANGE ME ***
//*
//  SET CFLAGS='LANGLVL(EXTENDED),OPT(2),NOMAR,LONG,ARCH(3),NOSEQ'
//*
//         JCLLIB ORDER=CBC.SCCNPRC
//*
//VMA      EXEC CBCCL,
//         PARM.COMPILE='&CFLAGS',
//         OUTFILE='*.COMPILE.SYSLMOD,DISP=SHR'
//*
//USERLIB  DD DISP=SHR,DSN=&PFX..VMA.H
//SYSIN    DD DISP=SHR,DSN=&PFX..VMA.C(VMA)
//         DD DISP=SHR,DSN=&PFX..VMA.C(VMALIB)
//SYSLMOD  DD DISP=SHR,DSN=&PFX..VMA.LOAD(VMA)
//*
