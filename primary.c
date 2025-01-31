/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

primary (lval)
long int	*lval;
{
	char	*ptr, sname[NAMESIZE];
	int	num[1];
	int	k;

	lval[2] = 0;  /* clear pointer/array type */
	if (match ("(")) {
		k = heir1 (lval);
		needbrack (")");
		return (k);
	}
	if (amatch("sizeof", 6)) {
		needbrack("(");
		immed();
		if (amatch("int", 3)) onum(intsize());
		else if (amatch("char", 4)) onum(1);
		else if (symname(sname)) {
			if ((ptr = findloc(sname)) ||
				(ptr = findglb(sname))) {
				if (ptr[STORAGE] == LSTATIC)
					error("sizeof local static");
				k = glint(ptr);
				if ((ptr[TYPE] == CINT) ||
					(ptr[IDENT] == POINTER))
					k *= intsize();
				onum(k);
			} else {
				error("sizeof undeclared variable");
				onum(0);
			}
		} else {
			error("sizeof only on type or variable");
		}
		needbrack(")");
		nl();
		return(lval[0] = lval[1] = 0);
	}
	if (symname (sname)) {
		if (ptr = findloc (sname)) {
			getloc (ptr);
			lval[0] = ptr;
			lval[1] =  ptr[TYPE];
			if (ptr[IDENT] == POINTER) {
				lval[1] = CINT;
				lval[2] = ptr[TYPE];
			}
			if (ptr[IDENT] == ARRAY) {
				lval[2] = ptr[TYPE];
				lval[2] = 0;
				return (0);
			}
			else
				return (1);
		}
		if (ptr = findglb (sname))
			if (ptr[IDENT] != FUNCTION) {
				lval[0] = ptr;
				lval[1] = 0;
				if (ptr[IDENT] != ARRAY) {
					if (ptr[IDENT] == POINTER)
						lval[2] = ptr[TYPE];
					return (1);
				}
				immed ();
				prefix ();
				outstr (ptr);
				nl ();
				lval[1] = lval[2] = ptr[TYPE];
				lval[2] = 0;
				return (0);
			}
		blanks ();
		if (ch() != '(')
			error("undeclared variable");
		ptr = addglb (sname, FUNCTION, CINT, 0, PUBLIC);
		lval[0] = ptr;
		lval[1] = 0;
		return (0);
	}
	if (constant (num))
		return (lval[0] = lval[1] = 0);
	else {
		error ("invalid expression");
		immed ();
		onum (0);
		nl ();
		junk ();
		return (0);
	}
}

/*
 *	true if val1 -> int pointer or int array and val2 not pointer or array
 */
dbltest (val1, val2)
long int	val1[], val2[];
{
	return(FALSE);  // no byte pointers on nisan, only word pointers, so we never need to scale pointers to point at words
/*
	if (val1 == NULL)
		return (FALSE);
	if (val1[2] != CINT)
		return (FALSE);
	if (val2[2])
		return (FALSE);
	return (TRUE);
 */
}

/*
 *	determine type of binary operation
 */
result (lval, lval2)
long int	lval[],
	lval2[];
{
	if (lval[2] && lval2[2])
		lval[2] = 0;
	else if (lval2[2]) {
		lval[0] = lval2[0];
		lval[1] = lval2[1];
		lval[2] = lval2[2];
	}
}
		
constant (val)
long int	val[];
{
	if (number (val))
		immed ();
	else if (pstr (val))
		immed ();
	else if (qstr (val)) {
		immed ();
		printlabel (litlab);
		outbyte ('+');
	} else
		return (0);
	onum (val[0]);
	nl ();
	return (1);
}

number (val)
long int	val[];
{
	int	k, minus, base;
	char	c;

	k = minus = 1;
	while (k) {
		k = 0;
		if (match ("+"))
			k = 1;
		if (match ("-")) {
			minus = (-minus);
			k = 1;
		}
	}
	if (!numeric (c = ch ()))
		return (0);
	if (match ("0x") || match ("0X"))
		while (numeric (c = ch ()) ||
		       (c >= 'a' && c <= 'f') ||
		       (c >= 'A' && c <= 'F')) {
			inbyte ();
			k = k * 16 +
			    (numeric (c) ? (c - '0') : ((c & 07) + 9));
		}
	else {
		base = (c == '0') ? 8 : 10;
		while (numeric (ch ())) {
			c = inbyte ();
			k = k * base + (c - '0');
		}
	}
	if (minus < 0)
		k = (-k);
	val[0] = k;
	return (1);
}

pstr (val)
long int	val[];
{
	int	k;
	char	c;

	k = 0;
	if (!match ("'"))
		return (0);
	while ((c = gch ()) != 39) {
		c = (c == '\\') ? spechar(): c;
		k = (k & 255) * 256 + (c & 255);
	}
	val[0] = k;
	return (1);
}

qstr (val)
long int	val[];
{
	char	c;

	if (!match (quote))
		return (0);
	val[0] = litptr;
	while (ch () != '"') {
		if (ch () == 0)
			break;
		if (litptr >= LITMAX) {
			error ("string space exhausted");
			while (!match (quote))
				if (gch () == 0)
					break;
			return (1);
		}
		c = gch();
		litq[litptr++] = (c == '\\') ? spechar(): c;
	}
	gch ();
	litq[litptr++] = 0;
	return (1);
}

/*
 *	decode special characters (preceeded by back slashes)
 */
spechar() {
	char c;
	c = ch();

	if	(c == 'n') c = EOL;
	else if	(c == 't') c = TAB;
	else if (c == 'r') c = CR;
	else if (c == 'f') c = FFEED;
	else if (c == 'b') c = BKSP;
	else if (c == '0') c = EOS;
	else if (c == EOS) return 0; // XXX probably should be error

	gch();
	return (c);
}

/*
 *	perform a function call
 *
 *	called from "heir11", this routine will either call the named
 *	function, or if the supplied ptr is zero, will call the contents
 *	of HL
 *
 */
callfunction (ptr)
char	*ptr;
{
	int	nargs;

	nargs = 0;
	blanks ();
	if (ptr == 0)
		gpush ();
	while (!streq (line + lptr, ")")) {
		if (endst ())
			break;
		expression (NO);
		if (ptr == 0)
			swapstk ();
		gpush ();
		nargs = nargs + intsize();
		if (!match (","))
			break;
	}
	needbrack (")");
	if (aflag)
		gnargs(nargs / intsize());
	if (ptr)
		gcall (ptr);
	else
		callstk ();
	stkp = modstk (stkp + nargs);
}

needlval ()
{
	error ("must be lvalue");
}
