#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

struct Process
{
    char name[64];
    pid_t pid;
    double execution_time;
    double wait_time;
    int arrivalTime;
    int burstTime;
    int priority;
};

// Define a node structure for the linked list
struct Node
{
    struct Process data;
    struct Node *next;
};

// Define the linked list structure
struct LinkedList
{
    struct Node *head;
    int size;
};

// Function to initialize an empty linked list
struct LinkedList *createLinkedList()
{
    struct LinkedList *list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    list->head = NULL;
    list->size = 0;
    return list;
}

// Function to check if a linked list is empty
bool isEmpty(struct LinkedList *list)
{
    return list->size == 0;
}

// Function to add a process to the linked list
void push(struct LinkedList *list, struct Process data)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

// Function to remove a process from the linked list
void pop(struct LinkedList *list)
{
    if (!isEmpty(list))
    {
        struct Node *temp = list->head;
        list->head = list->head->next;
        free(temp);
        list->size--;
    }
}

// Code to help handle and generate signals
int pause_process(pid_t pid)
{
    if (kill(pid, SIGSTOP) == 0)
    {
        printf("Process with PID %d paused.\n", (int)pid);
        return 0;
    }
    else
    {
        perror("Error!! could not pause process");
        return -1;
    }
}

int resume_process(pid_t pid)
{
    if ((kill(pid, SIGCONT) == 0))
    {
        printf("\nProcess with PID %d resumed\n", (int)pid);
        return 0;
    }
    else
    {
        perror("Could not continue execution of process ");
        return -1;
    }
}
int main()
{
    struct LinkedList *ready_queue = createLinkedList();   // Initializing Ready Queue
    struct LinkedList *running_queue = createLinkedList(); // Initializing Running Queue
    int TSLICE;
    int NCPU;

    return 0;
}
