#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
//create the needed mutexes
pthread_mutex_t travel_check;

//helper function to return the priority of the train. 
int priority(char c){
    int pri = 0;
    
    if(isupper(c) >0){
        pri = 1;
    } 
   
    
    return pri;
}
//helper to get our direction from input.
void direction(char* c, char p){
   

    if(p == 'W' || p == 'w'){
        strcpy(c, "West");
    }else if(p == 'E' || p == 'e'){
        strcpy(c, "East");
    }

}

typedef struct Train{
    int Number;
    char Direction[5];
    int Priority;
    int Loading;
    int Crossing;
}Train;

typedef struct Queue_Node{
    struct Queue_Node* next;
    struct Queue_Node* prev;
    Train Train;
}Node;

//location variable will contain which station it is, ie W for WB high priority or e for EB low priority
typedef struct Queue{
    Node *front;
    Node *tail;
    char location;
}Queue;


//helper to make a new train
Train new_Train(Train T, int N, char* D, int P, int L, int C){

    T.Number = N;
    strcpy(T.Direction, D);
    T.Priority = P;
    T.Loading = L;
    T.Crossing = C;
    return T;
}


//helper to make a queue
Queue *createQueue(char location){
    Queue *newQ = (Queue*)malloc(sizeof(Queue));
    newQ->front = NULL;
    newQ->tail = NULL;
    newQ->location = location;
    return newQ;

}


//typical enqueue function
void enqueue(Queue* Q, Train t){
     
    //make the new node, allocate memory
    Node* newN = (Node*)malloc(sizeof(Node));
    
    //store the train
    newN->Train = t;
   
    newN->prev = Q->tail;
     
   
    //if the queue is empty, need to make the front and tail point to new node.
    if(Q->tail == NULL){
        Q->front = newN;
        Q->tail = newN;
        return;
    }
    //make the current item at the back of our queue point to our new node, and then point our tail pointer to the new node. 
    
    Q->tail->next = newN;
    
    Q->tail = newN;
}    


//checks if queue given is empty or not
int Empty(Queue *Q){
    if(Q->front == NULL){
        return 1;
    }
    return 0;
}


//typical dequeue function, removes front from queue and returns it
Train dequeue(Queue* Q){
    Train dequeued;
    // make an empty train to stop warning showing up on compiler
    dequeued = new_Train(dequeued, 0,0,0,0,0);

    
    
    
    if(Empty(Q)){
        printf("Queue is empty!");
        // will return an empty train if the queue is empty.
        return dequeued;
    }
    //Current front gets stored in Temp, then the head pointer is pointed at cur's successor
    Node *temp = Q->front;
    Q->front = Q->front->next;

    if(Q->front == NULL){
        Q->tail = NULL;
    }
    //Set our removed train to our return value
    dequeued = temp->Train;
    free(temp);
    //return the removed train.
    return dequeued;
    


}
    //helper for testing, making sure my queues are working
void print_queue(Queue *Q){
    Node *cur = Q->front;
    printf("This is station %c\n", Q->location);
    while(cur != NULL){
        printf("%d, %s, %d, %d, %d \n", cur->Train.Number, cur->Train.Direction, cur->Train.Priority, cur->Train.Loading, cur->Train.Crossing);
        cur = cur->next;
    }
}

void* cross_track(void* arg){
    pthread_mutex_lock(&travel_check);
    Train *cur = arg;
    printf("Train %2d is ON the main track going %4s\n", cur->Number, cur->Direction);
    usleep(cur->Crossing);
    printf("Train %2d is OFF the main track after going %4s\n", cur->Number, cur->Direction);
    pthread_mutex_lock(&travel_check);
    pthread_exit(NULL);
    return NULL;
}

void* load_train(void* arg){
    Queue *Q = arg;
    Train cur = Q->front->Train;





}

int main(int argc, char*argv[]){

//initialize travel mutex
pthread_mutex_init(&travel_check, NULL);

//make pthread variable for crossing.
pthread_t cross;

FILE* input = fopen("input.txt", "r");

int cur_N = 0;
char cur_D[5];
char cur_P;
int cur_L;
int cur_C;
Train cur_T;
int notice = 0;

//start a queue for each station. 
Queue *HEB = createQueue('E');
Queue *LEB = createQueue('e');
Queue *HWB = createQueue('W');
Queue *LWB = createQueue('w');



    if(input == NULL){
        printf("Input File was unable to be opened.");
        return 0;
    }

while(notice == 0){
    if(fscanf(input, "%s %d %d", &cur_P,&cur_L,&cur_C) != 3){
        notice = 1;
        break;
    }
   
    direction(cur_D, cur_P);
    cur_P = priority(cur_P);

    
    cur_T = new_Train(cur_T,cur_N,cur_D,cur_P,cur_L,cur_C);
    cur_N++;
     
    if(cur_D[0] == 'E'){
        if(cur_P == 1){

            enqueue(HEB,cur_T);
        }else if(cur_P == 0){
            enqueue(LEB,cur_T);
        }
        
        
    }else if(cur_D[0] == 'W'){
         if(cur_P == 1){
           
            enqueue(HWB,cur_T);
            
        }else if(cur_P == 0){
            enqueue(LWB,cur_T);
        }
    }

}

fclose(input);

print_queue(HEB);
print_queue(LEB);
print_queue(HWB);
print_queue(LWB);





















    return 0;

}
