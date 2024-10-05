#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

// Maximum length of commands 
#define MAX_LINE 80 

// Helper functions 
char **process_command(char cmd[], int *arg_count);
void add_to_history(char history[][MAX_LINE], char cmd[], int *cmd_count);

int main(void)
{
    int cmd_count = 0; // number of cmds 
    char history[5][MAX_LINE]; // 2D array that stores 5 cmds of MAX_LINE length 
    int should_run = 1; // flag to determine when to exit program 

    while (should_run) {
        printf("osh> ");
        fflush(stdout); // flush the output buffer 

        // Gets input for cmd 
        char cmd[MAX_LINE];
        if (fgets(cmd, MAX_LINE, stdin) == NULL){
            fprintf(stderr, "Error reading command\n");
            continue;
        }

        int arg_count = 0;
        char **args = process_command(cmd, &arg_count);

        // exit: terminate
        if (strcmp(cmd, "exit") == 0){
            should_run = 0;
            continue;
        }

        // history: print the last 5 commands from history
        if (strcmp(cmd, "history") == 0){ 
            int bound = cmd_count < 5 ? cmd_count : 5; // assigns bound to cmd_count if cmd_count < 5; else 5
            for (int i=0; i<bound; i++){
                printf("%d %s\n", cmd_count-i, history[i]);
            }
            continue;
        }

        // !!: execute the last command 
        if (strcmp(cmd, "!!") == 0){ 
            if (cmd_count == 0){
                printf("No commands in history\n");
                continue;
            } 

            // Copies the most recent cmd from history 
            char cmd2[MAX_LINE];
            strcpy(cmd2, history[0]);
            int arg_count2 = 0;
            char **args2 = process_command(cmd2, &arg_count2);
            add_to_history(history, cmd2, &cmd_count); // still increments the origin cmd_count 

            // Creates child process to run the cmd 
            pid_t pid2 = fork();
            if (pid2 < 0) { // error occurred 
                fprintf(stderr, "Fork Failed\n");
                return 1;
            } else if (pid2 == 0) { // child process
                if (execvp(args2[0], args2) == -1){ // checks if error occurs in execution 
                    fprintf(stderr, "Error executing command\n");
                    continue;
                }
            } else { // parent process 
                wait(NULL);
            }
            continue; // continues to next while loop, as to not fork again 
        } 

        add_to_history(history, cmd, &cmd_count); // must be after history, and !! cmd as to not add it  

        /* remove & */
        int concurrency_flag = 0;
        if (strcmp(args[arg_count-1], "&") == 0) { 
            concurrency_flag = 1;
            args[arg_count-1] = NULL; 
        }

        // Creates child process to run the cmd 
        pid_t pid;
        pid = fork();

        if (pid < 0){ // error occurred 
            fprintf(stderr, "Fork Failed\n");
            return 1;
        } else if (pid == 0){ // child process
            if (execvp(args[0], args) == -1){ // checks if error occurs in execution 
                fprintf(stderr, "Error executing command\n");
                continue;
            }
        } else { // parent process
            if (concurrency_flag) { 
                // runs concurrently
            } else {
                wait(NULL); // parent waits 
            }
        }
    }
    return 0;
}

// Tokenizes the cmd by spliting it's arguments into tokens and returns the tokens in an array of strings 
char **process_command(char cmd[], int *arg_count){
    
    cmd[strcspn(cmd, "\n")] = 0; // removes \n character 

    // Allocates memroy for the return array of strings 
    char **args = malloc((MAX_LINE / 2 + 1) * sizeof(char *));    
    char *token;

    // Makes a copy of cmd 
    char cmd_copy[MAX_LINE];
    strcpy(cmd_copy, cmd); 

    // Splits command into arguments and dynamically stores then in args 
    token = strtok(cmd_copy, " ");
    while (token != NULL) {
        args[*arg_count] = malloc(strlen(token) + 1);
        strcpy(args[*arg_count], token);
        (*arg_count)++; 
        token = strtok(NULL, " ");
    }
    args[*arg_count] = NULL; // NULL-terminated
    return args;
}

// Adds cmd to history by shifting all stored cmds down 1 
void add_to_history(char history[][MAX_LINE], char cmd[], int *cmd_count){
    for (int i=4; i>0; i--){
        strcpy(history[i],history[i-1]);
    }
    strcpy(history[0],cmd);
    (*cmd_count)++;
}