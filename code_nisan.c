
#include <stdio.h>
#include "defs.h"
#include "data.h"

int	needr0;
int	needh;
int	need_rest_of_immed=0;


void ppubext(char*);


/*
 *	Some predefinitions:
 *
 *	INTSIZE is the size of an integer in the target machine
 *	BYTEOFF is the offset of an byte within an integer on the
 *		target machine. (ie: 8080,pdp11 = 0, 6809 = 1,
 *		360 = 3)
 *	This compiler assumes that an integer is the SAME length as
 *	a pointer - in fact, the compiler uses INTSIZE for both.
 */
#define	INTSIZE	1
#define	BYTEOFF	0

// local def

#define STACKBOT 20

/*
 *	print all assembler info before any code is generated
 *
 */
header ()
{
	outstr("//\tSmall C Nisan's Hack Academic VM\n//\tWalters (0.0,1/2/2022) ");
	FEvers();
	nl();

        // initialize a stack pointer
	ot("@"); outdec(STACKBOT); nl();   // starting here and growing upward
	ol("D=A");
	ol("@stack");  // assembler allocates stack
	ol("M=D");

	// initialize a backup space for the primary "data" register
	// this isn't used but I kind of like having the stack pointer at loc 16.
	ol("@primary");  // assembler allocates primary
	ol("M=0");

}

/*
 *  called on the end of a statement in the lexer
 */

gns()
{
	ot("// ^-- "); outstr(line); nl();
}

nl()
{
	outbyte(EOL);
	if(need_rest_of_immed) {
		// kinda goofy.  the front-end should pass the immediate value to immed() in here instead of trying to output the number itself so we don't have to hook in here to finish up.  XXX todo.
		ot("D=A\n"); // also avoid using ol() which calls us recursively
		need_rest_of_immed = 0;
	}
}

initmac() {
	defmac("nisan\t1");
	defmac("unix\t1");
	defmac("smallc\t1");
}

int galign(int t)
{   
        return(t);
}
/*
  what the fuck does this do?
XXX oh, heh, this might fix the 64->16 bit negative numbers for stack offset problem from the naive initial attempt.
  alternatively:
galign(t)
int t;
{
	int sign;
	if (t < 0) {
		sign = 1;
		t = -t;
	} else
		sign = 0;
	t = (t + INTSIZE - 1) & ~(INTSIZE - 1);
	t = sign? -t: t;
	return (t);
}
*/


/*
 *	return size of an integer
 */
intsize() {
	return(INTSIZE);
}

/*
 *	return offset of ls byte within word
 *	(ie: 8080 & pdp11 is 0, 6809 is 1, 360 is 3)
 */
byteoff() {
	return(BYTEOFF);
}

/*
 *	Output internal generated label prefix
 *      Nisan has labels like "(label)" instead of like "label:"
 */
olprfix() {
	outstr("(LL");
}

/*
 *	Output a label definition terminator
 *      Nisan has labels like "(label)" instead of like "label:"
 */
col ()
{
	outstr(")");
}

/*
 *	begin a comment line for the assembler
 *      Nisan continues to be non-standard here too.
 *
 */
comment ()
{
	outstr("//");
}

/*
 *	Output a prefix in front of user labels
 *      Nisan has labels like "(label)" instead of like "label:"
 */
prefix () {
	outstr("(_"); 
}


/*
 *	print any assembler stuff needed after all code
 *
 */
trailer ()
{
}

/*
 *	function prologue
 */
prologue ()
{
	/* this is where we'd put splimit stuff */
}

/*
 *	text (code) segment
 *      default and only segment on Nisan
 */
gtext ()
{
}

/*
 *	data segment
 *      not supported by Nisan.  separate RAM/ROM and instructions cannot access constants in ROM.  XXX if things need data segments, they will break.  looks like only the switch() implementation uses this.
 */
gdata ()
{
        // error("can't do data segments\n");
	// ol("data");
}

