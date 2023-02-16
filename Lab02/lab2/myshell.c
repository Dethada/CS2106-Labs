/**
 * CS2106 AY22/23 Semester 2 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PROCESS 50

// PCB Status Codes
#define EXITED 1
#define RUNNING 2
#define TERMINATING 3
#define STOPPED 4

typedef struct Command {
    char *command;
    int argc;
    char **argv;
} Command;

typedef struct PCBTable PCBTable;

// typedef struct PCBList {
//     int pointer;
//     PCBTable *table;
// }

PCBTable *g_PCBList[MAX_PROCESS] = { NULL };

static pid_t mywaitpid(pid_t pid, int *status, int options) {
    pid_t ret = waitpid(pid, status, options);
    if (ret < 0) {
        fprintf(stderr, "waitpid failed\n");
    }
    // if (ret >= 0) {
    //     for (int i = 0; i < MAX_PROCESS; i++) {
    //         if (g_PCBList[i] == NULL) {
    //             break;
    //         } else if (g_PCBList[i]->pid == ret) {
    //             g_PCBList[i]->status = EXITED;
    //             g_PCBList[i]->exitCode = WEXITSTATUS(*status);
    //             break;
    //         }
    //     }
    // } else if (ret < 0) {
    //     fprintf(stderr, "waitpid failed\n");
    // }
    return ret;
}

/*******************************************************************************
 * Signal handler : ex4
 ******************************************************************************/

static void signal_handler(int signo) {

        // Use the signo to identy ctrl-Z or ctrl-C and print “[PID] stopped or print “[PID] interrupted accordingly.
        // Update the status of the process in the PCB table

}

static void update_pcb(PCBTable *pcb, int status) {
    if (WIFEXITED(status)) {
        pcb->status = EXITED;
        pcb->exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        pcb->status = EXITED;
        pcb->exitCode = WTERMSIG(status);
    } else if (WIFSTOPPED(status)) {
        pcb->status = STOPPED;
    }
}

static void proc_update_status() {
    for (int i = 0; i < MAX_PROCESS; i++) {
        if (g_PCBList[i] == NULL) {
            break;
        }
        if (g_PCBList[i]->status != EXITED) {
            int status;
            pid_t ret = mywaitpid(g_PCBList[i]->pid, &status, WNOHANG);
            if (ret == g_PCBList[i]->pid) {
                update_pcb(g_PCBList[i], status);
            }
        }
    }
}


/*******************************************************************************
 * Built-in Commands
 ******************************************************************************/

static void command_info(Command *cmd) {
    if (cmd->argc != 1) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    // parse option
    char *endptr;
    unsigned long option = strtoul(cmd->argv[0], &endptr, 10);

    if (endptr == cmd->argv[0] || *endptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "Wrong command\n");
        return;
    }
    char *statusCaps[] = { "Exited", "Running", "Terminating", "Stopped" };
    char *statusLower[] = { "exited", "running", "terminating", "stopped" };

    if (option == 0) {
        // print details of all processes
        proc_update_status();
        for (int i = 0; i < MAX_PROCESS; i++) {
            if (g_PCBList[i] == NULL) {
                break;
            }
            if (g_PCBList[i]->status == EXITED) {
                printf("[%d] %s %d\n", g_PCBList[i]->pid, statusCaps[g_PCBList[i]->status-1], g_PCBList[i]->exitCode);
            } else {
                printf("[%d] %s\n", g_PCBList[i]->pid, statusCaps[g_PCBList[i]->status-1]);
            }
        }
    } else if (option >= 1 && option <= 4) {
        proc_update_status();
        uint count = 0;
        for (int i = 0; i < MAX_PROCESS; i++) {
            if (g_PCBList[i] == NULL) {
                break;
            } else if (g_PCBList[i]->status == option) {
                count++;
            }
        }
        printf("Total %s process: %d\n", statusLower[option-1], count);
    } else {
        fprintf(stderr, "Wrong command\n");
    }
}

