#include <stdint.h>
#include "../sfds.h"

#ifndef _STDIO_DRV_
#define _STDIO_DRV_

#define STDIO_DRV_FILE_SIZE (524288) //512k

void setup_stdio_drv(const char* filename);
void close_stdio_drv(const char* filename);

int write_byte(uint8_t byte, unsigned int addr);
int write_byte_array(uint8_t *bytes, unsigned int len, unsigned int addr);
int read_byte(uint8_t *byte, unsigned int addr);
int read_byte_array(uint8_t *bytes, unsigned int len,  unsigned int addr);
void read_in(const char* filename);
void write_out(const char* filename);

#endif /* stdio-drv.h */
