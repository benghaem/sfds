#include "stdio-drv.h"
#include <stdio.h>
#include <string.h>

uint8_t buffer[STDIO_DRV_FILE_SIZE] = {0};

void setup_stdio_drv(const char* filename){
    read_in(filename);
}

void close_stdio_drv(const char* filename){
    write_out(filename);
}

int write_byte(uint8_t byte, unsigned int addr){
    if ( addr < STDIO_DRV_FILE_SIZE ){
        buffer[addr] = byte;
        return 1;
    }
    return 0;
}

int write_byte_array(uint8_t* bytes, unsigned int len, unsigned int addr){
    if ( addr + len < STDIO_DRV_FILE_SIZE ){
        memcpy(buffer + addr, bytes, len);
        return 1;
    }
    return 0;
}

int read_byte(uint8_t *byte, unsigned int addr){
    if (addr < STDIO_DRV_FILE_SIZE){
        *byte = buffer[addr];
        return 1;
    }
    return 0;
}

int read_byte_array(uint8_t *bytes, unsigned int len, unsigned int addr){
    if (addr + len < STDIO_DRV_FILE_SIZE){
        memcpy(bytes, buffer+addr, len);
        return 1;
    }
    return 0;
}

void read_in(const char* filename){
    FILE* fp = fopen(filename, "r");
    fread(buffer, sizeof(uint8_t), STDIO_DRV_FILE_SIZE, fp);
    fclose(fp);
}

void write_out(const char* filename){
    FILE* fp = fopen(filename, "w");
    fwrite(buffer, sizeof(uint8_t), STDIO_DRV_FILE_SIZE, fp);
    fclose(fp);
}