/*
 *  Output the variable symbol at scptr as an extrn or a public
 *  XXX this probably won't work either
 * it tries to do a few of these automatically and then does one for 'main()' and I assume any other funcs defined
 */
void
ppubext(scptr) char *scptr; {
        ol("// ppubext TODO ");
	if(scptr[STORAGE] == STATIC) return;
        // error("extrn public probably needs work");
	// ot("global\t");
	// prefix();
	// outstr(scptr);
	// nl();
}

/*
 * Output the function symbol at scptr as an extrn or a public
 */
fpubext(scptr) char *scptr; {
	ppubext(scptr);
}

/*
 *  Output a decimal number to the assembler file
 */
onum(num) int num; {
	outdec(num);	/* pdp11 needs a "." here */
}


/*
 *	fetch a static memory cell into the primary register
 *      XXX not being used in our test case?
 */
getmem (sym)
char	*sym;
{
        ol("// getmem TODO half implemented and untested");
	if ((sym[IDENT] != POINTER) & (sym[TYPE] == CCHAR)) {
                // char case
                error("chars not supported");
		// ot ("mov.b\t");
		// prefix ();
		// outstr (sym + NAME);
	        // outstr(",%d0\n");
	} else {
                ot("@");      // maybe 'ot()' means 'out tab indentend' but anyway that's what it does.  anyway, load a constant into the address register, then move it to the data register.
		outstr(sym + NAME);   // this ISA is horrible which is why I'm not writing it by hand.
		nl();

		ol("D=M");        // D=M fetches memory contents
	}
}

/*
 *	fetch the address of the specified symbol into the primary register
 *
 */
void
getloc(sym)
char	*sym;
{
        ol("// getloc");
	if (sym[STORAGE] == LSTATIC) {
        	ol("// XXX unimplemented TODO ");
		error("XXX getloc with LSTATIC probably needs work");  // at the very least, emit the derpy @<const> \n D=A \n junk
		immed();
		printlabel(glint(sym));
		nl();
	} else {
		unsigned int address = STACKBOT + stkp + (glint(sym) - stkp);  // top of the stack relative addressing
                char tmp[1000];
		sprintf(tmp, "debug: getloc (code generator in code_nisan) on %s: glint(sym) = %lx, stkp = %lx, computed memory address=%u\n", sym, glint(sym), stkp, address);
                fprintf(stderr, tmp);
                outstr("// "); outstr(tmp);
                // stkp is the current stack pointer which includes anything on the evaluation stack.
		// glint() fetches the stkp recorded when the symbol was created with declloc()/addloc().
		// glint(sym) is observed stpk when the symbol was created.
		// stkkp - glint(sym) is the offset from the current top of the stack to where sym was placed
		// making the evaluation stack separate would simplify this process of finding variables in stack frames.
		// I'm also skeptical that this will work for subroutine calls.
		ot("@"); onum( STACKBOT + stkp + (glint(sym) - stkp) ); nl();  // top of the stack relative addressing
		ol("D=A");                  // D=A just uses the address register, not the memory contents, and this operation wants the memory address of the stack position, not the contents
	}
}

/*
 *	store the primary register into the specified static memory cell
 *
 */
putmem (sym)
char	*sym;
{
        ol("// putmem");
	if ((sym[IDENT] != POINTER) & (sym[TYPE] == CCHAR)) {
                error("bytes don't work");
	} else {
		ot("@_");
	        outstr(sym + NAME);
		nl();
		ol("D=M");
        }
}

/*
 *	store the specified object type in the primary register
 *	at the address on the top of the stack
 *
 */
putstk (typeobj)
char	typeobj;
{
        ol("// putstk");
	if (typeobj == CCHAR) {
		error("putstrk char not supported");
		return 0;
	}
//        ol ("mov.l\t(%sp)+,%a0");
//        if (typeobj == CCHAR)
//                ol ("mov.b\t%d0,(%a0)");
//        else
//                ol ("mov.l\t%d0,(%a0)");
// oh, shit, this is indirect

        ol("@stack");
        ol("A=M");    // indirect operation.  use the address in the stack to fetch/store from/to.
	ol("A=M");    // but indirect again... address the stack pointer pointed to is itself an address to load/store from/to.
        ol("M=D");    // store data
        // post-dec the stack pointer to pop the address we just used
        ol("@stack");
        ol("M=M-1");  // dec the stack pointer to remove from the stack, assuming it grows upwards.  post-decs and pre-incs.  so the first slot doesn't get used as currently set (testing stuff here).
	stkp = stkp - INTSIZE;
}

