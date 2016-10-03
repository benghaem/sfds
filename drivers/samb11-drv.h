#include <stdint.h>
#include "../sfds.h"

#ifndef _SAMB11_DRV_
#define _SAMB11_DRV_

#define BLOCKS_TO_BUFFER (2)

void setup_samb11_drv(uint8_t spi_target);
void close_samb11_drv();

int write_byte(uint8_t byte, unsigned int addr);
int write_byte_array(uint8_t *bytes, unsigned int len, unsigned int addr);
int read_byte(uint8_t *byte, unsigned int addr);
int read_byte_array(uint8_t *bytes, unsigned int len,  unsigned int addr);

void read_in();
void write_out();

#endif /* samb11-drv.h */
