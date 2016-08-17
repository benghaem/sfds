/*
 * sfds.c
 * simple data storage
 *
 * Created: 8/12/2016 13:36:44
 *  Author: Ben
 */
#include "sfds.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

int sfds_init_ds(struct sfds_state* sfds){
    // should also check for previously existing data


    sfds->cursor_addr = 0;
    sfds->session_id = 0;
    sfds->data_block_cursor = 0;
    return 1;
}

int sfds_open_session(struct sfds_state* sfds, uint8_t data_item_byte_count){
    sfds->session_id++;
    sfds->key_block.session_id = sfds->session_id;
    sfds->data_item_byte_count = data_item_byte_count;
    sfds->data_block_max_items = 506 / data_item_byte_count;

    printf("Opening new session\n");
    printf("Session id: %i\n", sfds->session_id);
    printf("Data block max items: %i\n", sfds->data_block_max_items);

    return 1;
}

int sfds_add_data(struct sfds_state* sfds, uint8_t* data, int size){
    if (sfds->data_item_byte_count != size){
        return 0;
    }

    struct sfds_data_block* target_block = &sfds->data_blocks[sfds->data_block_cursor];

    if (target_block->items < sfds->data_block_max_items){
        memcpy(target_block->bytes+target_block->cursor, data, size);
        target_block->cursor += size;
        target_block->items += 1;
    }

    if (target_block->items == sfds->data_block_max_items){
        sfds->key_block.checksums[sfds->data_block_cursor] = calculate_crc32(target_block);
        sfds->key_block.sample_count += target_block->items;
        sfds->data_block_cursor += 1;

        if (sfds->data_block_cursor == DATA_BLOCK_COUNT){
            sfds->ready_for_write = 1;
        }
    }

    return 1;
}

void to_raw_bytes(uint8_t* raw, int size, struct sfds_state* sfds){
    uint8_t d_block[DATA_BLOCK_COUNT][512] = {0};
    uint8_t k_block[512] = {0};
    int cursor = 0;

    //begin key block
    k_block[0] = (uint8_t)'B';
    k_block[1] = (uint8_t)'K';
    k_block[2] = (uint8_t)'B';
    cursor = 3;

    // pointer to next kb
    memcpy(k_block+cursor, &sfds->key_block.next_kb, sizeof(uint32_t));
    printf("Key block_next kb: %X\n", sfds->key_block.next_kb);
    printf("Key block_next kb: %X%X%X%X\n", k_block[cursor], k_block[cursor+1], k_block[cursor+2], k_block[cursor+3]);
    cursor += sizeof(uint32_t);

    // session id
    memcpy(k_block+cursor, &sfds->key_block.session_id, sizeof(uint16_t));
    cursor += sizeof(uint16_t);

    // sample count over next 8 db
    memcpy(k_block+cursor, &sfds->key_block.sample_count, sizeof(uint16_t));
    cursor += sizeof(uint16_t);

    //copy in checksums
    int checksum_len = sizeof(uint32_t) * DATA_BLOCK_COUNT;
    memcpy(k_block+cursor, sfds->key_block.checksums, checksum_len);
    cursor += checksum_len;

    k_block[cursor] = (uint8_t)'E';
    k_block[cursor + 1] = (uint8_t)'K';
    k_block[cursor + 2] = (uint8_t)'B';



    //for each data block
    for (int i = 0; i < DATA_BLOCK_COUNT; i++){
        int block_byte_total = sfds->data_item_byte_count * sfds->data_blocks[i].items;

        //begin data block
        d_block[i][0] = (uint8_t)'B';
        d_block[i][1] = (uint8_t)'D';
        d_block[i][2] = (uint8_t)'B';

        // 3 for offset from start
        memcpy(d_block[i]+3, sfds->data_blocks[i].bytes, block_byte_total);

        d_block[i][block_byte_total] = (uint8_t)'E';
        d_block[i][block_byte_total + 1] = (uint8_t)'D';
        d_block[i][block_byte_total + 2] = (uint8_t)'B';
    }

    memcpy(raw, k_block, 512);
    memcpy(raw+512, d_block, DATA_BLOCK_COUNT * 512);
}

uint32_t calculate_crc32(struct sfds_data_block* d){
    return 0;
}
