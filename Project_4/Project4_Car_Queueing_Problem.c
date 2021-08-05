/*******************************************************************************
 * Author: Meng-Shiun, Tsai
 * Program: Car Queueing Problem
    1. Write a program to solve this problem with minimum time (optimization).
    2. Read the cars.txt and lane.txt and create the corresponding queue.
    PS. Bonus function is not provided here.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Deal with the list of car
typedef struct car_node {
    int enter_time, out_time, processing_lane_ID;
    char category[10];
    struct car_node *next;
}CAR_NODE;

typedef struct car_list {
    struct car_node *head, *tail;
}CAR_LIST;

// Deal with the list of lane
typedef struct lane_node {
    int lane_ID, processing_time, waiting_time;
    char category[10];
    struct car_node *head, *tail;
    struct lane_node *next;
}LANE_NODE;

typedef struct lane_list {
    int car_counter;
    struct lane_node *head, *tail;
}LANE_LIST;

// Deal with the list of car
CAR_LIST *create_car_list();
CAR_NODE *create_car_node(int, char*);
void car_enqueue_to_list(CAR_LIST*, CAR_NODE*);
CAR_LIST *read_cars_from_file();

// Deal with the list of lane
LANE_LIST *create_lane_list();
LANE_NODE *create_lane_node(int, int, char*);
void lane_enqueue_to_list(LANE_LIST*, LANE_NODE*);
LANE_NODE *lane_dequeue_from_list(LANE_LIST *lane_list, char *car_category);
LANE_LIST *read_lane_from_file();
void car_enqueue_to_lane(LANE_LIST*, LANE_NODE*, CAR_NODE*);
CAR_NODE *car_dequeue_from_lane(LANE_LIST*, LANE_NODE*);

// Deal with optimization of Car Scheduling System
LANE_LIST *update_and_sort_lane_list(LANE_LIST*);
void car_scheduling_system(LANE_LIST*, CAR_LIST*);

int main() {
    // Read car data and lane data from cars.txt and lane.txt
    CAR_LIST *car_list = read_cars_from_file();
    LANE_LIST *lane_list = read_lane_from_file();

    // Optimize the Car Scheduling System
    car_scheduling_system(lane_list, car_list);

    return 0;
}

CAR_LIST *create_car_list() {
    CAR_LIST *car_list = (CAR_LIST*)malloc(sizeof(CAR_LIST));
    car_list->head = NULL;
    car_list->tail = NULL;

    return car_list;
}

CAR_NODE *create_car_node(int enter_time, char *category) {
    CAR_NODE *car_node = (CAR_NODE*)malloc(sizeof(CAR_NODE));
    car_node->enter_time = enter_time;
    car_node->out_time = -1;            // Set to -1 by default (would be determined when enqueued to lane)
    car_node->processing_lane_ID = -1;  // Set to -1 by default (would be determined when enqueued to lane)
    strcpy(car_node->category, category);
    car_node->next = NULL;

    return car_node;
}

void car_enqueue_to_list(CAR_LIST *car_list, CAR_NODE *car_node) {
    if(car_list->head == NULL) {
        car_list->head = car_node;
        car_list->tail = car_node;
    } else {
        CAR_NODE *previous_car_node=NULL, *current_car_node=car_list->head;

        while(current_car_node != NULL) {
            // Assume that priority ... truck > car (at the same enter time, enqueue truck firstly)
            if(car_node->enter_time == current_car_node->enter_time && !strcmp(car_node->category, "truck")) {
                if(previous_car_node == NULL) {
                    // CASE ... Before head node
                    car_node->next = car_list->head;
                    car_list->head = car_node;
                } else {
                    // CASE ... Before other nodes except for head node
                    previous_car_node->next = car_node;
                    car_node->next = current_car_node;
                }
                return ;
            }
            previous_car_node = current_car_node;
            current_car_node = current_car_node->next;
        }
        // CASE ... After tail node
        previous_car_node->next = car_node;
        car_list->tail = car_node;
    }
}

CAR_LIST *read_cars_from_file() {
    int enter_time=0;
    char category[10];
    CAR_LIST *car_list = create_car_list();
    CAR_NODE *car_node = NULL;
    FILE *fptr_read = fopen("cars.txt", "r");

    // Read cars from cars.txt and Enqueue to car list
    while(fscanf(fptr_read, "%d %s", &enter_time, category) != EOF) {
        car_node = create_car_node(enter_time, category);
        car_enqueue_to_list(car_list, car_node);
    }
    fclose(fptr_read);

    return car_list;
}

LANE_LIST *create_lane_list() {
    LANE_LIST *lane_list = (LANE_LIST*)malloc(sizeof(LANE_LIST));
    lane_list->car_counter = 0;
    lane_list->head = NULL;
    lane_list->tail = NULL;

    return ;
}

LANE_NODE *create_lane_node(int lane_ID, int processing_time, char *category) {
    LANE_NODE *lane_node = (LANE_NODE*)malloc(sizeof(LANE_NODE));
    lane_node->lane_ID = lane_ID;
    lane_node->processing_time = processing_time;
    lane_node->waiting_time = processing_time; // Assume that waiting time is at least higher than processing time
    strcpy(lane_node->category, category);
    lane_node->next = NULL;
    lane_node->head = NULL;
    lane_node->tail = NULL;

    return lane_node;
}

void lane_enqueue_to_list(LANE_LIST *lane_list, LANE_NODE *lane_node) {
    if(lane_list->head == NULL) {
        lane_list->head = lane_node;
        lane_list->tail = lane_node;
    } else {
        LANE_NODE *previous_lane_node=NULL, *current_lane_node=lane_list->head;
        while(current_lane_node != NULL) {
            // Assume that priority ... waiting time (enqueue smaller one firstly) > category (enqueue car firstly) > processing time (enqueue smaller one firstly)
            if((lane_node->waiting_time < current_lane_node->waiting_time) ||
               (lane_node->waiting_time == current_lane_node->waiting_time && strcmp(lane_node->category, current_lane_node->category) && !strcmp(lane_node->category, "car")) ||
               (lane_node->waiting_time == current_lane_node->waiting_time && !strcmp(lane_node->category, current_lane_node->category) && lane_node->processing_time < current_lane_node->processing_time)) {
                if(previous_lane_node == NULL) {
                    // CASE ... Before head node
                    lane_node->next = lane_list->head;
                    lane_list->head = lane_node;
                } else {
                    // CASE ... Before other nodes except for head node
                    previous_lane_node->next = lane_node;
                    lane_node->next = current_lane_node;
                }
                return ;
            }
            previous_lane_node = current_lane_node;
            current_lane_node = current_lane_node->next;
        }
        // CASE ... After tail node
        previous_lane_node->next = lane_node;
        lane_list->tail = lane_node;
    }
}

LANE_NODE *lane_dequeue_from_list(LANE_LIST *lane_list, char *car_category) {
    LANE_NODE *previous_lane_node=NULL, *current_lane_node=lane_list->head;

    while(current_lane_node != NULL) {
        if((!strcmp(car_category, "truck") && !strcmp(current_lane_node->category, "truck")) || !strcmp(car_category, "car")) {
            if(previous_lane_node == NULL) {
                lane_list->head = lane_list->head->next;
            } else {
                previous_lane_node->next = current_lane_node->next;
            }
            current_lane_node->next = NULL; // Prevent the case of being enqueued to the last node in the next round
            return current_lane_node;
        }
        previous_lane_node = current_lane_node;
        current_lane_node = current_lane_node->next;
    }

    return previous_lane_node;
}

LANE_LIST *read_lane_from_file() {
    int processing_time=0;
    char lane_ID[10], category[15];
    LANE_LIST *lane_list = create_lane_list();
    LANE_NODE *lane_node = NULL;
    FILE *fptr_read = fopen("lane.txt", "r");

    // Read cars from cars.txt and Enqueue to car list
    while(fscanf(fptr_read, "%s %s %d", lane_ID, category, &processing_time) != EOF) {
        lane_node = create_lane_node(atoi(lane_ID+5), processing_time, strtok(category, "_"));
        lane_enqueue_to_list(lane_list, lane_node);
    }
    fclose(fptr_read);

    return lane_list;
}

void car_enqueue_to_lane(LANE_LIST *lane_list, LANE_NODE *lane_node, CAR_NODE *car_node) {
    // Copy the car node to another memory space
    CAR_NODE *copy_car_node = create_car_node(car_node->enter_time, car_node->category);
    copy_car_node->out_time = car_node->out_time;
    copy_car_node->processing_lane_ID = car_node->processing_lane_ID;

    // Enqueue copy_car_node to lane node
    if(lane_node->head == NULL) {
        lane_node->head = copy_car_node;
        lane_node->tail = copy_car_node;
    } else {
        lane_node->tail->next = copy_car_node;
        lane_node->tail = lane_node->tail->next;
    }

    // Update the information of lane node
    lane_node->waiting_time += lane_node->processing_time;
    (lane_list->car_counter)++;
}

CAR_NODE *car_dequeue_from_lane(LANE_LIST *lane_list, LANE_NODE *lane_node) {
    CAR_NODE *temp_car_node = lane_node->head;

    if(lane_node->head == lane_node->tail) {
        lane_node->head = NULL;
        lane_node->tail = NULL;
    } else {
        lane_node->head = lane_node->head->next;
    }
    (lane_list->car_counter)--;

    return temp_car_node;
}

void car_scheduling_system(LANE_LIST *lane_list, CAR_LIST *car_list) {
    int system_time=0;
    LANE_NODE *min_waiting_time_lane_node=NULL, *out_current_lane_node=NULL;
    CAR_NODE *enter_current_car_node=car_list->head, *out_current_car_node=NULL;
    FILE *fptr = fopen("output.txt", "w");

    while(enter_current_car_node != NULL || lane_list->car_counter != 0) {
        // CASE ... Enqueue car node to lane node (when system time is equal to the enter time of car node)
        while(enter_current_car_node != NULL && enter_current_car_node->enter_time == system_time) {
            // Dequeue lane node from lane list according to the category of car node
            min_waiting_time_lane_node = lane_dequeue_from_list(lane_list, enter_current_car_node->category);

            // Update the information of car node according to the lane node and Enqueue to the lane node
            enter_current_car_node->out_time = enter_current_car_node->enter_time + min_waiting_time_lane_node->waiting_time;
            enter_current_car_node->processing_lane_ID = min_waiting_time_lane_node->lane_ID;
            car_enqueue_to_lane(lane_list, min_waiting_time_lane_node, enter_current_car_node);

            // Enqueue lane node to lane list again
            lane_enqueue_to_list(lane_list, min_waiting_time_lane_node);

            // Go to next car node
            enter_current_car_node = enter_current_car_node->next;
        }

        // CASE ... Dequeue car node from lane node (when system time is equal to the out time of car node)
        out_current_lane_node = lane_list->head;
        while(out_current_lane_node != NULL) {
            out_current_car_node = out_current_lane_node->head;
            while(out_current_car_node != NULL && out_current_car_node->out_time == system_time) {
                car_dequeue_from_lane(lane_list, out_current_lane_node);
                printf("%d lane_%d %s\n", out_current_car_node->out_time, out_current_car_node->processing_lane_ID, out_current_car_node->category);
                fprintf(fptr, "%d lane_%d %s\n", out_current_car_node->out_time, out_current_car_node->processing_lane_ID, out_current_car_node->category);
                out_current_car_node = out_current_car_node->next;
            }
            out_current_lane_node = out_current_lane_node->next;
        }

        // Update waiting time of each lane node in lane list and Sort lane list based on waiting time
        lane_list = update_and_sort_lane_list(lane_list);
        system_time++;
    }

    fclose(fptr);
}

LANE_LIST *update_and_sort_lane_list(LANE_LIST *lane_list) {
    LANE_LIST *sorted_lane_list = create_lane_list();
    LANE_NODE *current_lane_node = NULL;

    // Copy car counter of lane list to sorted lane list
    sorted_lane_list->car_counter = lane_list->car_counter;

    while(lane_list->head != NULL) {
        // Update the information of lane node
        current_lane_node = lane_dequeue_from_list(lane_list, "car");
        if(current_lane_node->waiting_time > current_lane_node->processing_time) {
            (current_lane_node->waiting_time)--;
        }

        // Enqueue the new lane node into sorted lane list
        lane_enqueue_to_list(sorted_lane_list, current_lane_node);
    }

    return sorted_lane_list;
}
