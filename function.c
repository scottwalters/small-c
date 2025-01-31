/*	File function.c: 2.1 (83/03/20,16:02:04) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

void getarg (int t);

/*
 *	begin a function
 *
 *	called from "parse", this routine tries to make a function out
 *	of what follows
 *	modified version.  p.l. woods
 *
 */
int argtop;
void
newfunc ()
{
	char	n[NAMESIZE], *ptr;
	fexitlab = getlabel();

	fprintf(stderr, "debug: newfunc %s\n", n);

	if (!symname (n) ) {
		error ("illegal function or declaration");
		kill ();
		return;
	}
	if (ptr = findglb (n)) {
                // if(! ptr) fprintf(stderr, "debug: findglb failed"); else fprintf(stderr, "debug: findglb=%ud\n", ptr);
		if (ptr[IDENT] != FUNCTION)
			multidef (n);
		else if (ptr[OFFSET] == FUNCTION)
			multidef (n);
		else
			ptr[OFFSET] = FUNCTION;
	} else
		addglb (n, FUNCTION, CINT, FUNCTION, PUBLIC);
	prologue ();
	if (!match ("("))
		error ("missing open paren");
	prefix ();
	outstr (n);
	col ();
	nl ();
	locptr = STARTLOC;
	argstk = 0;
	while (!match (")")) {
		if (symname (n)) {
			if (findloc (n)) {
                                fprintf(stderr, "debug: newfunc: multidef %s\n", n);
				multidef (n);
			} else {
				addloc (n, 0, 0, argstk, AUTO);
				argstk = argstk + intsize();
                                fprintf(stderr, "debug: newfunc: added %s, new argstk=%lx\n", n, argstk);
			}
		} else {
			error ("illegal argument name");
			junk ();
		}
		blanks ();
		if (!streq (line + lptr, ")")) {
			if (!match (","))
				error ("expected comma");
		}
		if (endst ())
			break;
	}
	stkp = 0;
	argtop = argstk;
	while (argstk) {
		if (amatch ("register", 8)) {
			if (amatch("char", 4)) 
				getarg((int)CCHAR);
			else if (amatch ("int", 3))
				getarg((int)CINT);
			else
				getarg((int)CINT);
			ns();
		} else if (amatch ("char", 4)) {
			getarg (CCHAR);
			ns ();
		} else if (amatch ("int", 3)) {
			getarg (CINT);
			ns ();
		} else {
			error ("wrong number args");
			break;
		}
	}
	statement(YES);
	printlabel(fexitlab);
	col();
	nl();
	modstk (0);
	gret ();
	stkp = 0;
	locptr = STARTLOC;
}

/*
 *	declare argument types
 *
 *	called from "newfunc", this routine add an entry in the local
 *	symbol table for each named argument
 *	completely rewritten version.  p.l. woods
 *
 */
void
getarg (int t)
{
	int	j, legalname, address;
	char	n[NAMESIZE], c, *argptr;

	FOREVER {
		if (argstk == 0)
			return;
		if (match ("*"))
			j = POINTER;
		else
			j = VARIABLE;
		if (!(legalname = symname (n)))
			illname ();
		if (match ("[")) {
			while (inbyte () != ']')
				if (endst ())
					break;
			j = POINTER;
		}
		if (legalname) {
			if (argptr = findloc (n)) {
				argptr[IDENT] = j;
				argptr[TYPE] = t;
				address = argtop - glint(argptr);
				if (t == CCHAR && j == VARIABLE)
					address = address + byteoff();
				argptr[OFFSET] = (address) & 0xff;
				argptr[OFFSET + 1] = (address >> 8) & 0xff;
			} else
				error ("expecting argument name");
		}
		argstk = argstk - intsize();
		if (endst ())
			return;
		if (!match (","))
			error ("expected comma");
	}
}