/*
 *	fetch the specified object type indirect through the primary
 *	register into the primary register
 *
 */
indirect (typeobj)
char	typeobj;
{
        ol("// indirect ");
	if (typeobj == CCHAR) {
                error("bytes don't work");
                return 0;
	}
        ol("A=D"); // primary register, which is our data register, supplies the address
	// ol("A=M"); // indirect operation ... nope, one too many.  verify this then delete this line.
        ol("D=M"); // whatever that indirectly pointed to goes in to our data register
}

/*
 *	swap the primary and secondary registers
 *	ie, swap top of stack and data, but also looks like this is never used by the front end
 *
 */
swap ()
{
        ol("// swap TODO UNIMPL");
	// don't have a secondary reg really
	// ok, it looks like primary/expr/etc never call this but they do call swapstk() in exactly one place, so secondary seems like an implmenentation detail
	// ol ("mov.l\t%d0,%d2\n\tmov.l\t%d1,%d0\n\tmov.l\t%d2,%d1");
}

/*
 *	print partial instruction to get an immediate value into
 *	the primary register
 *
 */
immed ()
{
        ol("// immed");
	ot("@");
	need_rest_of_immed = 1;
}

/*
 *	push the primary register onto the stack
 *
 */
gpush ()
{
        ol("// gpush");
        ol("@stack");
	ol("M=M+1");  // pre-inc the stack pointer
	ol("A=M");    // load stack pointer in to A
        ol("M=D");    // store data
	stkp = stkp + INTSIZE;
}

/*
 *	pop the top of the stack into the secondary register
 *	gpop is never used in the front end, and the top of the stack is our secondary register.  secondary register is an implementation detail.
 *
 */
gpop ()
{
        ol("// gpop TODO");

  /*
	// back up our primary register
	ol("@primary");
	ol("M=D");
	// secondary register by way of the data register.
	ol("@stack");
	ol("A=M");   // indirect operation... use the stack pointer as an address to read/write.
	ol("D=M");   // read from the location specified by the stack pointer.
	// write to the secondary register
	ol("@secondary");
	ol("M=D");
	// post inc the stack pointer
        ol("@stack");
	ol("M=M-1");   // post-dec
	// restore primary register
	ol("@primary");
	ol("D=M");
	stkp = stkp - INTSIZE;
   */
}

/*
 *	swap the primary register and the top of the stack
 *
 */
swapstk ()
{
        ol("// swapstk TODO");
	// ol ("mov.l\t(%sp)+,%d2\nmov.l\t%d0,-(%sp)\nmov.l\t%d2,%d0");
}

/*
 *	call the specified subroutine name
 *
 */
gcall (sname)
char	*sname;
{
        ol("// gcall TODO");
	if (*sname == '^') {
		ot ("jsr\tT");
		outstr (++sname);
	} else {
		ot ("jsr\t");
		prefix ();
		outstr (sname);
	}
	nl ();
}

/*
 *	return from subroutine
 *
 */
gret ()
{
        ol("// grts TODO");
	// ol ("rts");
}

/*
 *	perform subroutine call to value on top of stack
 *
 */
callstk ()
{
        ol("// callstk TODO");
	// ol ("jsr\t(%sp)+");
	stkp = stkp - INTSIZE;  // post-dec
}

/*
 *	jump to specified internal label number
 *
 */
jump (label)
int	label;
{
        ol("// jump");
	ot("@LL"); outdec(label); nl();
	ol("0;JMP");
}

/*
 *	test the primary register and jump if false to label
 *
 */
