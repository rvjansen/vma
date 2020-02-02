//*
//* -------------------------------------------------------------------
//* To run VMA under z/OS:
//*
//* 1)  Create the following datasets:
//*       <pfx>.VMA.XMIT, RECFM=FB, LRECL=80, BLKSIZE=0
//* 2)  Upload the VMA.XMIT file to <pfx>.VMA.XMIT in BINARY mode
//* 3)  Extract the load library:
//*       RECEIVE INDS('<pfx>.VMA.XMIT')
//*     when prompted for restore parameters, enter:
/*        DATASET('<pfx>.VMA.LOAD')
//* 4)  Upload the Run.jcl file to wherever you like
//* 5)  Change the "SET PFX=" line to the value used for <pfx> above
//* 6)  Change the PARM= value and run the job
//*
//* -------------------------------------------------------------------
//  SET PFX=<pfx>                                     *** CHANGE ME ***
//*
//VMA      EXEC PGM=VMA,
//         PARM='/-v -x YOUR.VMARC * * *'
//*
//STEPLIB  DD DISP=SHR,DSN=<pfx>.VMA.LOAD
//SYSPRINT DD SYSOUT=*
//*
