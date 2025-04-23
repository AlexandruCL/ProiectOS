#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

pid_t monitor_pid = -1; 
int monitor_running = 0; 

void handle_sigchld(int sig) {
    (void)sig; 
    int status;
    pid_t pid = waitpid(monitor_pid, &status, WNOHANG);
    if (pid > 0) {
        monitor_running = 0;
        printf("Monitor process terminated with status %d.\n", WEXITSTATUS(status));
    }
}

void start_monitor() {
    if (monitor_running) {
        printf("Monitor is already running.\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == -1) {
        perror("Error starting monitor process");
        return;
    }

    if (monitor_pid == 0) {
        execl("./treasure_manager", "./treasure_manager", "monitor", NULL);
        perror("Error starting monitor process");
        exit(EXIT_FAILURE);
    }

    monitor_running = 1;
    printf("Monitor process started with PID %d.\n", monitor_pid);
}

void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor is not running.\n");
        return;
    }

    kill(monitor_pid, SIGUSR2); 
    printf("Sent stop signal to monitor process.\n");
}

void send_command_to_monitor(const char *command) {
    if (!monitor_running) {
        printf("Monitor is not running. Cannot send command.\n");
        return;
    }

    char full_command[256];
    snprintf(full_command, sizeof(full_command), "%s", command);

    if (strcmp(command, "list_hunt") == 0 || strcmp(command, "view_treasure") == 0) {
        char hunt_id[128];
        printf("Enter hunt ID: ");
        if (!fgets(hunt_id, sizeof(hunt_id), stdin)) {
            perror("Error reading hunt ID");
            return;
        }
        hunt_id[strcspn(hunt_id, "\n")] = '\0'; // Remove newline
        strncat(full_command, " ", sizeof(full_command) - strlen(full_command) - 1);
        strncat(full_command, hunt_id, sizeof(full_command) - strlen(full_command) - 1);

        if (strcmp(command, "view_treasure") == 0) {
            char treasure_id[128];
            printf("Enter treasure ID: ");
            if (!fgets(treasure_id, sizeof(treasure_id), stdin)) {
                perror("Error reading treasure ID");
                return;
            }
            treasure_id[strcspn(treasure_id, "\n")] = '\0'; // Remove newline
            strncat(full_command, " ", sizeof(full_command) - strlen(full_command) - 1);
            strncat(full_command, treasure_id, sizeof(full_command) - strlen(full_command) - 1);
        }
    }

    FILE *command_file = fopen("monitor_command.txt", "w");
    if (!command_file) {
        perror("Error opening command file");
        return;
    }

    fprintf(command_file, "%s\n", full_command); // Write the full command to the file
    fclose(command_file);

    kill(monitor_pid, SIGUSR1); // Notify the monitor of the new command
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    char input[256];
    while (1) {
        printf("treasure_hub> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        input[strcspn(input, "\n")] = '\0'; 

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        // } else if (strcmp(input, "list_hunts") == 0) {
        //     send_command_to_monitor("list_hunts");
        } else if (strcmp(input, "list_hunt") == 0) {
            send_command_to_monitor("list_hunt");
        } else if (strcmp(input, "view_treasure") == 0) {
            send_command_to_monitor("view_treasure");
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_running) {
                printf("Error: Monitor is still running. Stop it first.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command: %s\n", input);
        }
    }

    if (monitor_running) {
        printf("Stopping monitor before exiting...\n");
        stop_monitor();
        while (monitor_running) {
            sleep(1); 
        }
    }

    printf("Exiting treasure_hub.\n");
    return 0;
}