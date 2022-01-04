#	Requires System V make
#	@(#)Makefile 1.5 86/05/13
.SUFFIXES:	.o .c .c~ .h .h~
.PRECIOUS:	scclib.a
#	You'll probabably want to change these.  These are used by the compilers#	to figure out where the include files should go.
TARGDIR = /u/clewis/lib
INCDIR = "/u/clewis/src/scc/include/"

INSTFLAGS = -DINCDIR=$(INCDIR)
CFLAGS = '$(INSTFLAGS)' -g 
AR = ar
ARFLAGS = rv

# LIB = scclib.a  # related to the 'lib' directory somehow?
LIB = 

FE =	$(LIB)(data.o) \
	$(LIB)(error.o) \
	$(LIB)(expr.o) \
	$(LIB)(function.o) \
	$(LIB)(gen.o) \
	$(LIB)(io.o) \
	$(LIB)(lex.o) \
	$(LIB)(main.o) \
	$(LIB)(preproc.o) \
	$(LIB)(primary.o) \
	$(LIB)(stmt.o) \
	$(LIB)(sym.o) \
	$(LIB)(while.o)

# all:	scc8080 sccas09 sccvax sccm68k
# all:	sccm68k
all:	sccnisan

# $(FE) code8080.o codeas09.o codevax.o codem68k.o: defs.h data.h
# $(FE) codem68k.o: defs.h data.h
$(FE) codenisan.o: defs.h data.h

install:	all
	mv sccvax scc8080 sccas09 sccm68k $(TARGDIR)

#Alternately, you may have to do an lorder
$(LIB):	$(FE)
	-ranlib $(LIB)
	-ucb ranlib $(LIB)

#scc8080:	code8080.o $(LIB)
#	$(CC) -o scc8080 $(CFLAGS) $(LIB) code8080.o

#sccas09:	codeas09.o $(LIB)
	$(CC) -o sccas09 $(CFLAGS) $(LIB) codeas09.o

#sccvax:		codevax.o $(LIB)
#	$(CC) -o sccvax $(CFLAGS) $(LIB) codevax.o

# I started hacking stuff up here
#sccm68k:	codem68k.o $(LIB)
#	$(CC) -o sccm68k $(CFLAGS) codem68k.o data.o error.o expr.o function.o gen.o io.o lex.o main.o preproc.o primary.o stmt.o sym.o while.o

sccnisan:	code_nisan.o data.o error.o expr.o function.o gen.o io.o lex.o main.o preproc.o primary.o stmt.o sym.o while.o
	$(CC) -o sccnisan $(CFLAGS) code_nisan.o data.o error.o expr.o function.o gen.o io.o lex.o main.o preproc.o primary.o stmt.o sym.o while.o

print:
	pr -n defs.h data.h data.c error.c expr.c function.c gen.c \
		io.c lex.c main.c preproc.c primary.c stmt.c \
		sym.c while.c code*.c | lp
clean:
	rm -f code8080.o codeas09.o codevax.o codem68k.o *.o \
		     sccvax scc8080 sccas09 sccm68k sccnisan
