
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

//create the needed mutexes
pthread_mutex_t travel_check;
pthread_mutex_t output_lock;
pthread_mutex_t ready_lock;
pthread_cond_t ready_cond;
pthread_t dispatcher_thread;
//extra size just in case, only should use pthread 0-3 from this array(one for each)
pthread_t stations[10];
int ready_counter = 0;

int total_trains = 0;
int trains_crossed = 0;

FILE *output;
struct timespec start_time;
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
    int ReadyOrder;
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
    int size;
}Queue;

Queue *readyQueue;

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
    newQ->size = 0;
    return newQ;

}


//typical enqueue function
void enqueue(Queue* Q, Train t){
     
    //make the new node, allocate memory
    Node* newN = (Node*)malloc(sizeof(Node));
    
    //store the train
    newN->Train = t;
    newN->next = NULL;
    newN->prev = Q->tail;
     
   
    //if the queue is empty, need to make the front and tail point to new node.
    if(Q->tail == NULL){
        Q->front = newN;
        Q->tail = newN;
        Q->size++;
        return;
    }
    //make the current item at the back of our queue point to our new node, and then point our tail pointer to the new node. 
    
    Q->tail->next = newN;
    
    Q->tail = newN;
    Q->size++;
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
    dequeued = new_Train(dequeued, 0,"",0,0,0);
    
    
        
    
    if(Empty(Q)){
        printf("Queue is empty!");
        // will return an empty train if the queue is empty.
        return dequeued;
    }

    //Current front gets stored in Temp, then the head pointer is pointed at cur's successor
    Node *temp = Q->front;
    Q->front = Q->front->next;
    Q->size--;
    
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
    printf("This is station %c, with size %d\n", Q->location, Q->size);
    while(cur != NULL){
        printf("%d, %s, %d, %d, %d \n", cur->Train.Number, cur->Train.Direction, cur->Train.Priority, cur->Train.Loading, cur->Train.Crossing);
        cur = cur->next;
    }
}
// remove specific train from queue
void remove_from_queue(Queue *Q, Train t) {
    Node *cur = Q->front;

    while (cur != NULL) {
        if (cur->Train.Number == t.Number) {
            if (cur->prev){
                cur->prev->next = cur->next;
            }else{
                Q->front = cur->next;
            }
            if (cur->next){
                cur->next->prev = cur->prev;
            }else{
                Q->tail = cur->prev;
            }
            Q->size--;
            free(cur);
            return;
        }

        cur = cur->next;
    }
}

void print_time(FILE *out) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    double elapsed =
        (now.tv_sec - start_time.tv_sec) +
        (now.tv_nsec - start_time.tv_nsec) / 1000000000.0;

    int hours = elapsed / 3600;
    int minutes = ((int)elapsed % 3600) / 60;
    double seconds = elapsed - hours * 3600 - minutes * 60;

    fprintf(out, "%02d:%02d:%04.1f ", hours, minutes, seconds);
}
// helper for if the trains finished being loaded are done at the same time and direction
int earlier_same_direction(Train a, Train b) {
    if (a.Loading != b.Loading) {
        return a.Loading < b.Loading;
    }
    return a.Number < b.Number;
}

void* load_train(void* arg){
    Train *t = arg;

    usleep(t->Loading * 100000);

    pthread_mutex_lock(&ready_lock);

    t->ReadyOrder = ready_counter++;
    
    enqueue(readyQueue, *t);

    pthread_mutex_lock(&output_lock);
    print_time(output);
    fprintf(output, "Train %2d is ready to go %4s\n", t->Number, t->Direction);
    pthread_mutex_unlock(&output_lock);

    pthread_cond_signal(&ready_cond);
    pthread_mutex_unlock(&ready_lock);

    free(t);
    return NULL;
}
//
// each one of these threads acts as a station. It takes a queue of trains, and makes a thread for each member of the queue, starting the loading/travel process.
void* station(void* arg){
    Queue *Q = arg;
   
    pthread_t thread[100];
    int count = 0;
    //printf("we got to station creation!\n");
    while(Empty(Q) != 1){
        Train *t = malloc(sizeof(Train));
        *t = dequeue(Q);
        pthread_create(&thread[count], NULL, load_train,t);
        
        count++;
    }

    for(int i = 0; i < count; i++){
        pthread_join(thread[i], NULL);
    };
   
    
    
    pthread_exit(0);

}
int direction_value_train(Train t) {
    return t.Direction[0] == 'E' ? 1 : 0;  // East = 1, West = 0
}

int opposite_direction_exists(Queue *q, int direction) {
    Node *cur = q->front;

    while (cur != NULL) {
        if (direction_value_train(cur->Train) != direction) {
            return 1;
        }
        cur = cur->next;
    }

    return 0;
}

int dir_value(Train t) {
    return t.Direction[0] == 'E' ? 1 : 0;
}

