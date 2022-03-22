//
// Created by edent on 22/03/2022.
//
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int flipped=0;
int parity(uint32_t v)
{
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}
uint32_t findErrorLocation(uint32_t h) {
    //computing the parity check according to the algorithm
    uint32_t i =
            parity(h & 0b10101010101010101010101010101010) |
            parity(h & 0b11001100110011001100110011001100) << 1 |
            parity(h & 0b11110000111100001111000011110000) << 2 |
            parity(h & 0b11111111000000001111111100000000) << 3 |
            parity(h & 0b11111111111111110000000000000000) << 4;
    return i;
}
uint32_t removeParityBits(uint32_t h){
    uint32_t result=((h >> 3) & 1) | //bit at index 3
    ((h >> 4) & 0b1110| //bits at indices 5-7
    ((h >> 5) & 0b11111110000) |//bits at indices 9-15
    ((h >> 6) & 0b11111111111111100000000000)); //bits at indices 17-31
    return result;
}

uint32_t decode(uint32_t h)
{
    uint32_t i = findErrorLocation(h);
    if(h%2==1){ //handling the case that bit in index 0 was flipped by channel,
        // this will not be detected by the hamming code algorithm because it ignores bit 0.
        h^=1;
        flipped++;
    }
    if (i != 0) {
        h ^= (1 << i);
        flipped++;
        }
    return removeParityBits(h);
}