testjump (label, ft)
int	label,
	ft;
{
        ol("// testjump");

	// ol ("cmp.l\t%d0,&0");
	// if (ft)
 	//	ot ("beq\t");
	// else
	//	ot ("bne\t");
	// printlabel(label);
	// nl ();

	if(ft) {
		ot("@LL"); outdec(label); nl(); // jump target
		ol("D;JEQ");
	} else {
		ot("@LL"); outdec(label); nl(); // jump target
		ol("D;JNE");
	}
}

/*
 *	print pseudo-op  to define a byte
 *
 */
defbyte ()
{
	error("byte not implemented");
	ot ("byte\t");
}

/*
 *	print pseudo-op to define storage
 *
 */
defstorage ()
{
	error("space not implemented");
	ot ("space\t");
}

/*
 *	print pseudo-op to define a word
 *
 */
defword ()
{
	error("word not implemented");
	ot ("long\t");
}

/*
 *	modify the stack pointer to the new value indicated
 *	we mirror with our stack pointer here what the compiler is doing with its
 *
 */
modstk (newstkp)
int	newstkp;
{
        ol("// modstk");
	int	k;

	fprintf(stderr, "modstk (backend): old stack pointer %d, new stack pointer %d\n", stkp, newstkp);
	k = newstkp - stkp;
	if (k == 0)
		return (newstkp);


/*
	ot("@"); onum(abs(k)); nl();
	ol("D=A");
	ol("@stack");
	if( k > 0 ) {
		ol("M=M+D"); // extend stack higher in memory.  not sure if I have this straight now.
	} else {
		// *(int *)0=0; // stop in gdb 
		ol("M=M-D"); // remove items from stack
	}
 */

	ot("@"); onum(STACKBOT+newstkp); nl();
	ol("D=A");
	ol("@stack");
	ol("M=D");  // just set the stack pointer absolutely

	return (newstkp);
}

/*
 *	multiply the primary register by INTSIZE
 *      this seems odd.  wordsize here is 2 bytes, so double it.
 */
gaslint ()
{
        ol("// asl");

	// ol ("asl.l\t&2,%d0");
	// kludge... add the number to itself to double it
	ol("A=D");
	ol("D=D+A");
}

/*
 *	divide the primary register by INTSIZE
 */
gasrint()
{
        ol("// asr TODO ");
	ol ("asr.l\t&2,%d0");
}

/*
 *	Case jump instruction
 *      ahh, part of switch()
 *      the gcall() stuff generates a jsr style call to a user supplied routine or part of the library, and none of those are written/built/tested here
 */
gjcase() {
        ol("// case TODO ");
	gcall("^case");
}

/*
 *	add the primary and secondary registers
 *	if lval2 is int pointer and lval is int, scale lval
 *      huh?  but Nisan's architecture addresses words only so no need to scale pointers to words
 */
gadd (lval, lval2) int *lval, *lval2;
{
        ol("// gadd");

	// if (dbltest (lval2, lval)) {  // "true if val1 -> int pointer or int array and val2 not pointer or array"
	//	ol ("asl.l\t&2,(%sp)");
	//}
	// ol ("add.l\t(%sp)+,%d0");
	// stkp = stkp + INTSIZE;

	ol("@stack");
	ol("A=M");   // indirect operation... use the stack pointer as an address to read/write.
	ol("D=D+M");

	ol("@stack");
	ol("M=M-1");   // post-dec stack pointer
	stkp = stkp - INTSIZE;
}

/*
 *	subtract the primary register from the secondary
 *
 */
gsub ()
{
        ol("// gsub TODO ");
	// ol ("mov.l\t(%sp)+,%d2");
	// ol ("sub.l\t%d0,%d2");
	// ol ("mov.l\t%d2,%d0");
	// stkp = stkp - INTSIZE;
}

/*
 *	multiply the primary and secondary registers
 *	(result in primary)
 *
 */
gmult ()
{
        ol("// gmult TODO ");
	// gcall ("^mult");
	// stkp = stkp - INTSIZE;
}

