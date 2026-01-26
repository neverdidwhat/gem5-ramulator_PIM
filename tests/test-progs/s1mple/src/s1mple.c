#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE (64 * 1024 * 1024)  // 512 MB，自己改这里
char src[ARRAY_SIZE];
char dst[ARRAY_SIZE];
int main(void)
{
    memset(src, 0, ARRAY_SIZE);
    memset(dst, 0, ARRAY_SIZE);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    memcpy(dst, src, ARRAY_SIZE);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double sec = (t1.tv_sec - t0.tv_sec)
               + (t1.tv_nsec - t0.tv_nsec) * 1e-9;

    printf("Size: %d MB\n", ARRAY_SIZE / (1024 * 1024));
    printf("Time: %.6f s\n", sec);
    printf("Bandwidth: %.2f MB/s\n",
           (ARRAY_SIZE / (1024.0 * 1024.0)) / sec);

    return 0;
}
