

# ###########################################################################
#  DEFINE Package Base Directory
# ###########################################################################
REST_HOME   = $(HOME)/workspace/hurl
INSTALL_DEST= $(HOME)/bin


# ###########################################################################
#  DEFINE Compiler & Linkage Editor
# ###########################################################################
INSTALL		= /usr/bin/install
LD			= g++
CC			= g++

PROGRAM1= hclient
SRCS1   = init.c \
		  job.c \
		  main.c \
		  eigw.c
OBJS1   =$(SRCS1:.c=.o)


PROGRAM2= hperf
SRCS2	= hperf.c
OBJS2   =$(SRCS2:.c=.o)

CFLAGS  = -g -W -Wall -Werror -std=gnu++11 ${DEBUG_OPTION}
INC_DIR =  -I../include	\
		  -I$(REST_HOME)/include \
		  -I$(WORKHOME)/inc \
		  -I$(HOME)/include
LIB_DIR = -L../lib \
		  -L$(HOME)/lib \
		  -L$(WORKHOME)/lib
LIBS    = -lhgw -ljson -ljlog -lcfg -lfile -lsignal -lshm -lbase -lcurl -lpthread

all: program1  program2
program1 : $(OBJS1)
	$(LD) -g -o $(PROGRAM1) $(OBJS1) $(LIB_DIR) $(LIBS)


program2 : $(OBJS2)
	$(LD) -g -o $(PROGRAM2) $(OBJS2) $(LIB_DIR) $(LIBS)

install:
	$(INSTALL) $(PROGRAM1)  $(INSTALL_DEST)

clean:
	$(RM) $(PROGRAM1) $(OBJS1)
	$(RM) $(PROGRAM2) $(OBJS2)
	$(RM) core.*


.SUFFIXES : .c .o
.c.o  :
	$(CC) -g -c $(CFLAGS) $(INC_DIR) $<