/*
 *	divide the secondary register by the primary
 *	(quotient in primary, remainder in secondary)
 *
 */
gdiv ()
{
        ol("// gdiv TODO ");
	// gcall("^div");
	// stkp = stkp - INTSIZE;
}

/*
 *	compute the remainder (mod) of the secondary register
 *	divided by the primary register
 *	(remainder in primary, quotient in secondary)
 *
 */
gmod ()
{
        ol("// gmod TODO ");
	// gcall("^mod");
	// stkp = stkp - INTSIZE;
}

/*
 *	inclusive 'or' the primary and secondary registers
 *
 */
gor ()
{
        ol("// gor TODO ");
	// ol ("or.l\t(%sp)+,%d0");  // thinking the 68000 backend's use of the stack is incorrect given local variables are addressed relative the top of the stack
	// stkp = stkp - INTSIZE;
}

/*
 *	exclusive 'or' the primary and secondary registers
 *
 */
gxor ()
{
        ol("// gxor TODO ");
	// ol ("mov.l\t(%sp)+,%d1");
	// ol ("eor.l\t%d1,%d0");
	// stkp = stkp - INTSIZE;
}

/*
 *	'and' the primary and secondary registers
 *
 */
gand ()
{
        ol("// gand TODO ");
	// ol ("and.l\t(%sp)+,%d0");
	// stkp = stkp - INTSIZE;
}

/*
 *	arithmetic shift right the secondary register the number of
 *	times in the primary register
 *	(results in primary register)
 *
 */
gasr ()
{
        ol("// gasr TODO ");
	// ol ("mov.l\t(%sp)+,%d1");
	// ol ("asr.l\t%d0,%d1");
	// ol ("mov.l\t%d1,%d0");
	// stkp = stkp - INTSIZE;
}

/*
 *	arithmetic shift left the secondary register the number of
 *	times in the primary register
 *	(results in primary register)
 *
 */
gasl ()
{
        ol("// gasl TODO ");
	// ol ("mov.l\t(%sp)+,%d1");
	// ol ("asl.l\t%d0,%d1");
	// ol ("mov.l\t%d1,%d0");
	// stkp = stkp - INTSIZE;
}

/*
 *	two's complement of primary register
 *
 */
gneg ()
{
        ol("// gneg TODO ");
	// ol ("neg.l\t%d0");
}

/*
 *	logical complement of primary register
 *
 */
glneg ()
{
        ol("// glneg TODO ");
	// gcall("^lneg");
}

/*
 *	one's complement of primary register
 *
 */
gcom ()
{
        ol("// gcom TODO ");
	// ol ("not\t%d0");
}

/*
 *	convert primary register into logical value
 *
 */
gbool ()
{
        ol("// gbool TODO ");
	// gcall ("^bool");
}
/*
 *	increment the primary register by 1 if char, INTSIZE if
 *      int
 *      Nisan's architecture doesn't have char pointers and pointers always point to words so always inc by 1
 */
ginc (lval) int lval[];
{
        ol("// ginc");

	//if (lval[2] == CINT)
	//	ol ("addq.l\t&4,%d0");
	// else
	// 	ol ("addq.l\t&1,%d0");

	ol("D=D+1");  // finally, a task Nisan's architecture can actually do without massive dramatics
}

/*
 *	decrement the primary register by one if char, INTSIZE if
 *	int
 *      as above, pointers always point to words
 */
gdec (lval) int lval[];
{
        ol("// gdec");

	// if (lval[2] == CINT)
	// 	ol ("subq.l\t&4,%d0");
	// else
	// 	ol ("subq.l\t&1,%d0");

	ol("D=D-1");
}

/*
 *	following are the conditional operators.
 *	they compare the secondary register against the primary register
 *	and put a literl 1 in the primary if the condition is true,
 *	otherwise they clear the primary register
 *
 */

/*
 *	equal
 *
 */
