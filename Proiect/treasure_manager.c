#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#define MAX_CLUE_LENGTH 256
#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"

// Structure to represent a treasure
typedef struct {
    int id;
    char username[50];
    float latitude;
    float longitude;
    char clue[MAX_CLUE_LENGTH];
    int value;
} Treasure;

// Function prototypes
void add_treasure(const char *hunt_id, const char *treasure_id);
void list_hunt(const char *hunt_id);
void view_treasure(const char *hunt_id, int treasure_id);
void remove_treasure(const char *hunt_id, int treasure_id);
void remove_hunt(const char *hunt_id);
void log_operation(const char *hunt_id, const char *operation, const char *details);
void handle_sigusr1(int sig);
void handle_sigusr2(int sig);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <operation> <hunt_id> [additional arguments]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "monitor") == 0) {
        // Monitor mode
        printf("Monitor process started. Waiting for commands...\n");

        // Set up signal handlers
        struct sigaction sa_usr1, sa_usr2;
        sa_usr1.sa_handler = handle_sigusr1;
        sa_usr1.sa_flags = SA_RESTART;
        sigaction(SIGUSR1, &sa_usr1, NULL);

        sa_usr2.sa_handler = handle_sigusr2;
        sa_usr2.sa_flags = SA_RESTART;
        sigaction(SIGUSR2, &sa_usr2, NULL);

        // Wait indefinitely for signals
        while (1) {
            pause(); // Wait for a signal
        }
    }

    const char *operation = argv[1];
    const char *hunt_id = argv[2];

    if (strcmp(operation, "add") == 0) {
        if (argc == 3) {
            // Only hunt ID is provided, create the hunt
            add_treasure(hunt_id, NULL);
        } else if (argc == 4) {
            // Hunt ID and treasure ID are provided, add a treasure
            const char *treasure_id = argv[3];
            add_treasure(hunt_id, treasure_id);
        } else {
            fprintf(stderr, "Usage: %s add <hunt_id> [treasure_id]\n", argv[0]);
            return EXIT_FAILURE;
        }
    } else if (strcmp(operation, "list") == 0) {
        list_hunt(hunt_id);
    } else if (strcmp(operation, "view") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s view <hunt_id> <treasure_id>\n", argv[0]);
            return EXIT_FAILURE;
        }
        int treasure_id = atoi(argv[3]);
        view_treasure(hunt_id, treasure_id);
    } else if (strcmp(operation, "remove_treasure") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s remove_treasure <hunt_id> <treasure_id>\n", argv[0]);
            return EXIT_FAILURE;
        }
        int treasure_id = atoi(argv[3]);
        remove_treasure(hunt_id, treasure_id);
    } else if (strcmp(operation, "remove_hunt") == 0) {
        remove_hunt(hunt_id);
    } else {
        fprintf(stderr, "Unknown operation: %s\n", operation);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void add_treasure(const char *hunt_id, const char *treasure_id) {
    char dir_path[PATH_MAX];
    snprintf(dir_path, sizeof(dir_path), "hunt/%s", hunt_id);

    // Create the hunt directory if it doesn't exist
    if (mkdir("hunt", 0755) == -1 && errno != EEXIST) {
        perror("Error creating hunt directory");
        return;
    }

    if (mkdir(dir_path,0755) == -1 && errno != EEXIST) {
        perror("Error creating hunt subdirectory");
        return;
    }

    // Create the logs directory in the root folder if it doesn't exist
    if (mkdir("logs", 0755) == -1 && errno != EEXIST) {
        perror("Error creating logs directory");
        return;
    }

    // If no treasure ID is provided, just create the hunt directory
    if (treasure_id == NULL) {
        printf("Hunt '%s' created successfully.\n", hunt_id);
        log_operation(hunt_id, "create_hunt", "Hunt created");
            // Ensure log file exists
            char log_file_path[PATH_MAX];
            snprintf(log_file_path, sizeof(log_file_path), "%s/hunt/%s/logged_hunt", getcwd(NULL, 0), hunt_id);
            int fd = open(log_file_path, O_CREAT | O_WRONLY, 0644);
            if (fd == -1) {
                perror("Error creating log file");
                return;
            }
            close(fd);
            
            // Create symlink
            char symlink_path[PATH_MAX];
            snprintf(symlink_path, sizeof(symlink_path), "%s/logs/logs_%s", getcwd(NULL, 0), hunt_id);
            
            // Remove old symlink if it exists
            remove(symlink_path);
            
            if (symlink(log_file_path, symlink_path) == -1) {
                perror("Error creating symbolic link in logs folder");
                log_operation(hunt_id, "create_symlink", "Failed to create symlink");
            } else {
                printf("Symlink created: %s -> %s\n", symlink_path, log_file_path);
                log_operation(hunt_id, "create_symlink", "Symlink created successfully");
            }
        return;
    }

    // Add a treasure to the hunt
    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", dir_path);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    treasure.id = atoi(treasure_id); // Use the provided treasure ID

    printf("Enter Username: ");
    if (scanf("%49s", treasure.username) != 1) {
        perror("Error reading username");
        close(fd);
        return;
    }

    printf("Enter Latitude: ");
    if (scanf("%f", &treasure.latitude) != 1) {
        perror("Error reading latitude");
        close(fd);
        return;
    }

    printf("Enter Longitude: ");
    if (scanf("%f", &treasure.longitude) != 1) {
        perror("Error reading longitude");
        close(fd);
        return;
    }

    printf("Enter Clue: ");
    getchar(); // Consume newline
    if (fgets(treasure.clue, MAX_CLUE_LENGTH, stdin) == NULL) {
        perror("Error reading clue");
        close(fd);
        return;
    }
    treasure.clue[strcspn(treasure.clue, "\n")] = '\0'; // Remove newline

    printf("Enter Value: ");
    if (scanf("%d", &treasure.value) != 1) {
        perror("Error reading value");
        close(fd);
        return;
    }

    // Write treasure to file
    ssize_t bytes_written = write(fd, &treasure, sizeof(Treasure));
    if (bytes_written != sizeof(Treasure)) {
        perror("Error writing full treasure to file");
        close(fd);
        return;
    }

    printf("Treasure '%s' added successfully to hunt '%s'.\n", treasure_id, hunt_id);
    // After successfully adding the treasure
    char log_details[512];
    snprintf(log_details, sizeof(log_details), "Added treasure ID %d, Username: %s, Latitude: %.2f, Longitude: %.2f, Clue: %s, Value: %d",
             treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
    log_operation(hunt_id, "add_treasure", log_details);
    close(fd);
}

void list_hunt(const char *hunt_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "hunt/%s/%s", hunt_id, TREASURE_FILE);

    // Get file information
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("Error retrieving file information");
        return;
    }

    printf("Hunt: %s\n", hunt_id);
    printf("File Size: %ld bytes\n", file_stat.st_size);
    printf("Last Modified: %s", ctime(&file_stat.st_mtime));

    // Open the treasures file
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    // Read and display treasures
    Treasure treasure;
    printf("Treasures:\n");
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        printf("ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, Clue: %s, Value: %d\n",
               treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
    }

    close(fd);
    char log_details[512];
    snprintf(log_details, sizeof(log_details), "Listed all treasures in hunt '%s'", hunt_id);
    log_operation(hunt_id, "list_hunt", log_details);
}

