#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE (32 * 1024 * 1024)  // 自己改这里

char *src;
volatile uint64_t sink;

int main(void)
{
    src = mmap(NULL, ARRAY_SIZE,
               PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS,
               -1, 0);

    // memset(src, 1, ARRAY_SIZE);

    uint64_t *p = (uint64_t *)src;
    size_t n = ARRAY_SIZE / sizeof(uint64_t);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    uint64_t s0=0,s1=0,s2=0,s3=0,s4=0,s5=0,s6=0,s7=0;
    for (size_t i = 0; i + 8*8 <= n; i += 8) {
        s0 += p[i + 0];
        s1 += p[i + 1];
        s2 += p[i + 2];
        s3 += p[i + 3];
        s4 += p[i + 4];
        s5 += p[i + 5];
        s6 += p[i + 6];
        s7 += p[i + 7];
    }

    sink = s0+s1+s2+s3+s4+s5+s6+s7;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double sec = (double)(t1.tv_sec - t0.tv_sec)
               + (double)(t1.tv_nsec - t0.tv_nsec) * 1e-9;

    printf("t0.tv_sec=%ld, t0.tv_nsec=%ld\n", t0.tv_sec, t0.tv_nsec);
    printf("t1.tv_sec=%ld, t1.tv_nsec=%ld\n", t1.tv_sec, t1.tv_nsec);
    printf("Size: %d MB\n", (int)(ARRAY_SIZE / (1024 * 1024)));
    printf("Time: %.6f s\n", sec);
    printf("Bandwidth: %.2f MB/s\n",
           (ARRAY_SIZE / (1024.0 * 1024.0)) / sec);

    return 0;
}
