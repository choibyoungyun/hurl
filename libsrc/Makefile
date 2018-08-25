

# ###########################################################################
#  DEFINE Package Base Directory
# ###########################################################################
WORKHOME	= $(HOME)/workspace/KT114
REST_HOME   = $(HOME)/workspace/hurl
INSTALL_DEST= ../lib


# ###########################################################################
#  DEFINE Compiler & Linkage Editor
# ###########################################################################
INSTALL		= /usr/bin/install
LD			= g++
CC			= g++

PROGRAM1= libhgw.a
SRCS1   = worker.c \
		  hcurl.c \
		  eigw.c \
		  fcgi.c \
		  mchunk.c \
		  mpool.c \
		  stream.c \
		  socket.c \
		  signal.c \
		  log.c \
		  util.c
OBJS1   =$(SRCS1:.c=.o)


CFLAGS  = -g -W -Wall -Werror -std=gnu++11 ${DEBUG_OPTION}
INC_DIR =  -I../include	\
		  -I$(REST_HOME)/include \
		  -I$(WORKHOME)/inc \
		  -I$(HOME)/include
LIB_DIR = -L$(HOME)/lib \
		  -L$(WORKHOME)/lib
LIBS    = -ljlog -lcfg -lfile -lsignal -lshm -lbase -lcurl -lpthread

all: program1
program1 : $(OBJS1)
	$(AR) -r $(PROGRAM1) $(OBJS1)


install:
	$(INSTALL) $(PROGRAM1)  $(INSTALL_DEST)

clean:
	$(RM) $(PROGRAM1) $(OBJS1)
	$(RM) $(PROGRAM2) $(OBJS2)
	$(RM) core.*


.SUFFIXES : .c .o
.c.o  :
	$(CC) -g -c $(CFLAGS) $(INC_DIR) $<
