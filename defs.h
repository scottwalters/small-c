/*	File defs.h: 2.1 (83/03/21,02:07:20) */

extern void error(char *);
char * findglb (char *);
char * findloc (char *);
int * readwhile();
int * findwhile();
int * readswitch();





#define	FOREVER	for(;;)
#define	FALSE	0
#define	TRUE	1
#define	NO	0
#define	YES	1

/* miscellaneous */

#define	EOS	0
#define	EOL	10
#define	BKSP	8
#define	CR	13
#define	FFEED	12
#define TAB	9

/* symbol table parameters */

#define	SYMSIZ	16
#define	SYMTBSZ	3200
#define	NUMGLBS	150
#define	STARTGLB	symtab
#define	ENDGLB	(STARTGLB+NUMGLBS*SYMSIZ)
#define	STARTLOC	(ENDGLB+SYMSIZ)
#define	ENDLOC	(symtab+SYMTBSZ-SYMSIZ)

/* symbol table entry format */

#define	NAME	0
#define	IDENT	11
#define	TYPE	12
#define	STORAGE	13
// OFFSET is low byte, OFFSET+1 at [15] is highbyte, so [15] is used
#define	OFFSET	14

/* system-wide name size (for symbols) */

#define	NAMESIZE	9
#define	NAMEMAX	8

/* possible entries for "ident" */

#define	VARIABLE	1
#define	ARRAY	2
#define	POINTER	3
#define	FUNCTION	4

/* possible entries for "type" */

#define	CCHAR	1
#define	CINT	2

/* possible entries for storage */

#define	PUBLIC	1
#define	AUTO	2
#define	EXTERN	3

#define	STATIC	4
#define	LSTATIC	5
#define	DEFAUTO	6

/* "do"/"for"/"while"/"switch" statement stack */

// changed from 100 to 128 to be a multiple of 8
#define	WSTABSZ	128
// changed from 7 to 8 to try to make int pointers align, but this should be int array size anyway, not char, so not sure
#define	WSSIZ	8
#define	WSMAX	ws+WSTABSZ-WSSIZ

/* entry offsets in "do"/"for"/"while"/"switch" stack */

#define	WSSYM	0
#define	WSSP	1
#define	WSTYP	2
#define	WSCASEP	3
#define	WSTEST	3
#define	WSINCR	4
#define	WSDEF	4
#define	WSBODY	5
#define	WSTAB	5
#define	WSEXIT	6

/* possible entries for "wstyp" */

#define	WSWHILE	0
#define	WSFOR	1
#define	WSDO	2
#define	WSSWITCH	3

/* "switch" label stack */

#define	SWSTSZ	100

/* literal pool */

#define	LITABSZ	2000
#define	LITMAX	LITABSZ-1

/* input line */

#define	LINESIZE	150
#define	LINEMAX	(LINESIZE-1)
#define	MPMAX	LINEMAX

/* macro (define) pool */

#define	MACQSIZE	1000
#define	MACMAX	(MACQSIZE-1)

/* "include" stack */

#define	INCLSIZ	3

/* statement types (tokens) */

#define	STIF	1
#define	STWHILE	2
#define	STRETURN	3
#define	STBREAK	4
#define	STCONT	5
#define	STASM	6
#define	STEXP	7
#define	STDO	8
#define	STFOR	9
#define	STSWITCH	10

#define	DEFLIB	inclib()
