NAME=reprint
LIB=lib$(NAME).so
ARCH=ARCH_X86_64

all : lib/$(LIB)

src/$(LIB):
	ARCH=$(ARCH) make -C src

lib/$(LIB) : src/$(LIB)
	mkdir -p include lib
	cp src/$(LIB) lib/$(LIB)
	INC_DEST=../include make -C src header_install

clean:
	make -C src clean
	rm -rf include lib bin
