CC=clang

bin/sfdscli: sfds.c sfds.h sfds_test.c bin/stdio-drv.o bin/crc32.o
	$(CC) -g sfds.c sfds_test.c bin/*.o -o bin/sfdscli

bin/stdio-drv.o: drivers/stdio-drv.c drivers/stdio-drv.h
	$(CC) -g -c drivers/stdio-drv.c -o bin/stdio-drv.o

bin/crc32.o: crc32.c crc32.h
	$(CC) -g -c crc32.c -o bin/crc32.o

clean:
	rm -rf bin/*
