CC=clang

bin/sfdscli: sfds.c sfds.h sfds_test.c bin/stdio-drv.o
	$(CC) -g sfds.c sfds_test.c bin/*.o -o bin/sfdscli

bin/stdio-drv.o: drivers/stdio-drv.c drivers/stdio-drv.h
	$(CC) -g -c drivers/stdio-drv.c -o bin/stdio-drv.o

clean:
	rm -rf bin/*
