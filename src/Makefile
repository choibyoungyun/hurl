

# ###########################################################################
#  DEFINE Package Base Directory
# ###########################################################################
APACHE_HOME	= $(HOME)/apache
REST_HOME   = $(HOME)/workspace/restapi
INSTALL_DEST= $(HOME)/bin


# ###########################################################################
#  DEFINE Compiler & Linkage Editor
# ###########################################################################
INSTALL		= /usr/bin/install
LD			= g++
CC			= g++

PROGRAM1= htest
SRCS1   = hcurl.c \
		  htest.c
OBJS1   =$(SRCS1:.c=.o)


CFLAGS  = -g -W -Wall -Werror -std=gnu++11 -D_DEBUG
#CFLAGS  = -g -std=gnu++11 -D_DEBUG
INC_DIR = -I. -I$(REST_HOME)/include -I$(APACHE_HOME)/include -I$(HOME)/include
LIB_DIR = -L$(HOME)/lib
LIBS    = -lcurl -luv

all: program1
program1 : $(OBJS1)
	$(LD) -g -o $(PROGRAM1) $(OBJS1) $(LIB_DIR) $(LIBS)


install:
	$(INSTALL) $(PROGRAM1)  $(INSTALL_DEST)

clean:
	$(RM) $(PROGRAM1) $(OBJS1)


.SUFFIXES : .c .o
.c.o  :
	$(CC) -g -c $(CFLAGS) $(INC_DIR) $<
