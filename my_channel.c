//
// Created by edent on 22/03/2022.
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int noise, prob, messagesInCycle,bitToFlip;
int currentMessageInCycle=0;
int flipped=0;


void setNoiseType(int noiseCMD, int probCMD, int seedCMD, int spanCMD) {
    noise = noiseCMD;
    prob = probCMD / (2 ^ -16);
    srand(seedCMD);
    if(noiseCMD==0){ //deterministic
        messagesInCycle=(int)floorf(spanCMD/31.0)+1;
        bitToFlip=spanCMD%31;
    }

}

void analyzeArguments(int argc, char**argv) {
    char* end;
    if(argc==3){
        if(strcmp(argv[1], "-d")==0){
            setNoiseType(0,0,0,strtol(argv[2], & end, 10));
        }
    }
    else if(argc==4){
        if(strcmp(argv[1], "-r")==0){
            setNoiseType(1,strtol(argv[2], &end, 10),
                         strtol(argv[3],&end,10),0);
        }
    }
    else{
        printf("Invalid number of arguments.\n");
    }
}
int getRandom(){ //returns 1 with probability of n/2^-16
    return rand()/RAND_MAX<=prob;
}

uint32_t noiseMessage(uint32_t message){
    int i;
    if(noise){//random
        for(i=0; i<32; i++){
            if (getRandom()){
                message^=1<<i;
                flipped++;
            }
        }return message;

    }
    else{ //deterministic
        currentMessageInCycle++;
        if(currentMessageInCycle!=messagesInCycle){
            return message;
        }
        else{
            message^=1<<bitToFlip;
            flipped++;
            currentMessageInCycle=0;
            return message;
        }
    }

}


int main(int argc, char **argv){
    analyzeArguments(argc, argv);
    //read 32 bits and send to noiseMessage
}