geq ()
{
        ol("// geq");
	ol("@stack");
	ol("A=M");   // indirect operation... use the stack pointer as an address to read/write.
	ol("D=D-M"); // leave as 0 if they are the same.  testjump() will do a JNE later.

	ol("@stack");
	ol("M=M-1");   // post-dec stack pointer
	stkp = stkp - INTSIZE;
}

/*
 *	not equal
 *
 */
gne ()
{
        ol("// gne");
	ol("@stack");
	ol("A=M");   // indirect operation... use the stack pointer as an address to read/write.
	ol("D=D-M");
	ol("D=!D"); // D=D-M leaves D=0 if they are the same so invert the sense of the test so that D=-1 if they are the same and 0 if not same.  seems to be the sense wanted later in testjump().

	ol("@stack");
	ol("M=M-1");   // post-dec stack pointer
	stkp = stkp - INTSIZE;
}

/*
 *	less than (signed)
 *      XXX except not signed here
 *      arguments must be swapped because this works with JGT, not JLT
 *
 */
glt ()
{
        ol("// glt");
	int label1 = getlabel();  // next sequential label number
	int label2 = getlabel();  // next sequential label number
	ol("@stack");
	ol("A=M");   // indirect operation... use the stack pointer as an address to read/write.
	ol("D=D-M");
	ot("@LL"); outdec(label1); nl(); // jump target
	ol("D;JGT");  // jump to label1 if less-than-or-equal.  argumets must be swapped so reversing the test.
	ot("@LL"); outdec(label2); nl(); // jump target
	ol("D=1;JMP");   // true ; jump to end
        outstr("(LL"); outdec(label1); outstr(")"); nl(); // if-less-than target
	ol("D=0");       // false and fall-through to end
        outstr("(LL"); outdec(label2); outstr(")"); nl(); // end target

	ol("@stack");
	ol("M=M-1");   // post-dec stack pointer
	stkp = stkp - INTSIZE;
}
/*
 *	less than or equal (signed)
 *
 */
gle ()
{
        ol("// gge TODO ");
}

/*
 *	greater than (signed)
 *      XXX except not signed here
 *	and arguments swapped so sense of test reversed.
 *
 */
ggt ()
{
        ol("// ggt");
	int label1 = getlabel();  // next sequential label number
	int label2 = getlabel();  // next sequential label number
	ol("@stack");
	ol("A=M");   // indirect operation... use the stack pointer as an address to read/write.
	ol("D=D-M");
	ot("@LL"); outdec(label1); nl(); // jump target
	ol("D;JLT");  // jump to label1 if not greater-than
	ot("@LL"); outdec(label2); nl(); // jump target
	ol("D=1;JMP");   // true; jump to end
        outstr("(LL"); outdec(label1); outstr(")"); nl(); // if-greater-than target
	ol("D=0");       // false and fall-through to end
        outstr("(LL"); outdec(label2); outstr(")"); nl(); // end target

	ol("@stack");
	ol("M=M-1");   // post-dec stack pointer
	stkp = stkp - INTSIZE;
}

/*
 *	greater than or equal (signed)
 *
 */
gge ()
{
        ol("// gge TODO ");
}

/*
 *	less than (unsigned)
 *
 */
gult ()
{
        ol("// gult TODO ");
}

/*
 *	less than or equal (unsigned)
 *
 */
gule ()
{
        ol("// gule TODO ");
}

/*
 *	greater than (unsigned)
 *
 */
gugt ()
{
        ol("// gugt TODO ");
}

/*
 *	greater than or equal (unsigned)
 *
 */
guge ()
{
        ol("// guge TODO ");
}

inclib() {
#ifdef	flex
	return("B.");
#endif
#ifdef	unix
	return(INCDIR);
#endif
#ifdef	cpm
	return("B:");
#endif
}

/*	Squirrel away argument count in a register that modstk/getloc/stloc
	doesn't touch.
*/

gnargs(d)
int	d; 
{
        ol("// gnargs TODO ");
	// ot ("mov.l\t&");
	// onum(d);
	// outstr(",%d3\n");
}

assemble(s)
char    *s;
{
	return(0);
}

link() {
	return(0);
}
