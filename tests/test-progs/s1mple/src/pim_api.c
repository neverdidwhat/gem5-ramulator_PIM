#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>  // 修复：添加此头文件以解决 close() 警告
#include <stdlib.h>  // 修复：为了使用 strtoul 将命令行参数转为数字

#define PIM_CTRL_BASE 0x2E010000 // 必须和 Python 脚本里的一致

void pim_gemv(unsigned long val) { 
    // 打开 /dev/mem 获取物理内存访问权限
    int fd = open("/dev/mem", O_RDWR | O_SYNC); 
    if (fd < 0) {
        perror("Error opening /dev/mem (Try sudo?)");
        return;
    }

    // 将物理地址映射到虚拟地址
    void *ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PIM_CTRL_BASE); 
    if (ptr == MAP_FAILED) {
        perror("Error mmap");
        close(fd);
        return;
    }

    // 执行非缓存写 (Uncached Write)
    // 使用 (void*) 转换确保偏移正确，触发 gem5 中的 0x10 偏移逻辑
    printf("LazyMan: Sending value %lu to PIM_CTRL at offset 0x10...\n", val);
    *((volatile unsigned char *)((unsigned char *)ptr + 0x0000)) = val; 
    
    // 释放资源
    munmap(ptr, 4096); 
    close(fd); 
}

// 修复：添加 main 函数，让程序可以运行
int main(int argc, char *argv[]) {
    unsigned long test_val = 1; // 默认值

    // 如果运行程序时跟了参数，如 ./pim_test 1234
    if (argc > 1) {
        test_val = strtoul(argv[1], NULL, 10);
    }

    printf("--- PIM Controller Test Start ---\n");
    
    pim_gemv(test_val);
    pim_gemv(test_val);
    pim_gemv(test_val);
    pim_gemv(test_val);
    pim_gemv(test_val);

    printf("--- PIM Controller Test End ---\n");
    return 0;
}