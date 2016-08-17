bin/sfdscli: sfds.c sfds.h sfds_test.c
	$(CC) -g sfds.c sfds_test.c -o bin/sfdscli

clean:
	rm -rf bin/*
