#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
struct Process
{
    pid_t pid;
    int arrivalTime;
    int burstTime;
    int burstTimeRemaining;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    bool isComplete;
    bool inQueue;
    struct Process *next;
};

struct Queue
{
    struct Process *front;
    struct Process *rear;
};

// Function to create a new process with user input.
struct Process *createProcess(int pid, int arrivalTime, int burstTime)
{
    struct Process *process = (struct Process *)malloc(sizeof(struct Process));
    process->pid = pid;
    process->arrivalTime = arrivalTime;
    process->burstTime = burstTime;
    process->burstTimeRemaining = burstTime;
    process->completionTime = 0;
    process->turnaroundTime = 0;
    process->waitingTime = 0;
    process->isComplete = false;
    process->inQueue = false;
    process->next = NULL;
    return process;
}

// Function to add a process to the scheduler.
void addProcess(struct Queue *readyQueue, struct Process *process)
{
    if (readyQueue->rear == NULL)
    {
        readyQueue->front = readyQueue->rear = process;
    }
    else
    {
        readyQueue->rear->next = process;
        readyQueue->rear = process;
    }
}

void enqueue(struct Queue *queue, struct Process *process)
{
    if (queue->rear == NULL)
    {
        queue->front = queue->rear = process;
        return;
    }

    queue->rear->next = process;
    queue->rear = process;
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

void checkForNewArrivals(struct Process processes[], const int n, const int currentTime, struct Queue *readyQueue)
{
    for (int i = 0; i < n; i++)
    {
        struct Process *p = &processes[i];
        if (p->arrivalTime <= currentTime && !p->inQueue && !p->isComplete)
        {
            p->inQueue = true;
            enqueue(readyQueue, p);
        }
    }
}

bool allProcessesCompleted(struct Process processes[], const int n)
{
    for (int i = 0; i < n; i++)
    {
        if (!processes[i].isComplete)
        {
            return false;
        }
    }
    return true;
}

void updateQueue(struct Process processes[], const int n, const int quantum, struct Queue *readyQueue, int *currentTime, int *programsExecuted)
{
    struct Process *p = dequeue(readyQueue);
    if (p == NULL)
    {
        if (allProcessesCompleted(processes, n))
        {
            return;
        }
        else
        {
            (*currentTime)++;
            checkForNewArrivals(processes, n, (*currentTime), readyQueue);
        }
    }
    else
    {
        printf("Running process: Process%d\n", p->pid);
        if (p->burstTimeRemaining <= quantum)
        {
            p->isComplete = true;
            (*currentTime) += p->burstTimeRemaining;
            p->completionTime = (*currentTime);
            p->waitingTime = p->completionTime - p->arrivalTime - p->burstTime;
            p->turnaroundTime = p->waitingTime + p->burstTime;

            if (p->waitingTime < 0)
                p->waitingTime = 0;

            p->burstTimeRemaining = 0;
            (*programsExecuted)++;

            checkForNewArrivals(processes, n, (*currentTime), readyQueue);
        }
        else
        {
            p->burstTimeRemaining -= quantum;
            (*currentTime) += quantum;
            checkForNewArrivals(processes, n, (*currentTime), readyQueue);
            enqueue(readyQueue, p);
        }
    }
}

void output(struct Process processes[], const int n)
{
    double avgWaitingTime = 0;
    double avgTurnaroundTime = 0;

    for (int i = 0; i < n; i++)
    {
        printf("Process %d: Waiting Time: %d Turnaround Time: %d\n", processes[i].pid, processes[i].waitingTime, processes[i].turnaroundTime);
        avgWaitingTime += processes[i].waitingTime;
        avgTurnaroundTime += processes[i].turnaroundTime;
    }
    printf("Average Waiting Time: %.2f\n", avgWaitingTime / n);
    printf("Average Turnaround Time: %.2f\n", avgTurnaroundTime / n);
}

void roundRobin(struct Process processes[], int n, int quantum)
{
    struct Queue readyQueue = {NULL, NULL};
    enqueue(&readyQueue, &processes[0]);
    processes[0].inQueue = true;

    int currentTime = 0;
    int programsExecuted = 0;

    while (!allProcessesCompleted(processes, n))
    {
        updateQueue(processes, n, quantum, &readyQueue, &currentTime, &programsExecuted);
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
    int n, quantum;
    char input[64] = "";
    printf("Enter the number of processes: ");
    scanf("%d", &n);
    printf("Enter time quantum: ");
    scanf("%d", &quantum);

    struct Process processes[n + 1];
    int i = 0;
    while (input != "exit")
    {
        printf("Enter process name:");
        fgets(input, sizeof(input), stdin);
        printf("Enter arrival time and burst time of each process: %d", ++i);
        processes[i].burstTimeRemaining = processes[i].burstTime;
        processes[i].pid = i + 1;
        printf("\n");
    }
    // for (int i = 0; i < n; i++)
    // {
    //     printf("Enter arrival time and burst time of each process %d: ", i + 1);
    //     scanf("%d %d", &processes[i].arrivalTime, &processes[i].burstTime);
    //     processes[i].burstTimeRemaining = processes[i].burstTime;
    //     processes[i].pid = i + 1;
    //     printf("\n");
    // }
    roundRobin(processes, i, quantum);
    output(processes, n);
    return 0;
}
