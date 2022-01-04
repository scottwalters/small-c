/*	File sym.c: 2.1 (83/03/20,16:02:19) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/*
 *	declare a static variable
 */
void
declglb (typ, stor)
int	typ,
	stor;
{
	int	k, j;
	char	sname[NAMESIZE];

	FOREVER {
		FOREVER {
			if (endst ())
				return;
			k = 1;
			if (match ("*"))
				j = POINTER;
			else
				j = VARIABLE;
			if (!symname (sname))
				illname ();
			if (findglb (sname))
				multidef (sname);
			if (match ("[")) {
				k = needsub ();
				if (k || stor == EXTERN)
					j = ARRAY;
				else
					j = POINTER;
			}
			addglb (sname, j, typ, k, stor);
			break;
		}
		if (!match (","))
			return;
	}
}

/*
 *	declare local variables
 *
 *	works just like "declglb", but modifies machine stack and adds
 *	symbol table entry with appropriate stack offset to find it again
 */
void
declloc (typ, stclass)
int	typ, stclass;
{
	int	k, j;
	char	sname[NAMESIZE];

	FOREVER {
		FOREVER {
			if (endst ())
				return;
			if (match ("*"))
				j = POINTER;
			else
				j = VARIABLE;
			if (!symname (sname))
				illname ();
			if (findloc (sname))
				multidef (sname);
			if (match ("[")) {
				k = needsub ();
				if (k) {
					j = ARRAY;
					if (typ == CINT)
						k = k * intsize();
				} else {
					j = POINTER;
					k = intsize();
				}
			} else
				if ((typ == CCHAR) & (j != POINTER))
					k = 1;
				else
					k = intsize();
			if (stclass != LSTATIC) {
				k = galign(k);
				/* stkp = modstk(stkp - k);  XXX stack growing up for nisan... arbitrary but testing avoiding negative numbers since that's currently getting mangled going from 64 to 16 bit */
				/* stkp = modstk(stkp + k);  XXX end of the array vs the start is recorded when the stack grows upwards */
				fprintf(stderr, "debug: declloc: adding %s, stk=%lx\n", sname, stkp);
				addloc(sname, j, typ, stkp, AUTO);  // j is ARRAY, POINTER, VARIABLE, k is storage size, stkp is relative stack location
				stkp = modstk(stkp + k); // XXXXXX testing allocating storage after
				// fprintf(stderr, "debug: declloc: added %s, new stk=%lx\n", sname, stkp);
			} else {
				addloc( sname, j, typ, k, LSTATIC);
			}
			break;
		}
		if (!match (","))
			return;
	}
}

/*
 *	get required array size
 */
int
needsub ()
{
	int	num[1];

	if (match ("]"))
		return (0);
	if (!number (num)) {
		error ("must be constant");
		num[0] = 1;
	}
	if (num[0] < 0) {
		error ("negative size illegal");
		num[0] = (-num[0]);
	}
	needbrack ("]");
	return (num[0]);
}

char *
findglb (sname)
char	*sname;
{
	char	*ptr;

	ptr = STARTGLB;
	while (ptr != glbptr) {
                // fprintf(stderr, "debug: looking for %s at offset %ud which holds %s\n", sname, ptr, ptr);
		if (astreq (sname, ptr, NAMEMAX))
			return (ptr);
		ptr = ptr + SYMSIZ;
	}
	// fprintf(stderr, "debug: findglb on ``%s`` failed.\n", sname);
	return (0);
}

char *
findloc (sname)
char	*sname;
{
	char	*ptr;

	ptr = locptr;
	while (ptr != STARTLOC) {
		ptr = ptr - SYMSIZ;
		if (astreq (sname, ptr, NAMEMAX))
			return (ptr);
	}
	return (0);
}

addglb (sname, id, typ, value, stor)
char	*sname, id, typ;
int	value,
	stor;
{
	char	*ptr;

	if (cptr = findglb (sname))
		return (cptr);
	if (glbptr >= ENDGLB) {
		error ("global symbol table overflow");
		return (0);
	}
	// fprintf(stderr, "debug: addglb adding ``%s``.\n", sname);
	cptr = ptr = glbptr;
	while (an (*ptr++ = *sname++));
	cptr[IDENT] = id;
	cptr[TYPE] = typ;
	cptr[STORAGE] = stor;
	cptr[OFFSET] = value & 0xff;	
	cptr[OFFSET+1] = (value >> 8) & 0xff;
	glbptr = glbptr + SYMSIZ;
	return (cptr);
}

addloc (sname, id, typ, value, stclass)
char	*sname, id, typ;
int	value, stclass;
{
	char	*ptr;
	int	k;
	char *origsname = sname;

	if (cptr = findloc (sname))
		return (cptr);
	if (locptr >= ENDLOC) {
		error ("local symbol table overflow");
		return (0);
	}
	cptr = ptr = locptr;
	while (an (*ptr++ = *sname++));
	cptr[IDENT] = id;
	cptr[TYPE] = typ;
	cptr[STORAGE] = stclass;
	if (stclass == LSTATIC) {
		gdata();
		printlabel(k = getlabel());
		col();
		defstorage();
		onum(value);
		nl();
		gtext();
		value = k;
	} else
		value = galign(value);
	cptr[OFFSET] = value & 0xff;               // XXX it would be better to have a parrallel array of long ints (or void *s) instead of requiring pointers to be 16 bits and doing this kludge
	cptr[OFFSET+1] = (value >> 8) & 0xff;
	// fprintf(stderr, "debug: addloc: adding %s, offset=%lx, low=%hhu, high=%hhu\n", origsname, value, cptr[OFFSET], cptr[OFFSET+1]);
	locptr = locptr + SYMSIZ;
	return (cptr);
}

/*
 *	test if next input string is legal symbol name
 *
 */
symname (sname)
char	*sname;
{
	int	k;
	char	c;

	blanks ();
	if (!alpha (ch ()))
		return (0);
	k = 0;
	while (an (ch ()))
		sname[k++] = gch ();
	sname[k] = 0;
	return (1);
}

illname ()
{
	error ("illegal symbol name");
}

multidef (sname)
char	*sname;
{
	error ("already defined");
	comment ();
	outstr (sname);
	nl ();
}

glint(syment)
char *syment; 
{
	int l,u,r;
	// fprintf(stderr, "debug: glint: fetching %s, low=%hhu, high=%hhu\n", syment, syment[OFFSET], syment[OFFSET+1]);
	l = syment[OFFSET];
	u = syment[OFFSET+1];
	r = (l & 0xff) + ((u << 8) & ~0x00ff);
	return (r);
}
