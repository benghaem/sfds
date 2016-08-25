
#define DATA_ITEM_BYTES 128

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfds.h"
#include "drivers/stdio-drv.h"


int main(int argc, char** argv){
    struct sfds_state sfds = {0};
    uint8_t data_buffer[DATA_ITEM_BYTES] = {0};
    uint8_t data_raw[512 * (DATA_BLOCK_COUNT + 1)] = {0};
    int session_id = 0;

    if (argc > 1){
        printf("opening: %s\n", argv[1]);

        setup_stdio_drv(argv[1]);
        sfds_init_ds(&sfds);

        printf("last session id: %i\n", sfds.session_id);
        printf("cursor location: %i\n", sfds.cursor_addr);

        session_id = sfds_open_session(&sfds, DATA_ITEM_BYTES);
        printf("Opened new session: %i\n",session_id);
        printf("Items per data block: %i\n",sfds.data_block_max_items);

        /*
         * For testing purposes we add 9 data items
         * Data block count has been overridden to force only 2 data blocks per
         * key block. At an item byte sizing of 128 we can fit 3 items into each data block
         * thus we should utilize 2 key blocks to store all 9 items
         */

        for(int i = 0; i < 9; i++){

            printf("item #%i : block[%i] > ", i, sfds.data_block_cursor);

            /* reset the buffer and get data from stdin */
            memset(data_buffer, 0, sizeof(data_buffer));
            fgets((char*)data_buffer, DATA_ITEM_BYTES, stdin);

            if(!sfds_add_data(&sfds, data_buffer, DATA_ITEM_BYTES)){
                printf("sizing error\n");
            };

            if (!sfds.ready_for_flush){
                printf("data block items: %i\n", sfds.data_blocks[sfds.data_block_cursor].items);
            }
            else{
                printf("triggered write\n");
                printf("kb checksum 0: %X\n", sfds.key_block.checksums[0]);
                printf("kb checksum 1: %X\n", sfds.key_block.checksums[1]);
                printf("kb next kb: %X\n", sfds.key_block.next_kb);
                printf("db0 item count: %i\n", sfds.data_blocks[0].items);
                printf("db1 item count: %i\n", sfds.data_blocks[1].items);
                sfds_write(&sfds);
            }
        }

        sfds_close_session(&sfds);

        close_stdio_drv(argv[1]);
    }
}
