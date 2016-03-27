#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv){

    if(argc == 1){
        printf("Usage: %s length_of_key\n", argv[0]);
        exit(1);
    }

    char allowed_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    int keySize = 0;  
    int seed, idx;                  

    keySize = atoi(argv[1]);  

    seed = time(NULL);        
    srand(seed);              

    for(idx = 0; idx < keySize; idx++){
        printf("%c", allowed_chars[rand() % (int)strlen(allowed_chars)]);
    }

    printf("\n");

    return 0;
}