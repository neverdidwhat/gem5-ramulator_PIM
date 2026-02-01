#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
    printf("addr test\n");

    uintptr_t phys = 0x123;                 // 你要映射的“物理地址”
    size_t len = 1;                         // 你要访问的长度（字节）

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf");
        return 1;
    }

    uintptr_t page_base = phys & ~(uintptr_t)(page_size - 1);
    uintptr_t page_off  = phys - page_base;

    int fd = open("/dev/mem", O_RDONLY | O_SYNC); // 只读示例；要写就改 O_RDWR
    if (fd < 0) {
        perror("open(/dev/mem)");
        return 1;
    }

    void *map = mmap(NULL, page_off + len, PROT_READ, MAP_SHARED, fd, (off_t)page_base);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    void *virt = (char *)map + page_off;
    printf("phys addr: 0x%" PRIxPTR "\n", phys);
    printf("virt addr: %p\n", virt);

    // 示例：读 1 字节（仅当该物理地址确实可读且有意义）
    unsigned char val = *(volatile unsigned char *)virt;
    printf("val: 0x%02x\n", val);

    munmap(map, page_off + len);
    close(fd);
    return 0;
}
