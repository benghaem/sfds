/*
 * sfds.c
 * simple data storage
 *
 */
#include "sfds.h"
#include "crc32.h"
#include <string.h>
#include <stdlib.h>


int sfds_init_ds(struct sfds_state* sfds){
    uint8_t block_start[3] = {0};
    uint8_t kb_pointer[4] = {0};
    uint8_t session_id[2] = {0};
    int hit_end = 0;
    uint32_t cursor = 0;

    /* Check for 'BKB' starting bytes. If we find them follow pointer to next kb */

    do{
        read_byte_array(block_start, 3, cursor);
        if (block_start[0] == 'B' && block_start[1] == 'K' && block_start[2] == 'B'){
            /* BKB, 32b pointer, session_id */
            /* read info here before updating the cursor */
            read_byte_array(kb_pointer, 4, cursor+3);
            read_byte_array(session_id, 2, cursor+3+sizeof(uint32_t));
            cursor = (kb_pointer[3] << 24 | kb_pointer[2] << 16 | kb_pointer[1] << 8 | kb_pointer[0]);
        } else {
            hit_end = 1;
        }
    } while (!hit_end);

    sfds->cursor_addr = cursor;
    sfds->session_id = session_id[1] << 8 | session_id[0];
    sfds->session_ready = 0;
    sfds->data_block_cursor = 0;
    return 1;
}


int sfds_open_session(struct sfds_state* sfds, uint8_t data_item_byte_count){
    /* if there is already a session return an error */
    if (sfds->session_ready){
        return -1;
    }
    /* update session id */
    sfds->session_id++;

    /* update key_block to match the correct writing location and session*/
    sfds->key_block.session_id = sfds->session_id;
    sfds->key_block.next_kb = sfds->cursor_addr;

    /* calculate the data block characteristics for the session */
    sfds->data_item_byte_count = data_item_byte_count;
    sfds->data_block_max_items = 506 / data_item_byte_count;

    sfds->session_ready = 1;

    return sfds->session_id;
}

int sfds_close_session(struct sfds_state* sfds){
    /* if there is not a session ready we have nothing to close */
    if (!sfds->session_ready){
         return -1;
    }

    //update the checksum for the last block
    struct sfds_data_block* last_block = &sfds->data_blocks[sfds->data_block_cursor];

    /* only continue with the last data block if it is incomplete */
    if (last_block->items > 0){
        sfds->key_block.checksums[sfds->data_block_cursor] = calculate_crc32(last_block);
        sfds->key_block.sample_count += last_block->items;
        sfds->data_block_cursor += 1;
    }

    /* data block cursor is 0 indexed but incremented after blocks complete so
     * only add one so that we have currently completed blocks plus the key block*/
    sfds->key_block.next_kb += (sfds->data_block_cursor + 1) * 512;
    sfds->ready_for_flush = 1;

    sfds_write(sfds);

    sfds->session_ready = 0;

    return 1;
};

int sfds_add_data(struct sfds_state* sfds, uint8_t* data, int size){
    if ( sfds->data_item_byte_count != size || sfds->ready_for_flush || !sfds->session_ready ){
        return -1;
    }

    struct sfds_data_block* target_block = &sfds->data_blocks[sfds->data_block_cursor];

    /* add a new item to the active data block*/
    memcpy(target_block->bytes+target_block->cursor, data, size);
    target_block->cursor += size;
    target_block->items += 1;

    /* if we have filled up the space update the data and check if we need to flush */
    if (target_block->items == sfds->data_block_max_items){
        /* Update checksums and sample count within keyblock */
        sfds->key_block.checksums[sfds->data_block_cursor] = calculate_crc32(target_block);
        sfds->key_block.sample_count += target_block->items;
        sfds->data_block_cursor += 1;

        /* we have used up all the allowed data blocks per key block prep for flush */
        if (sfds->data_block_cursor == DATA_BLOCK_COUNT){
            /* Update pointer */
            sfds->key_block.next_kb += (DATA_BLOCK_COUNT + 1) * 512;
            sfds->ready_for_flush = 1;
        }
    }

    return 1;
}

/*
 * sfds_flush: clears the buffers and serializes the data structures for writes
 * Returns: 0 on error
 *          address offset on success
 */
int sfds_flush(uint8_t* raw, int size, struct sfds_state* sfds){
    if (!sfds->ready_for_flush){
        return 0;
    }
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
    cursor += sizeof(uint32_t);

    // session id
    memcpy(k_block+cursor, &sfds->key_block.session_id, sizeof(uint16_t));
    cursor += sizeof(uint16_t);

    // data block count
    memcpy(k_block+cursor, &sfds->data_block_cursor, sizeof(uint16_t));
    cursor += sizeof(uint16_t);

    // sample count over next n db
    memcpy(k_block+cursor, &sfds->key_block.sample_count, sizeof(uint16_t));
    cursor += sizeof(uint16_t);

    //copy in the defined checksums
    int checksum_len = sizeof(uint32_t) * sfds->data_block_cursor;
    memcpy(k_block+cursor, sfds->key_block.checksums, checksum_len);
    cursor += checksum_len;

    k_block[cursor] = (uint8_t)'E';
    k_block[cursor + 1] = (uint8_t)'K';
    k_block[cursor + 2] = (uint8_t)'B';

    //for each data block
    for (int i = 0; i < sfds->data_block_cursor; i++){
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

    /* We have successfully moved the data out */

    sfds->ready_for_flush = 0;
    /* Clear data blocks */
    for (int i = DATA_BLOCK_COUNT - 1; i >= 0; i--){
        memset(sfds->data_blocks[i].bytes, 0, 506);
        sfds->data_blocks[i].items = 0;
        sfds->data_blocks[i].cursor = 0;
    }

    /* Clear key block. We retain the next_kb and session id values*/
    memset(sfds->key_block.checksums, 0, DATA_BLOCK_COUNT * sizeof(uint32_t));
    sfds->key_block.sample_count = 0;

    /* Reset the block cursor */
    sfds->data_block_cursor = 0;

    return (DATA_BLOCK_COUNT + 1) * 512;
}

int sfds_write(struct sfds_state* sfds){
    uint8_t buffer[(DATA_BLOCK_COUNT + 1) * 512];
    uint32_t cursor = sfds->key_block.next_kb;
    if (0 != sfds_flush(buffer, (DATA_BLOCK_COUNT+1)*512, sfds)){
        int ret = write_byte_array(buffer, (DATA_BLOCK_COUNT + 1) * 512, sfds->cursor_addr);
        sfds->cursor_addr = cursor;
        return ret;
    }
    return 0;
}

uint32_t calculate_crc32(struct sfds_data_block* d){
    return crc32(0, d->bytes, 506);
}
