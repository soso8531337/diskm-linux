CC=mipsel-linux-gcc
CROSS=mipsel-linux-
STRIP=$(CROSS)strip
AR=$(CROSS)ar
TARGET=diskm
DSTLIB=libdiskm.so
CFLAGS= -fPIC -g -Wall -Iinclude
ALL: clean $(TARGET) $(DSTLIB) disktst
FILE=comlib.o disk_manager.o exfat.o hfs.o ipc_msg.o \
 disk_triger.o libblkid-tiny.o ntfs.o sg_io.o vfat.o 
diskm: $(FILE) main.c
	$(CC) -o $(TARGET)  main.c $(FILE) -lpthread 
$(DSTLIB): $(FILE)
	$(CC) -o $(DSTLIB) -shared $^ 
	$(STRIP) $(DSTLIB)
	$(AR) rcs libdiskm.a $^ 
disktst: dtest.c
	$(CC) -O2 -o disktst dtest.c $(FILE)
%.o:%.c
	$(CC) -c -o $@ $^ $(CFLAGS) 
clean: 
	rm -f *.o diskm $(DSTLIB) 
