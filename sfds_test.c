#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfds.h"
#include "drivers/stdio-drv.h"

#define DATA_ITEM_BYTES (uint8_t)(128)

void manual_write_out(FILE* fp, struct sfds_state* sfds){
    char k_begin[3] = {"BKB"};
    char k_end[3] = {"EKB"};
    fwrite(k_begin, sizeof(char), 3, fp);
    fwrite(k_end, sizeof(char), 3, fp);

    char d_begin[3] = {"BDB"};
    char d_end[3] = {"EDB"};

    for (int i = 0; i < sfds->data_block_max_items; i++){
        fwrite(d_begin, sizeof(char), 3, fp);
        fwrite(sfds->data_blocks[0].bytes, sizeof(uint8_t), 506, fp);
        fwrite(d_end, sizeof(char), 3, fp);
    }
}


int main(int argc, char** argv){
    FILE* fp;
    struct sfds_state sfds = {0};
    uint8_t data_buffer[DATA_ITEM_BYTES] = {0};
    uint8_t data_raw[512 * (DATA_BLOCK_COUNT + 1)] = {0};

    if (argc > 1){
        printf("opening: %s", argv[1]);
        fp = fopen(argv[1], "a");
        sfds_init_ds(&sfds);
        sfds_open_session(&sfds, DATA_ITEM_BYTES);

        while (!sfds.ready_for_flush){
            printf("%i > ", sfds.data_block_cursor);
            fgets((char*)data_buffer, DATA_ITEM_BYTES, stdin);
            if(!sfds_add_data(&sfds, data_buffer, DATA_ITEM_BYTES)){
                printf("sizing error");
            };
            memset(data_buffer, 0, sizeof(data_buffer));
            if (!sfds.ready_for_flush){
                printf("data block items: %i\n", sfds.data_blocks[sfds.data_block_cursor].items);
            }
        }

        printf("kb checksum 0: %X\n", sfds.key_block.checksums[0]);
        printf("kb checksum 1: %X\n", sfds.key_block.checksums[1]);
        printf("kb next kb: %X\n", sfds.key_block.next_kb);
        printf("db0 item count: %i\n", sfds.data_blocks[0].items);
        printf("db1 item count: %i\n", sfds.data_blocks[1].items);


        sfds_flush(data_raw, sizeof(data_raw), &sfds);
        fwrite(data_raw, sizeof(uint8_t), sizeof(data_raw), fp);
        fclose(fp);
    }
}
