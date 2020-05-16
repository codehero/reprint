NAME=reprint
INC=reprint.h reprint_aux.h reprint_stdio.h arch.h msp430.h x86_64.h
LIB=lib$(NAME).so

CFLAGS+=-std=gnu11 -g -Wall -fPIC -D$(ARCH)
CXXFLAGS+=-std=gnu++14 -g -Wall -fPIC -D$(ARCH)

DEPEND = $(SOURCES:.c=.d)

SOURCES = reprint.c reprint_stdio.c reprint_aux.c
OBJECTS = $(SOURCES:.c=.o)

CC       = $(CROSS_COMPILE)gcc
CXX      = $(CROSS_COMPILE)g++
AR       = $(CROSS_COMPILE)ar
RANLIB   = $(CROSS_COMPILE)ranlib

all : $(LIB)

header_install :
	mkdir -p $(INC_DEST)/reprint
	cp $(INC) $(INC_DEST)/reprint

$(LIB): $(OBJECTS)
	$(LD) $(LDFLAGS) -shared -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPEND)
endif

%.d: %.c
	$(CC) -M ${CFLAGS} $< >$@

clean:
	rm -rf *.o *.d *.a bin lib *.so
