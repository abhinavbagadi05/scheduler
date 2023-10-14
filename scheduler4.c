#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_PROCESSES 10

struct P
{
    char name[64];
    pid_t pid;
    bool isPaused;
};

void pause_process(pid_t pid)
{
    if (kill(pid, SIGSTOP) == 0)
    {
        printf("Process with PID %d paused.\n", (int)pid);
    }
    else
    {
        perror("Error!! could not pause process");
    }
}

void resume_process(pid_t pid)
{
    if (kill(pid, SIGCONT) == 0)
    {
        printf("Process with PID %d resumed.\n", (int)pid);
    }
    else
    {
        perror("Could not continue execution of process");
    }
}

int main()
{
    int tslice;
    printf("Enter time slice (in ms): ");
    scanf("%d", &tslice);
    getchar(); // Consume the newline character.
    struct P processes[MAX_PROCESSES];
    int processCount = 0;
    char input[64];
    while (1)
    {
        printf("Enter process name (or 'exit' to quit): ");
        fgets(input, sizeof(input), stdin);
        // Remove the newline character from the input.
        if (input[strlen(input) - 1] == '\n')
        {
            input[strlen(input) - 1] = '\0';
        }
        if (strcmp(input, "exit") == 0)
        {
            break;
        }
        pid_t pid = fork();
        if (pid == 0) // Child process
        {
            execl("/bin/sh", "sh", "-c", input, (char *)NULL);
            perror("Error");
            exit(1);
        }
        else if (pid < 0)
        {
            perror("Fork failed");
        }
        else
        {
            struct P newProcess;
            strncpy(newProcess.name, input, sizeof(newProcess.name));
            newProcess.pid = pid;
            newProcess.isPaused = false;
            processes[processCount] = newProcess;
            processCount++;
            printf("Process '%s' with PID %d started.\n", input, (int)pid);
        }
    }
    int currentProcess = 0;
    while (processCount > 0)
    {
        if (!processes[currentProcess].isPaused)
        {
            pause_process(processes[currentProcess].pid);
            processes[currentProcess].isPaused = true;
            printf("Pausing process '%s' with PID %d.\n", processes[currentProcess].name, (int)processes[currentProcess].pid);
        }
        currentProcess = (currentProcess + 1) % processCount;
        usleep(tslice * 1000);
        resume_process(processes[currentProcess].pid);
        processes[currentProcess].isPaused = false;
        printf("Resuming process '%s' with PID %d.\n", processes[currentProcess].name, (int)processes[currentProcess].pid);
    }
    return 0;
}
