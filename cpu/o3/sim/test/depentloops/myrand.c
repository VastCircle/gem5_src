#include <stdio.h>
#include <stdint.h>

#define SIZE 100  // 生成 100 个随机数
#define MASK 0x7FF // 11位掩码 (2047)

uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x & MASK; // 限制在 11 位以内
}

int main() {
    uint32_t seed = 123456789; // 选择一个非零种子
    uint32_t rand_array[SIZE];

    // 生成随机数
    for (int i = 0; i < SIZE; i++) {
        rand_array[i] = xorshift32(&seed);
    }

    // 以 a[] = {val1, val2, ...}; 格式输出
    printf("uint32_t a[%d] = {", SIZE);
    for (int i = 0; i < SIZE; i++) {
        printf("%u", rand_array[i]);
        if (i < SIZE - 1) {
            printf(", ");
        }
    }
    printf("};\n");

    return 0;
}

