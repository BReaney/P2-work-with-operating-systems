#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>


//helper function to return the priority of the train. 
int priority(char c){
    if(isupper(c) >0){
        return 1;
    } 
    return 0;
}

typedef struct Train{
    int priority;
    int loading;
    int crossing;
}Train;

typedef struct Queue_Node{
    Train* next;
    Train* prev;
    Train cur;
}Node;

int main(int argc, char*argv[]){



FILE* input = fopen("input.txt", "r");

char cur_P;
int cur_L;
int cur_T;
int notice = 0;
//start a queue for each station
Node headEBH;
Node headEBL;
Node headWBH;
Node headWBL;


    if(input == NULL){
        printf("Input File was unable to be opened.");
        return 0;
    }

while(notice == 0){
    if(fscanf(input, "%s %d %d", &cur_P,&cur_L,&cur_T) != 3){
        notice = 1;
        break;
    }
    cur_P = priority(cur_P);
    printf("%d, %d, %d \n", cur_P, cur_L, cur_T);

}

fclose(input);





















    return 0;

}