static void command_wait(Command *cmd) {
    if (cmd->argc != 1) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    char *endptr;
    pid_t pid = strtoul(cmd->argv[0], &endptr, 10);

    if (endptr == cmd->argv[0] || *endptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    // seach PCB List for the pid
    for (int i = 0; i < MAX_PROCESS; i++) {
        if (g_PCBList[i] == NULL) {
            break;
        } else if (g_PCBList[i]->pid == pid) {
            if (g_PCBList[i]->status == RUNNING) {
                int status;
                pid_t ret = mywaitpid(pid, &status, WUNTRACED);
                // proc_update_status();
                if (ret == pid) {
                    update_pcb(g_PCBList[i], status);
                }
            }
            break;
        }
    }
}


static void command_terminate(Command *cmd) {
    if (cmd->argc != 1) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    char *endptr;
    pid_t pid = strtoul(cmd->argv[0], &endptr, 10);

    if (endptr == cmd->argv[0] || *endptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    for (int i = 0; i < MAX_PROCESS; i++) {
        if (g_PCBList[i] == NULL) {
            break;
        } else if (g_PCBList[i]->pid == pid) {
            if (g_PCBList[i]->status == RUNNING) {
                kill(pid, SIGTERM);
                g_PCBList[i]->status = TERMINATING;
            }
            break;
        }
    }
}

static void command_fg(Command *cmd) {

        /******* FILL IN THE CODE *******/


    // if the {PID} status is stopped
        //Print “[PID] resumed”
        // Use kill() to send SIGCONT to {PID} to get it continue and wait for it
        // After the process terminate, update status and exit code (call proc_update_status())
}


/*******************************************************************************
 * Program Execution
 ******************************************************************************/

static void command_exec(Command *cmd) {

        /******* FILL IN THE CODE *******/


    // check if program exists and is executable : use access()
    if (access(cmd->command, R_OK | X_OK) != 0) {
        // file is not readable or not executable
        fprintf(stderr, "%s not found\n", cmd->command);
        return;
    }

    // strip & from the end of the command if present
    bool background = false;
    if (strcmp(cmd->argv[cmd->argc-1], "&") == 0) {
        background = true;
        cmd->argv[cmd->argc-1] = NULL;
        cmd->argc--;
    }

    // fork a subprocess and execute the program
    pid_t pid = fork();
    if (pid < 0) {
        // fork failed
        fprintf(stderr, "fork failed\n");
        return;
    }
    if (pid == 0) {
        // CHILD PROCESS


        // check file redirection operation is present : ex3

        // if < or > or 2> present:
            // use fopen/open file to open the file for reading/writing with  permission O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC, O_SYNC and 0644
            // use dup2 to redirect the stdin, stdout and stderr to the files
            // call execv() to execute the command in the child process

        // else : ex1, ex2
            // call execv() to execute the command in the child process
            // decrement argv by 1 to include the command name which is required by execv()
        execv(cmd->command, cmd->argv-1);

        // Exit the child
        // The exec() functions only return if an error has occurred. The return value is -1, and errno is set to indicate the error.
        fprintf(stderr, "%s failed to executed.\n", cmd->command);
        exit(EXIT_FAILURE);
    } else {


        // PARENT PROCESS
        // register the process in process table
        PCBTable *pcb = malloc(sizeof(PCBTable));
        pcb->pid = pid;
        pcb->status = RUNNING;
        pcb->exitCode = -1;

        // insert into PCB List
        for (int i = 0; i < MAX_PROCESS; i++) {
            if (g_PCBList[i] == NULL) {
                g_PCBList[i] = pcb;
                break;
            }
        }

        if (background) {
            // If  child process need to execute in the background  (if & is present at the end )
            printf("Child [%d] in background\n", pid);
            if (mywaitpid(pid, &pcb->exitCode, WNOHANG) == pid) {
                pcb->status = EXITED;
                pcb->exitCode = WEXITSTATUS(pcb->exitCode);
            }
        } else {
            // else wait for the child process to exit
            // Use waitpid() with WNOHANG when not blocking during wait and  waitpid() with WUNTRACED when parent needs to block due to wait
            pid_t ret = mywaitpid(pid, &pcb->exitCode, WUNTRACED);
            // proc_update_status();
            if (ret == pid) {
                update_pcb(pcb, pcb->exitCode);
            }
        }
    }
}



/*******************************************************************************
 * Command Processor
 ******************************************************************************/

static void command(Command *cmd) {
    // printf("Cmd: %s\nArgs: ", cmd->command);
    // for (int i = 0; i < cmd->argc+1; i++) {
    //     printf("%s ", cmd->argv[i]);
    // }
    // printf("\n================\n");

    if (strcmp(cmd->command, "info") == 0) {
        command_info(cmd);
    } else if (strcmp(cmd->command, "wait") == 0) {
        command_wait(cmd);
    } else if (strcmp(cmd->command, "terminate") == 0) {
        command_terminate(cmd);
    } else if (strcmp(cmd->command, "fg") == 0) {
        command_fg(cmd);
    } else {
        command_exec(cmd);
    }
}

/*******************************************************************************
 * High-level Procedure
 ******************************************************************************/

void my_init(void) {

           /******* FILL IN THE CODE *******/

        // use signal() with SIGTSTP to setup a signalhandler for ctrl+z : ex4
        // use signal() with SIGINT to setup a signalhandler for ctrl+c  : ex4

        // anything else you require

}

void my_process_command(size_t num_tokens, char **tokens) {


    /******* FILL IN THE CODE *******/

    // Split tokens at NULL or ; to get a single command (ex1, ex2, ex3, ex4(fg command))

    // for example :  /bin/ls ; /bin/sleep 5 ; /bin/pwd
    // split the above line as first command : /bin/ls , second command: /bin/pwd  and third command:  /bin/pwd
    // Call command() and pass each individual command as arguements

    // printf("%zu tokens\n", num_tokens);
    uint command_count = 1;
    for (int i = 0; i < num_tokens - 1; i++) {
        if (strcmp(tokens[i], ";") == 0) {
            tokens[i] = NULL;
            command_count++;
        }
    }
    for (int i = 0; i < command_count; i++) {
        int argc = -1;
        char **start = tokens;
        while (*tokens != NULL) {
            tokens++;
            argc++;
        }

        Command *cmd = malloc(sizeof(Command));
        cmd->command = start[0];
        cmd->argc = argc;
        cmd->argv = start+1;
        command(cmd);
        free(cmd);

        // skip the NULL element
        tokens++;
    }
}

void my_quit(void) {
    // Kill every process in the PCB that is either stopped or running
    for (int i = 0; i < MAX_PROCESS; i++) {
        if (g_PCBList[i] == NULL) {
            break;
        }
        if (g_PCBList[i]->status == RUNNING || g_PCBList[i]->status == STOPPED) {
            printf("Killing [%d]\n", g_PCBList[i]->pid);
            kill(g_PCBList[i]->pid, SIGKILL);
        }
        free(g_PCBList[i]);
    }

    printf("\nGoodbye\n");
}
