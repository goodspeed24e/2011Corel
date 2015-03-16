CC = gcc

SUNAUDIOLIB = /usr/demo/SOUND

CFLAGS = -O2 -Wall -I$(SUNAUDIOLIB)

## the second scheduler often defeats hand-pipelined loops for 21k.

C2FLAGS = -O2 -a ez.ach  -mreserved=i8,l8,b8,i9,l9,b9 \
		-DEZ -fno-schedule-insns2 -Wall -save-temps

LFLAGS =  -L$(SUNAUDIOLIB) -laudio -lm

CSRCS =	codebook.c data.c adapters.c filters.c

DSRCS = adapters.c filters.c  data.c codebook.c postfil.c 

################################################################### Sparc

COBJS = $(CSRCS:.c=.o) cio-sparc.o

DOBJS = $(DSRCS:.c=.o) dio-sparc.o

cio-sparc.o:	io-sparc.c
	$(CC) -c -o cio-sparc.o $(CFLAGS) -DCODER io-sparc.c

dio-sparc.o:	io-sparc.c
	$(CC) -c -o dio-sparc.o $(CFLAGS) -DDECODER io-sparc.c

cmain.o:	cmain.c
	$(CC) -c -DMAIN $(CFLAGS) cmain.c

dmain.o:	dmain.c
	$(CC) -c -DMAIN $(CFLAGS) dmain.c

ccelp:	$(COBJS)  cmain.o global.o
	$(CC) -o ccelp $(COBJS) cmain.o global.o $(CFLAGS) $(LFLAGS) 

dcelp: $(DOBJS) dmain.o global.o
	$(CC) -o dcelp $(DOBJS) dmain.o global.o $(CFLAGS) $(LFLAGS)

p-dcelp: $(DOBJS) dmain.o global.o
	purify $(CC) -o p-dcelp $(DOBJS) dmain.o global.o $(CFLAGS) $(LFLAGS)

SDSRC = $(DSRCS) io-sparc.c dmain.c global.c

s-dcelp: $(SDSRC)
	#load $(CFLAGS) $(SDSRC)

############################################################ EZ-LAB demo
	
.SUFFIXES: .asm .o2k .obj

.c.o2k:
	g21k -c $(C2FLAGS) -o $@ $<

.asm.o2k:
	g21k -c $(C2FLAGS) -o $@ $<

C2OBJS = $(CSRCS:.c=.o2k) i3.o2k fi3.o2k global.o2k

D2OBJS = $(DSRCS:.c=.o2k) i3.o2k fi3.o2k global.o2k

ALLOBJ = adapters.o2k codebook.o2k data.o2k filters.o2k postfil.o2k 

EZOBJ  = ezplay.o2k dcder.o2k coder.o2k

fio3.o2k:	fio21k.asm
	g21k $(C2FLAGS) -c -DCODER -DDECODER -DTASK3 -o fio3.o2k \
	fio21k.asm -save-temps

coder.o2k:  cmain.c
	g21k $(C2FLAGS) -c -o coder.o2k cmain.c -save-temps
	
dcder.o2k:  dmain.c
	g21k $(C2FLAGS) -c -o dcder.o2k dmain.c -save-temps
	
ezplay.o2k:  ezplay.c
	g21k $(C2FLAGS) -c -o ezplay.o2k ezplay.c -save-temps
	
EZOBJS = $(ALLOBJ) ezplay.o2k coder.o2k dcder.o2k global.o2k \
		fio3.o2k cbindex.o2k 020_hdr.obj
	
020_hdr.obj:	020_hdr.asm
	g21k -c -o 020_hdr.obj 020_hdr.asm

ezplay.exe: $(EZOBJS) $(ALLOBJ)
	g21k -o ezplay.exe $(C2FLAGS) 020_hdr.obj $(EZOBJS) $(ALLOBJ) \
	 -nomem -map

#######################################################  Cleanup

sparc-clean:
	-rm -f *.o

21k-clean:
	-rm -f *.o2k *.i *.is *.s

clean:	sparc-clean 21k-clean

