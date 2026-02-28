#include <stdint.h>

// PIM_CTRL 外设基地址（必须与gem5 Python配置一致）
#define PIM_CTRL_BASE 0x2E010000

// ------------ 寄存器偏移定义（按8字节对齐，ARM64）------------
#define REG_WEIGHT_ADDR  0x2000  // 权重矩阵起始物理地址（8字节）
#define REG_RANK_NUM     0x2008  //使用的rank数目（1字节，预留7字节对齐）
#define REG_MATRIX_K     0x2010  // 矩阵维度 K（4字节，预留4字节对齐）
#define REG_MATRIX_N     0x2018  // 矩阵维度 N（4字节，预留4字节对齐）
#define REG_INPUT_PREC   0x2030  // 输入数据精度（如8/16/32，1字节）
#define REG_WEIGHT_PREC  0x2038  // 权重数据精度（字节）
#define REG_SCALE_PREC   0x2040  // Scale精度（1字节）
#define REG_CMD          0x2048  // 命令寄存器（写入1触发GEMV执行，1字节）
#define REG_INPUT_DATA   0x0000 // 输入向量数据写入起始偏移（批量写入INT8数据）

#define WRITE_WIDTH 8  // 每次写入数据宽度（字节）

// 映射内存大小（覆盖所有寄存器偏移，4KiB足够）
#define MAP_SIZE 0X4000

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// 精度类型枚举（增强可读性，避免魔法数字）
typedef enum {
    PREC_8BIT  = 8,
    PREC_16BIT = 16,
    PREC_32BIT = 32,
    PREC_64BIT = 64
} PimPrecision;

/**
 * @brief 向PIM_CTRL外设写入GEMV所有参数并触发执行
 * 
 * @param weight_addr 权重矩阵起始物理地址（64位）
 * @param k 矩阵维度K（如行数）
 * @param n 矩阵维度N（如列数）
 * @param input_prec 输入数据精度（PREC_8BIT/PREC_16BIT等）
 * @param weight_prec 权重数据精度
 * @param scale_prec scale精度
 * @return int 0=成功，-1=失败
 */
