

VMA/VMAgui - (Un)Archiver for VMARC files


WHAT
----------
This small utility allows you to list, extract, and/or create VMARC
archive files.  These files are used on the VM operating system, so if
you don't know what that is, you probably don't need VMA.

I used the latest VMARC source to create a portable version in C.  You
can find the latest assembler source at:

    http://www.geocities.com/RossPatterson/vmarc/

It is currently maintained by Ross Patterson, and VMARC was originally
written by John Fisher.

There is a command line utility called "vma" that should compile and run
on just about any platform, as it is not a very complicated build.  It is
currently known to run on Linux, OSX, and Windows.  To get usage info,
just run the command for help.

VMAgui uses wxWidgets to achieve cross-platform usability.  It is a
fairly basic interface, so it should be easy to figure out.  However,
there are 2 areas that warrant a bit of explanation.

The "Pattern" edit box allows control of the output name for extracted
files.  It accepts the following special patterns.  Anything else is
passed through unchanged.

    %f = CMS file name (lowercase)
    %F = CMS file name (uppercase)
    %t = CMS file type (lowercase)
    %T = CMS file type (uppercase)
    %m = CMS file mode (lowercase)
    %M = CMS file mode (uppercase)

In the Settings dialog, you can specify the file names of Unicode
Character Map files to control how the data within the subfiles is
converted during extraction.  You use the From name to specify the
original encoding of the files, prior to archiving.  The To name
specifies the desired encoding, after extraction.

These UCMs can be obtained from the ICU CVS repository at:

    http://dev.icu-project.org/cgi-bin/viewcvs.cgi/charset/data/ucm/


HOW
----------
The build process is fairly straightforward.  If you wish to build
VMAgui, then you'll need to install wxWidgets.  The only tested version
is 2.8.10, but VMAgui might build with others.  Besides that, just a
standard GNU build environment is needed.


WHY
----------
'Cause I'm lazy.  :-) It was way too much work to upload a VMARC to VM,
just to take a look at it.


WHEN
----------
One weekend in the summer of '05, when I should've been working on other
projects.  This soon blossomed into a few weeks as a Windows version was
written using MFC and then rewritten using wxWidgets.


WHO
---------
Leland Lucius (that's me) - this utility

Ross Patterson - current VMARC maintainer

John Fisher - original creator of VMARC

ozan s. yigit - glob pattern matcher


CONTRIBS
----------
I couldn't keep track, so if you made contributions, thank you.


CONTACT
----------
Despite what I say in the next 2 sections, feel free to contact me, if
you find a bug.  I'll do my best to stomp on it.  But, my depth perception
is getting worse in my old age, so it my take me a few trys to get it.
:-)

Email: vma@homerow.net


COPYRIGHT
----------
I reckon I own the copyrights to this utility, since it is sort of an
original work.  But, as it's based on John Fisher's work I wouldn't feel
right about making any claims to ownership.

So, I therefore release VMA into the public domain.  Do with it as you
like.  Mutilate it, molest it, spindle it, fondle it, or whatever else.

It wouldn't be very nice if you made money off of it, but if that's what
you want to do...knock yourself out.


DISCLAIMER
----------
If VMA breaks, or breaks anything around it, too bad.  You have made the
decision to use it and are therefore responsible for whatever happens.

I built the gun, but I didn't pull the trigger.  :-D

