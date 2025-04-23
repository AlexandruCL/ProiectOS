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

    FILE *command_file = fopen("monitor_command.txt", "w");
    if (!command_file) {
        perror("Error opening command file");
        return;
    }

    fprintf(command_file, "%s\n", command); 
    fclose(command_file);

    kill(monitor_pid, SIGUSR1); 
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
        } else if (strcmp(input, "list_hunts") == 0) {
            send_command_to_monitor("list_hunts");
        } else if (strcmp(input, "list_treasures") == 0) {
            send_command_to_monitor("list_treasures");
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