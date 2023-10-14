#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>  // for gettimeofday
#include <sys/types.h> // For pid_t
#include <stdbool.h>
#define MAX_HISTORY_SIZE 100
#define MAX_PIPELINE_SIZE 10
#define MAX_SCRIPT_LINE_LENGTH 1000
// Global array to store command history
char command_history[MAX_HISTORY_SIZE][1000];
int history_index = 0;
int process_index = 0;
struct ProcessInfo
{
    char name[100]; // Assuming a maximum string length of 100
    pid_t pid[100];
    long start_time; // Start time in milliseconds
    long end_time;   // End time in milliseconds
    long duration;   // Duration in milliseconds
};

struct ProcessInfo processinfo_history[MAX_HISTORY_SIZE][1000];
void add_process_to_history(struct ProcessInfo process, int index)
{
    if (index < MAX_HISTORY_SIZE)
    {
        // Copy the process information to the history array
        processinfo_history[index][0] = process;
    }
    else
    {
        // History is full, overwrite the oldest entry
        memmove(processinfo_history, processinfo_history + 1, sizeof(processinfo_history) - sizeof(processinfo_history[0]));
        processinfo_history[MAX_HISTORY_SIZE - 1][0] = process;
    }
}
void print_processinfo_history()
{
    for (int i = 0; i < MAX_HISTORY_SIZE; i++)
    {
        if (processinfo_history[i][0].name[0] == '\0')
        {
            // Entry is empty, break the loop
            break;
        }

        printf("Entry %d:\n", i + 1);
        printf("Name: %s\n", command_history[i]);
        printf("PIDs: ");
        printf("%d ", processinfo_history[i][0].pid[0]);
        printf("\n");
        printf("Start Time: %ld milliseconds\n", processinfo_history[i][0].start_time);
        printf("End Time: %ld milliseconds\n", processinfo_history[i][0].end_time);
        printf("Duration: %ld milliseconds\n", processinfo_history[i][0].duration);
        printf("\n");
    }
}
// Function to get the current time in microseconds
long long get_time_microseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + (long long)tv.tv_usec;
}

void add_to_history(char *command)
{
    if (history_index < MAX_HISTORY_SIZE)
    {
        strncpy(command_history[history_index], command, sizeof(command_history[0]) - 1);
        history_index++;
    }
    else
    {
        // History is full, overwrite the oldest command
        memmove(command_history, command_history + 1, sizeof(command_history) - sizeof(command_history[0]));
        strncpy(command_history[MAX_HISTORY_SIZE - 1], command, sizeof(command_history[0]) - 1);
    }
}

void print_history()
{
    printf("Command history:\n");
    for (int i = 0; i < history_index; i++)
    {
        printf("%s\n", command_history[i]);
    }
}

int create_process_and_run(char *command)
{
    // Tokenize the command into arguments
    char *args[10]; // Assuming a maximum of 10 arguments
    int arg_count = 0;

    // Tokenize the command using space as a delimiter
    char *token = strtok(command, " ");
    while (token != NULL)
    {
        args[arg_count] = token;
        arg_count++;

        // Get the next token
        token = strtok(NULL, " ");
    }

    args[arg_count] = NULL; // Null-terminate the argument list

    if (strncmp(command, "exit", 4) == 0)
    {
        return -1;
    }

    // Get the start time
    // long long start_time = get_time_microseconds();

    // Fork a new process
    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("Fork failed");
        return -1;
    }
    else if (child_pid == 0)
    {
        // This is the child process
        // Execute the command using execvp
        execvp(args[0], args);
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        // This is the parent process
        int status;
        waitpid(child_pid, &status, 0);

        return WEXITSTATUS(status);
    }
}

int execute_pipeline(char *pipeline[], int num_commands, char *command)
{
    int pipes[MAX_PIPELINE_SIZE - 1][2];
    int status = 0;
    // Get the start time
    long long start_time = get_time_microseconds();
    pid_t child_pids[num_commands];
    int child_pid_index = 0;

    // Create a separate variable to store the original command
    char command_name[100];
    strcpy(command_name, command);

    for (int i = 0; i < num_commands; i++)
    {
        if (i < num_commands - 1)
        {
            if (pipe(pipes[i]) == -1)
            {
                perror("Pipe creation failed");
                return -1;
            }
        }

        pid_t child_pid = fork();
        child_pids[child_pid_index++] = child_pid;

        if (child_pid == -1)
        {
            perror("Fork failed");
            return -1;
        }
        else if (child_pid == 0)
        {
            // This is the child process
            if (i > 0)
            {
                // Redirect stdin to the previous pipe's read end
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][0]);
            }

            if (i < num_commands - 1)
            {
                // Redirect stdout to the current pipe's write end
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][1]);
            }

            // Close all other pipe ends
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute the command
            if (strncmp(command, "exit", 4) == 0)
            {
                return -1;
            }

            status = create_process_and_run(pipeline[i]);
            exit(status);
        }
        else
        {
            // This is the parent process
            if (i > 0)
            {
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
            waitpid(child_pid, &status, 0);
        }
    }

    // Get the end time
    long long end_time = get_time_microseconds();

    // Calculate the total duration
    long long duration = end_time - start_time;

    struct ProcessInfo proinf;
    proinf.duration = duration;
    proinf.end_time = end_time;
    proinf.start_time = start_time;
    memcpy(proinf.name, command_name, sizeof(command_name));
    memcpy(proinf.pid, child_pids, sizeof(child_pids));
    add_process_to_history(proinf, process_index++);

    return WEXITSTATUS(status);
}

