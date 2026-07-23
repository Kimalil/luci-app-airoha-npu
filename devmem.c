#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>

#define FATAL do { exit(1); } while(0)
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main(int argc, char **argv) {
    int fd;
    void *map_base, *virt_addr; 
    uint32_t read_result, writeval;
    off_t target;
    int access_type = 'w';
    
    if(argc < 2) exit(1);
    
    target = strtoull(argv[1], 0, 0);

    if(argc > 2) {
        if (argv[2][0] == '3' && argv[2][1] == '2') access_type = 'w';
        else if (argv[2][0] == '1' && argv[2][1] == '6') access_type = 'h';
        else if (argv[2][0] == '8') access_type = 'b';
        else access_type = tolower(argv[2][0]);
    }

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;
    
    virt_addr = (void *)((uintptr_t)map_base + (target & MAP_MASK));
    
    if(argc > 3) {
        writeval = strtoul(argv[3], 0, 0);
        switch(access_type) {
            case 'b': *((volatile uint8_t *) virt_addr) = writeval; break;
            case 'h': *((volatile uint16_t *) virt_addr) = writeval; break;
            case 'w': *((volatile uint32_t *) virt_addr) = writeval; break;
            default: exit(1);
        }
    } else {
        switch(access_type) {
            case 'b': read_result = *((volatile uint8_t *) virt_addr); break;
            case 'h': read_result = *((volatile uint16_t *) virt_addr); break;
            case 'w': read_result = *((volatile uint32_t *) virt_addr); break;
            default: exit(1);
        }
        if (access_type == 'b') printf("0x%02X\n", read_result);
        else if (access_type == 'h') printf("0x%04X\n", read_result);
        else printf("0x%08X\n", read_result);
    }
    
    munmap(map_base, MAP_SIZE);
    close(fd);
    return 0;
}
