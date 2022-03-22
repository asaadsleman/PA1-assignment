//
// Created by edent on 22/03/2022.
//
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int parity(uint32_t v)
{ //implementation from https://gist.github.com/qsxcv/b2f9976763d52bf1e7fc255f52f05f5b#file-secded_31_26-c-L10
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}

uint32_t computeParityBits(uint32_t h) {
    if(parity(h & 0b10101010101010101010101010101010)) { h|=1<<1;}
    if(parity(h & 0b11001100110011001100110011001100)) { h|=1<<2;}
    if(parity(h & 0b11110000111100001111000011110000)) { h|=1<<4;}
    if(parity(h & 0b11111111000000001111111100000000)) { h|=1<<8;}
    if(parity(h & 0b11111111111111110000000000000000)) { h|=1<<16;}
    return h;
}


uint32_t encode(uint32_t d)
{
    uint32_t first =d&1;
    uint32_t second =d&0b1110;
    uint32_t third =d&0b11111110000;
    uint32_t forth =d&0b11111111111111100000000000;
    uint32_t h =forth<<6|third<<5|second<<4|first<<3; //padding with zeros

    return computeParityBits(h);
}