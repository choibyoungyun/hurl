

# ###########################################################################
#  DEFINE Package Base Directory
# ###########################################################################
WORKHOME	= $(HOME)/workspace/KT114
INSTALL_DEST= $(APACHE_HOME)/cgi-bin


# ###########################################################################
#  DEFINE Compiler & Linkage Editor
# ###########################################################################
INSTALL		= /usr/bin/install
LD			= g++
CC			= g++

PROGRAM1= seigwd
SRCS1   = main.c msg.c job.c send.c recv.c bind.c
OBJS1   =$(SRCS1:.c=.o)


CFLAGS  = -g -W -Wall -Werror -std=gnu++11 $(DEBUG_OPTION)
INC_DIR = -I../include \
		  -I../../../include \
		  -I$(HOME)/include \
		  -I$(WORKHOME)/inc
LIB_DIR = -L$(HOME)/lib -L$(WORKHOME)/lib
LIBS    = -lcfg -lfile -lbase -lcurl

all: program1

program1 : $(OBJS1)
	$(LD) -g -o $(PROGRAM1) $(OBJS1) $(LIB_DIR) $(LIBS) -luv

install:
	$(INSTALL) $(PROGRAM1)  $(INSTALL_DEST)

clean:
	$(RM) $(PROGRAM1) $(OBJS1) core*


.SUFFIXES : .c .o
.c.o  :
	$(CC) -g -c $(CFLAGS) $(INC_DIR) $<

