#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

struct Process
{
    pid_t pid;
    char name[64];
    double execution_time;
    double wait_time;
    struct Process *next;
};

struct Queue
{
    struct Process *front;
    struct Process *rear;
};

void initializeQueue(struct Queue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}

void enqueue(struct Queue *queue, struct Process *process)
{
    if (queue->rear == NULL)
    {
        queue->front = queue->rear = process;
    }
    else
    {
        queue->rear->next = process;
        queue->rear = process;
    }
    process->next = NULL;
}

struct Process *dequeue(struct Queue *queue)
{
    if (queue->front == NULL)
        return NULL;

    struct Process *temp = queue->front;
    queue->front = queue->front->next;

    if (queue->front == NULL)
        queue->rear = NULL;

    return temp;
}

void moveSecondToFront(struct Queue *runningQueue)
{
    if (runningQueue->front != NULL && runningQueue->front->next != NULL)
    {
        struct Process *secondProcess = runningQueue->front->next;
        runningQueue->front->next = secondProcess->next;
        secondProcess->next = runningQueue->front;
        runningQueue->front = secondProcess;
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

// Function to execute processes using round-robin scheduling
void roundRobinScheduler(struct Queue *runningQueue, struct Queue *readyQueue, int tslice)
{
    struct Process *currentProcess = NULL;
    while (runningQueue->front || readyQueue->front)
    {
        if (currentProcess == NULL && runningQueue->front)
        {
            currentProcess = dequeue(runningQueue);
        }

        if (currentProcess)
        {
            if (currentProcess->execution_time < tslice)
            {
                usleep((int)(currentProcess->execution_time * 1000000));
                currentProcess->execution_time = 0.0;
                printf("Process %s completed.\n", currentProcess->name);
            }
            else
            {
                usleep(tslice * 1000);
                currentProcess->execution_time -= tslice;
                printf("Process %s ran for %.2f seconds.\n", currentProcess->name, tslice);
            }

            // Move the current process to the end of the running queue
            enqueue(runningQueue, currentProcess);
            currentProcess = NULL;
        }

        // Check for new arrivals from the ready queue
        while (readyQueue->front)
        {
            struct Process *process = dequeue(readyQueue);
            enqueue(runningQueue, process);
            printf("Process %s added to running queue.\n", process->name);
        }
    }
}

int main()
{
    struct Queue runningQueue;
    struct Queue readyQueue;
    initializeQueue(&runningQueue);
    initializeQueue(&readyQueue);

    while (1)
    {
    }

    // Run the round-robin scheduler with a time slice of 2 seconds
    roundRobinScheduler(&runningQueue, &readyQueue, 2);

    return 0;
}
