/*
 * sfds.h
 *
 * Created: 8/12/2016 13:45:05
 *  Author: Ben
 */


#ifndef SDS_H_
#define SDS_H_

#include <stdint.h>
#include <stdio.h>

#ifndef USE_SD_DRV
#include "drivers/stdio-drv.h"
#else
#include "drivers/sd-drv.h"
#endif

#define DATA_BLOCK_COUNT (2)

struct sfds_data_block
{
    uint8_t bytes[506];
    uint16_t items;
    uint16_t cursor;
};

struct sfds_key_block
{
    uint32_t checksums[DATA_BLOCK_COUNT];
    uint32_t next_kb;

    uint16_t session_id;
    uint16_t sample_count;
};

struct sfds_state
{
    uint32_t cursor_addr;
    uint8_t data_item_byte_count;
    uint16_t session_id;

    struct sfds_key_block key_block;

    struct sfds_data_block data_blocks[DATA_BLOCK_COUNT];
    uint8_t data_block_cursor;
    uint8_t data_block_max_items;

    uint8_t ready_for_flush;
};

int sfds_init_ds(struct sfds_state* sfds);
int sfds_reset_ds();
int sfds_open_session(struct sfds_state* sfds, uint8_t data_item_byte_count);
int sfds_close_session();
int sfds_add_data(struct sfds_state* sfds, uint8_t* data, int byte_count);

uint32_t calculate_crc32(struct sfds_data_block* block);
int sfds_flush(uint8_t* raw, int size, struct sfds_state* sfds);

#endif /* SDS_H_ */