int pim_gemv(
    unsigned long long weight_addr,  // 64位物理地址
    unsigned int k,
    unsigned int n,
    PimPrecision input_prec,
    PimPrecision weight_prec,
    PimPrecision scale_prec
) {
    // 1. 参数校验（避免无效值）
    if (weight_addr == 0 || k == 0 || n == 0) {
        fprintf(stderr, "Error: weight_addr/k/n cannot be 0\n");
        return -1;
    }
    if (input_prec != PREC_8BIT && input_prec != PREC_16BIT && 
        input_prec != PREC_32BIT && input_prec != PREC_64BIT) {
        fprintf(stderr, "Error: Invalid input precision\n");
        return -1;
    }
    // 同理校验weight_prec/scale_prec
    if (weight_prec != PREC_8BIT && weight_prec != PREC_16BIT && 
        weight_prec != PREC_32BIT && weight_prec != PREC_64BIT) {
        fprintf(stderr, "Error: Invalid weight precision\n");
        return -1;
    }
    if (scale_prec != PREC_8BIT && scale_prec != PREC_16BIT && 
        scale_prec != PREC_32BIT && scale_prec != PREC_64BIT) {
        fprintf(stderr, "Error: Invalid scale precision\n");
        return -1;
    }

    // 2. 打开/dev/mem（获取物理内存访问权限）
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "Error opening /dev/mem: %s (Try sudo?)\n", strerror(errno));
        return -1;
    }

    // 3. 映射物理地址到虚拟地址
    void *ptr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PIM_CTRL_BASE);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "Error mmap: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    int input_write_num = input_prec * k / (8 * WRITE_WIDTH);  // 计算需要写入多少次（每次WRITE_WIDTH字节）

    // 4. 逐个写入参数（volatile禁止编译器优化，确保写入外设）
    volatile char *pim_reg_base = (volatile char *)ptr;
    printf("PIM: Mapped base address = %p\n", pim_reg_base);

    // 4.1 写入权重矩阵起始物理地址（8字节）
    *((volatile unsigned long long *)(pim_reg_base + REG_WEIGHT_ADDR)) = weight_addr;
    printf("PIM: Write weight addr = 0x%llx to offset 0x%x\n", weight_addr, REG_WEIGHT_ADDR);

    // 4.2 写入rank使用数量（4字节，强制转换为uint32_t保证长度）
    *((volatile uint8_t *)(pim_reg_base + REG_RANK_NUM)) = 1;
    printf("PIM: Write rank num = %u to offset 0x%x\n", 1, REG_RANK_NUM);

    // 4.2 写入矩阵维度K（4字节，强制转换为uint32_t保证长度）
    *((volatile uint32_t *)(pim_reg_base + REG_MATRIX_K)) = k;
    printf("PIM: Write matrix K = %u to offset 0x%x\n", k, REG_MATRIX_K);

    // 4.3 写入矩阵维度N（4字节）
    *((volatile uint32_t *)(pim_reg_base + REG_MATRIX_N)) = n;
    printf("PIM: Write matrix N = %u to offset 0x%x\n", n, REG_MATRIX_N);

    // 4.4 写入输入精度（1字节）
    *((volatile uint8_t *)(pim_reg_base + REG_INPUT_PREC)) = (uint8_t)input_prec;
    printf("PIM: Write input precision = %u bit to offset 0x%x\n", input_prec, REG_INPUT_PREC);

    // 4.5 写入权重精度（1字节）
    *((volatile uint8_t *)(pim_reg_base + REG_WEIGHT_PREC)) = (uint8_t)weight_prec;
    printf("PIM: Write weight precision = %u bit to offset 0x%x\n", weight_prec, REG_WEIGHT_PREC);

    // 4.6 写入scale精度（1字节）
    *((volatile uint8_t *)(pim_reg_base + REG_SCALE_PREC)) = (uint8_t)scale_prec;
    printf("PIM: Write scale precision = %u bit to offset 0x%x\n", scale_prec, REG_SCALE_PREC);

    //写入输入向量数据（按偏移批量写入INT8数据）
    for(int i = 0; i < input_write_num; i++){
        *((volatile uint64_t *)(pim_reg_base + REG_INPUT_DATA + i *8)) = (uint64_t)i;
        printf("PIM: Write input vector data[%d] = %lu to offset 0x%x\n", i, (uint64_t)i, REG_INPUT_DATA + i *8);
    }
    // 4.7 写入命令寄存器触发GEMV执行（写入1）
    *((volatile unsigned long long *)(pim_reg_base + REG_CMD)) = 1;
    printf("PIM: Trigger GEMV execution (cmd=1) to offset 0x%x\n", REG_CMD);
    

    // 5. 释放资源（必须成对调用，避免泄漏）
    munmap((void *)ptr, MAP_SIZE);
    close(fd);

    printf("PIM: All GEMV params write success!\n");
    return 0;
}

int main() {
    // 示例参数：权重地址0x80000000，K=128，N=64，输入8bit，权重16bit，scale 32bit
    int ret = pim_gemv(
        0x80000000,  // weight_addr
        1024,         // K
        1024,          // N
        PREC_8BIT,   // input_prec
        PREC_8BIT,  // weight_prec
        PREC_8BIT   // scale_prec
    );

    int ret1 = pim_gemv(
        0x80F00600,  // weight_addr
        512,         // K
        512,          // N
        PREC_8BIT,   // input_prec
        PREC_8BIT,  // weight_prec
        PREC_8BIT   // scale_prec
    );

    if (ret != 0) {
        fprintf(stderr, "PIM: GEMV params write failed!\n");
        return -1;
    }
    if (ret1 != 0) {
        fprintf(stderr, "PIM: GEMV params write failed for second call!\n");
        return -1;
    }
    return 0;
}