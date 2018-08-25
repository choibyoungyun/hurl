

# ###########################################################################
#  DEFINE Package Base Directory
# ###########################################################################
APACHE_HOME	= $(HOME)/apache
INSTALL_DEST= $(APACHE_HOME)/cgi-bin


# ###########################################################################
#  DEFINE Compiler & Linkage Editor
# ###########################################################################
INSTALL		= /usr/bin/install
LD			= g++
CC			= g++

PROGRAM1= notify_fcgi
SRCS1   = \
		  notify.c \
		  main.c 
OBJS1   =$(SRCS1:.c=.o)


CFLAGS  = -g -W -Wall -Werror -std=gnu++11
#CFLAGS  = -g -W -Wall -std=gnu++11 -D_HAF_DEBUG
INC_DIR =  -I../include	\
		  -I$(REST_HOME)/include \
		  -I$(WORKHOME)/inc \
		  -I$(HOME)/include
LIB_DIR = -L../lib \
		  -L$(HOME)/lib \
		  -L$(WORKHOME)/lib
LIBS    = -lhgw -ljlog -lcfg -lfile -lsignal -lshm -lbase -lfcgi -lpthread

all: program1
program1 : $(OBJS1)
	$(LD) -g -o $(PROGRAM1) $(OBJS1) $(LIB_DIR) $(LIBS)

install:
	$(INSTALL) $(PROGRAM1)  $(INSTALL_DEST)

clean:
	$(RM) $(PROGRAM1) $(OBJS1)
	$(RM) core.*


.SUFFIXES : .tbc .c .o
.c.o  :
	$(CC) -g -c $(CFLAGS) $(INC_DIR) $<