int execute_background(char *command)
{
    // Tokenize the command into arguments
    char *args[10]; // Assuming a maximum of 10 arguments
    int arg_count = 0;

    // Tokenize the command using space as a delimiter
    char *token = strtok(command, " ");
    while (token != NULL)
    {
        args[arg_count] = token;
        arg_count++;

        // Get the next token
        token = strtok(NULL, " ");
    }

    args[arg_count] = NULL; // Null-terminate the argument list

    if (strncmp(command, "exit", 4) == 0)
    {
        return -1;
    }

    // Get the start time
    long long start_time = get_time_microseconds();

    // Fork a new process
    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("Fork failed");
        return -1;
    }
    else if (child_pid == 0)
    {
        // This is the child process
        // Execute the command using execvp
        execvp(args[0], args);
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        // This is the parent process
        // Don't wait for the child to finish in the background
        printf("Background process started with PID %d\n", child_pid);

        // Record the process information
        struct ProcessInfo proinf;
        proinf.duration = -1; // Duration is unknown for background processes
        proinf.end_time = -1;
        proinf.start_time = start_time;
        strncpy(proinf.name, command, sizeof(proinf.name));
        proinf.name[sizeof(proinf.name) - 1] = '\0'; // Ensure null-terminated

        // Add the process information to history
        add_process_to_history(proinf, process_index++);

        return 0; // Return success for background processes
    }
}

int launch(char *command)
{
    int status;
    char *pipeline[MAX_PIPELINE_SIZE];
    int num_commands = 0;
    int is_background = 0;

    // Check if the command ends with "&" for background execution
    size_t cmd_length = strlen(command);
    if (cmd_length > 0 && command[cmd_length - 1] == '&')
    {
        // Remove the "&" symbol and set the background flag
        command[cmd_length - 1] = '\0';
        is_background = 1;
    }

    // Tokenize the command using "|" as a delimiter
    char *token = strtok(command, "|");
    while (token != NULL)
    {
        pipeline[num_commands] = token;
        num_commands++;

        // Get the next token
        token = strtok(NULL, "|");
    }

    // If it's a background process, execute it in the background
    if (is_background)
    {
        status = execute_background(command);
    }
    else
    {
        // Otherwise, execute it in the foreground
        status = execute_pipeline(pipeline, num_commands, command);
    }

    return status;
}

int execute_script(const char *script_file)
{
    FILE *file = fopen(script_file, "r");
    if (!file)
    {
        perror("Script file open failed");
        return -1;
    }

    char line[MAX_SCRIPT_LINE_LENGTH];
    int line_number = 0;
    int status = 0;
    bool in_command = false;
    char command_buffer[MAX_SCRIPT_LINE_LENGTH];
    size_t command_length = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_number++;
        size_t line_length = strlen(line);

        // Trim newline characters
        while (line_length > 0 && (line[line_length - 1] == '\n' || line[line_length - 1] == '\r'))
        {
            line_length--;
            line[line_length] = '\0';
        }

        // Skip empty lines and comments
        if (line_length == 0 || line[0] == '#')
        {
            continue;
        }

        // Check for multiline commands (commands spanning multiple lines)
        if (in_command)
        {
            // Append the current line to the command buffer
            strncat(command_buffer, line, sizeof(command_buffer) - command_length - 1);
            command_length += line_length;

            // Check if the command is complete
            if (line_length > 0 && line[line_length - 1] == '\\')
            {
                // This line continues the command
                continue;
            }
            else
            {
                // The command is complete, execute it
                status = launch(command_buffer);
                if (status == -1)
                {
                    break; // Exit the script execution on "exit" command
                }

                // Clear the command buffer
                command_buffer[0] = '\0';
                command_length = 0;
                in_command = false;
            }
        }
        else
        {
            // Check if this line starts a multiline command
            if (line_length > 0 && line[line_length - 1] == '\\')
            {
                // This line starts a multiline command
                in_command = true;
                // Remove the trailing '\' character
                line[line_length - 1] = '\0';
                strncpy(command_buffer, line, sizeof(command_buffer));
                command_length = line_length - 1; // Exclude the '\' character
            }
            else
            {
                // This line is a single-line command
                status = launch(line);
                if (status == -1)
                {
                    break; // Exit the script execution on "exit" command
                }
            }
        }
    }

    fclose(file);
    return status;
}

void shell_loop()
{
    int status = 1;
    char command[1000];
    do
    {
        printf("group80@xyz:~$ ");
        scanf(" %[^\n]", command);
        add_to_history(command);

        if (strncmp(command, "submit", 6) == 0)
        {
            const char *substring = command + 6;
            size_t length = strlen(substring);
            memmove(command, substring, length + 1);
        }
        else if (strncmp(command, "cd", 2) == 0)
        {
            char *token = strtok(command, " ");
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                if (chdir(token) != 0)
                {
                    perror("cd");
                }
            }
        }
        else if (strncmp(command, "history", 7) == 0)
        {
            print_history();
        }
        else if (strncmp(command, "exit", 4) == 0)
        {
            print_processinfo_history();
            status = -1;
        }
        else
        {
            status = launch(command);
        }
    } while (status != -1);
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        int status = execute_script(argv[1]);
        if (status == -1)
        {
            printf("Script execution failed.\n");
        }
        else
        {
            printf("Script execution completed.\n");
        }
    }
    else
    {
        shell_loop();
    }
    return 0;
}