void view_treasure(const char *hunt_id, int treasure_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "hunt/%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    int found = 0;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.id == treasure_id) {
            printf("ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, Clue: %s, Value: %d\n",
                   treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
            found = 1;
        }
    }
    if(!found){
        printf("Treasure with ID %d not found.\n", treasure_id);
    }
    if(found){
        char log_details[512];
        snprintf(log_details, sizeof(log_details), "Viewed treasure ID %d, Username: %s, Latitude: %.2f, Longitude: %.2f, Clue: %s, Value: %d",
                 treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
        log_operation(hunt_id, "view_treasure", log_details);
    }
    close(fd);
}

void remove_treasure(const char *hunt_id, int treasure_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "hunt/%s/%s", hunt_id, TREASURE_FILE);

    // Open the treasures file for reading
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    // Create a temporary file to store the updated treasures
    char temp_file_path[256];
    snprintf(temp_file_path, sizeof(temp_file_path), "hunt/%s/temp.dat", hunt_id);

    int temp_fd = open(temp_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        perror("Error creating temporary file");
        close(fd);
        return;
    }

    Treasure treasure;
    int found = 0;

    // Read treasures from the original file and process them
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.id == treasure_id) {
            found = 1; // Mark that the treasure was found
        } else {
            // If the treasure ID is greater than the removed ID, decrement it
            if (treasure.id > treasure_id) {
                treasure.id -= 1;
            }
            if (write(temp_fd, &treasure, sizeof(Treasure)) != sizeof(Treasure)) {
                perror("Error writing to temporary file");
                close(fd);
                close(temp_fd);
                unlink(temp_file_path);
                return;
            }
        }
    }

    close(fd);
    close(temp_fd);

    if (found) {
        // Replace the original file with the updated temporary file
        if (rename(temp_file_path, file_path) == -1) {
            perror("Error renaming temporary file");
            unlink(temp_file_path);
        } else {
            printf("Treasure with ID %d removed successfully from hunt '%s'.\n", treasure_id, hunt_id);
            char log_details[256];
            snprintf(log_details, sizeof(log_details), "Removed treasure ID %d and reindexed remaining treasures", treasure_id);
            log_operation(hunt_id, "remove_treasure", log_details);
        }
    } else {
        printf("Treasure with ID %d not found in hunt '%s'.\n", treasure_id, hunt_id);
        unlink(temp_file_path); // Clean up the temporary file
    }
}

