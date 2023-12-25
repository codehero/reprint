NAME=reprint
LIB=lib$(NAME).so
ARCH=ARCH_X86_64

all : lib/$(LIB) reprint

REPRINT_OBJ = reprint.o

src/$(LIB):
	ARCH=$(ARCH) make -C src

lib/$(LIB) : src/$(LIB)
	mkdir -p include lib
	cp src/$(LIB) lib/$(LIB)
	INC_DEST=../include make -C src header_install

%.o : %.c
	$(CC) -I./include -D$(ARCH) $(CFLAGS) -fPIC -c -o $@ $<

reprint : lib/$(LIB) $(REPRINT_OBJ)
	$(CC) -o $@ $(REPRINT_OBJ)  -fPIC -L./lib $(LDFLAGS) -lreprint -lm -lgcc_s -lgcc

clean:
	make -C src clean
	rm -rf include lib bin
