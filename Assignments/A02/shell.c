#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_LINE 80 /* The maximum length command */

char **process_command(char cmd[], int *arg_count);
void add_to_history(char history[][MAX_LINE], char cmd[], int *cmd_count);

int main(void)
{
    int cmd_count = 0; /* number of commands */
    char history[5][MAX_LINE];
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh> ");
        fflush(stdout); /* flush the output buffer */

        /* read command */
        char cmd[MAX_LINE];
        if (fgets(cmd, MAX_LINE, stdin) == NULL){
            fprintf(stderr, "Error reading command");
            continue;
        }
        /* command line arguments*/
        int arg_count = 0;
        char **args = process_command(cmd, &arg_count);
    
        /* add command to history */
        add_to_history(history, cmd, &cmd_count);

        /* terminate when exit is entered */
        if (strcmp(cmd, "exit") == 0){
            should_run = 0;
            continue;
        }

        /* history print the last 5 commands */
        if (strcmp(cmd, "history") == 0){
            int bound = cmd_count < 5 ? cmd_count : 5;
            for (int i=0; i<bound; i++){
                printf("%d %s\n", cmd_count-i, history[i]);
            }
            continue;
        }

        /* !! */
        if (strcmp(cmd, "!!") == 0){
            if (cmd_count == 0){
                printf("No commands in history.\n");
            } 
            // execute the last command
            arg_count = 0;
            args = process_command(history[1], &arg_count);
            add_to_history(history, cmd, &cmd_count);
            if (execvp(args[0], args) == -1){ 
                fprintf(stderr, "execvp failed\n");
            }
            continue;
        }
        /**
         * After reading user input, the steps are:
         * (1) for a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) parent will invoke wait() unless command included &
         */

        /* create the child process and execute the command in the child */
        pid_t pid;
        pid = fork();

        if (pid < 0){ /* error occured */
            fprintf(stderr, "Fork Failed\n");
            return 1;
        } else if (pid == 0){ /* child process */
                if (strcmp(args[arg_count-1], "&") == 0) { 
                    args[arg_count-1] = NULL; // remove &
                }
                if (execvp(args[0], args) == -1){ 
                    fprintf(stderr, "execvp failed\n");
                    continue;
                }
        } else { /* parent process */
            if (strcmp(args[arg_count-1], "&") != 0) { 
                wait(NULL); /* parent waits */
            }
            // run concurrrently
        }
    }
    return 0;
}

char **process_command(char cmd[], int *arg_count){
    char **args = malloc((MAX_LINE / 2 + 1) * sizeof(char *));    
    char *token;
    cmd[strcspn(cmd, "\n")] = 0; // remove \n

    char cmd_copy[MAX_LINE];
    strcpy(cmd_copy, cmd); // make a copy of cmd

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

void add_to_history(char history[][MAX_LINE], char cmd[], int *cmd_count){
    for (int i=4; i>0; i--){
        strcpy(history[i],history[i-1]);
    }
    strcpy(history[0],cmd);
    (*cmd_count)++;
}