void remove_hunt(const char *hunt_id) {
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "hunt/%s", hunt_id);

    char treasure_file_path[256];
    snprintf(treasure_file_path, sizeof(treasure_file_path), "%s/%s", dir_path, TREASURE_FILE);

    log_operation(hunt_id, "remove_hunt", "Hunt removed and log file moved");
    // Delete the treasure file if it exists
    if (unlink(treasure_file_path) == -1 && errno != ENOENT) {
        perror("Error deleting treasure file");
        return;
    }

    char log_file_path[256];
    snprintf(log_file_path, sizeof(log_file_path), "%s/%s", dir_path, LOG_FILE);

    // Move the log file to the root logs folder
    char final_log_path[256];
    snprintf(final_log_path, sizeof(final_log_path), "logs/final_logs%s", hunt_id);

    if (rename(log_file_path, final_log_path) == -1) {
        perror("Error moving log file to logs folder");
        return;
    }

    // Delete the symlink from the logs folder
    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "logs/logs_%s", hunt_id);
    if (unlink(symlink_path) == -1 && errno != ENOENT) {
        perror("Error deleting symlink from logs folder");
        return;
    }
    
    // Delete the hunt directory
    if (rmdir(dir_path) == -1) {
        perror("Error deleting hunt directory");
        return;
    }

    printf("Hunt '%s' removed successfully. Log file moved to '%s'.\n", hunt_id, final_log_path);
    
}

void log_operation(const char *hunt_id, const char *operation, const char *details) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "hunt/%s/logged_hunt", hunt_id);

    FILE *log_file = fopen(log_path, "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strcspn(timestamp, "\n")] = '\0'; // Remove newline

    fprintf(log_file, "[%s] %s: %s\n", timestamp, operation, details);
    fclose(log_file);
}

void handle_sigusr1(int sig) {
    (void)sig;
    FILE *command_file = fopen("monitor_command.txt", "r");
    if (!command_file) {
        perror("Error opening command file");
        return;
    }

    char command[256];
    if (fgets(command, sizeof(command), command_file)) {
        command[strcspn(command, "\n")] = '\0'; // Remove newline
        printf("Executing command: %s\n", command);

        // Redirect output to a file
        FILE *output_file = fopen("monitor_output.txt", "w");
        if (!output_file) {
            perror("Error opening output file");
            fclose(command_file);
            return;
        }

        // Parse the command
        char *operation = strtok(command, " ");
        char *hunt_id = strtok(NULL, " ");
        char *treasure_id = strtok(NULL, " ");

        if (strcmp(operation, "list_hunt") == 0) {
            if (hunt_id) {
                fprintf(output_file, "Listing treasures for hunt ID: %s\n", hunt_id);
                // Redirect the output of list_hunt to the file
                // Example: list_hunt(hunt_id, output_file);
                list_hunt(hunt_id); // Modify list_hunt to accept output_file if needed
            } else {
                fprintf(output_file, "Error: Hunt ID is required for list_hunt.\n");
            }
        } else if (strcmp(operation, "view_treasure") == 0) {
            if (hunt_id && treasure_id) {
                fprintf(output_file, "Viewing treasure ID %s in hunt ID: %s\n", treasure_id, hunt_id);
                // Redirect the output of view_treasure to the file
                // Example: view_treasure(hunt_id, atoi(treasure_id), output_file);
                view_treasure(hunt_id, atoi(treasure_id)); // Modify view_treasure to accept output_file if needed
            } else {
                fprintf(output_file, "Error: Hunt ID and Treasure ID are required for view_treasure.\n");
            }
        } else {
            fprintf(output_file, "Unknown command: %s\n", operation);
        }

        fclose(output_file);
    }
    fclose(command_file);
}

void handle_sigusr2(int sig) {
    (void)sig;
    printf("Monitor process terminating...\n");
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 500000000;
    nanosleep(&ts, NULL); 
    exit(0);
}