int better_train(Train candidate, Train best, int last_direction, int first) {
    int cdir = direction_value_train(candidate);
    int bdir = direction_value_train(best);

    if (candidate.Priority != best.Priority) {
        return candidate.Priority > best.Priority;
    }

    if (cdir == bdir) {
        return earlier_same_direction(candidate, best);
    }

    if (first) {
        return cdir == 0;   // West first if no train has crossed yet
    }

    return cdir != last_direction;
}

Train pick_best_train(Queue *q, int last_direction, int first, int same_direction_count) {
    Node *cur = q->front;
    Train best = cur->Train;

    
    
    // From my understading, if two trains have gone the same direction back-to-back, and any opposite-direction train is waiting, choose from opposite direction. I think?
     
    if (!first && same_direction_count >= 2 && opposite_direction_exists(q, last_direction)) {
        while (cur != NULL) {
            Train t = cur->Train;

            if (direction_value_train(t) != last_direction) {
                best = t;
                break;
            }

            cur = cur->next;
        }

        cur = q->front;
        while (cur != NULL) {
            Train t = cur->Train;

            if (direction_value_train(t) != last_direction) {
                if (better_train(t, best, last_direction, first)) {
                    best = t;
                }
            }

            cur = cur->next;
        }

        return best;
    }

    cur = q->front;
    while (cur != NULL) {
        Train t = cur->Train;

        if (better_train(t, best, last_direction, first)) {
            best = t;
        }

        cur = cur->next;
    }

    return best;
}

//
void *dispatcher() {
    int last_direction = -1;
    int first = 1;
    int same_direction_count = 0;

    while (1) {
        pthread_mutex_lock(&ready_lock);

        while (Empty(readyQueue) && trains_crossed < total_trains) {
            pthread_cond_wait(&ready_cond, &ready_lock);
        }

        if (trains_crossed >= total_trains) {
            pthread_mutex_unlock(&ready_lock);
            break;
        }

        Train best = pick_best_train(
            readyQueue,
            last_direction,
            first,
            same_direction_count
        );

        remove_from_queue(readyQueue, best);

        pthread_mutex_unlock(&ready_lock);

        pthread_mutex_lock(&travel_check);

        pthread_mutex_lock(&output_lock);
        print_time(output);
        fprintf(output, "Train %2d is ON the main track going %4s\n",best.Number, best.Direction);
        
        pthread_mutex_unlock(&output_lock);

        usleep(best.Crossing * 100000);

        pthread_mutex_lock(&output_lock);
        print_time(output);
        fprintf(output, "Train %2d is OFF the main track after going %4s\n",best.Number, best.Direction);
        
        pthread_mutex_unlock(&output_lock);

        int current_direction = direction_value_train(best);

        if (first || current_direction != last_direction) {
            same_direction_count = 1;
        } else {
            same_direction_count++;
        }

        last_direction = current_direction;
        first = 0;

        pthread_mutex_lock(&ready_lock);
        trains_crossed++;
        pthread_mutex_unlock(&ready_lock);

        pthread_mutex_unlock(&travel_check);
    }

    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    pthread_mutex_init(&travel_check, NULL);
    pthread_mutex_init(&output_lock, NULL);
    pthread_mutex_init(&ready_lock, NULL);
    pthread_cond_init(&ready_cond, NULL);

    readyQueue = createQueue('R');

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("input");
        return 1;
    }

    output = fopen("output.txt", "w");
    if (output == NULL) {
        perror("output.txt");
        fclose(input);
        return 1;
    }

    Queue *HEB = createQueue('E');
    Queue *LEB = createQueue('e');
    Queue *HWB = createQueue('W');
    Queue *LWB = createQueue('w');

    int cur_N = 0;
    char cur_D[5];
    char input_dir;
    int cur_L;
    int cur_C;
    Train cur_T;

    while (fscanf(input, " %c %d %d", &input_dir, &cur_L, &cur_C) == 3) {
        direction(cur_D, input_dir);
        int cur_priority = priority(input_dir);

        cur_T = new_Train(cur_T, cur_N, cur_D, cur_priority, cur_L, cur_C);
        cur_N++;
        total_trains++;

        if (cur_D[0] == 'E') {
            if (cur_priority == 1) enqueue(HEB, cur_T);
            else enqueue(LEB, cur_T);
        } else {
            if (cur_priority == 1) enqueue(HWB, cur_T);
            else enqueue(LWB, cur_T);
        }
    }

    fclose(input);

    clock_gettime(CLOCK_REALTIME, &start_time);

    pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);
    pthread_create(&stations[0], NULL, station, HEB);
    pthread_create(&stations[1], NULL, station, LEB);
    pthread_create(&stations[2], NULL, station, HWB);
    pthread_create(&stations[3], NULL, station, LWB);

    for (int i = 0; i < 4; i++) {
        pthread_join(stations[i], NULL);
    }

    pthread_join(dispatcher_thread, NULL);

    fclose(output);

    pthread_mutex_destroy(&travel_check);
    pthread_mutex_destroy(&output_lock);
    pthread_mutex_destroy(&ready_lock);
    pthread_cond_destroy(&ready_cond);

    return 0;
}