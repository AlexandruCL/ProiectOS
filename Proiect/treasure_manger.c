#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

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
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int treasure_id);
void remove_treasure(const char *hunt_id, int treasure_id);
void remove_hunt(const char *hunt_id);
void log_operation(const char *hunt_id, const char *operation);

int main(int argc, char *argv[]) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s <operation> <hunt_id> [additional arguments]\n", argv[0]);
            return EXIT_FAILURE;
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
        } else if (strcmp(operation, "list") == 0) {
            list_treasures(hunt_id);
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
}

void add_treasure(const char *hunt_id, const char *treasure_id) {
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "hunt/%s", hunt_id);

    // Create the hunt directory if it doesn't exist
    if (mkdir("hunt") == -1 && errno != EEXIST) {
        perror("Error creating hunt directory");
        return;
    }

    if (mkdir(dir_path) == -1 && errno != EEXIST) {
        perror("Error creating hunt subdirectory");
        return;
    }

    // If no treasure ID is provided, just create the hunt directory
    if (treasure_id == NULL) {
        printf("Hunt '%s' created successfully.\n", hunt_id);
        log_operation(hunt_id, "create_hunt");
        return;
    }

    // Add a treasure to the hunt
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", dir_path);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    treasure.id = atoi(treasure_id); // Use the provided treasure ID
    printf("Enter Username: ");
    scanf("%s", treasure.username);
    printf("Enter Latitude: ");
    scanf("%f", &treasure.latitude);
    printf("Enter Longitude: ");
    scanf("%f", &treasure.longitude);
    printf("Enter Clue: ");
    getchar(); // Consume newline
    fgets(treasure.clue, MAX_CLUE_LENGTH, stdin);
    treasure.clue[strcspn(treasure.clue, "\n")] = '\0'; // Remove newline
    printf("Enter Value: ");
    scanf("%d", &treasure.value);

    if (write(fd, &treasure, sizeof(Treasure)) == -1) {
        perror("Error writing to treasure file");
    } else {
        printf("Treasure '%s' added successfully to hunt '%s'.\n", treasure_id, hunt_id);
        log_operation(hunt_id, "add_treasure");
    }

    close(fd);
}

void list_treasures(const char *hunt_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("Error retrieving file information");
        return;
    }

    printf("Hunt: %s\n", hunt_id);
    printf("File Size: %ld bytes\n", file_stat.st_size);
    printf("Last Modified: %s", ctime(&file_stat.st_mtime));

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    printf("Treasures:\n");
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        printf("ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, Clue: %s, Value: %d\n",
               treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
    }

    close(fd);
}

void view_treasure(const char *hunt_id, int treasure_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.id == treasure_id) {
            printf("ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, Clue: %s, Value: %d\n",
                   treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
            close(fd);
            return;
        }
    }

    printf("Treasure with ID %d not found.\n", treasure_id);
    close(fd);
}

void remove_treasure(const char *hunt_id, int treasure_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return;
    }

    char temp_file_path[256];
    snprintf(temp_file_path, sizeof(temp_file_path), "%s/temp.dat", hunt_id);

    int temp_fd = open(temp_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        perror("Error creating temporary file");
        close(fd);
        return;
    }

    Treasure treasure;
    int found = 0;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.id == treasure_id) {
            found = 1;
        } else {
            write(temp_fd, &treasure, sizeof(Treasure));
        }
    }

    close(fd);
    close(temp_fd);

    if (found) {
        if (rename(temp_file_path, file_path) == -1) {
            perror("Error renaming temporary file");
        } else {
            printf("Treasure removed successfully.\n");
            log_operation(hunt_id, "remove_treasure");
        }
    } else {
        printf("Treasure with ID %d not found.\n", treasure_id);
        unlink(temp_file_path);
    }
}

void remove_hunt(const char *hunt_id) {
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);

    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, TREASURE_FILE);

    if (unlink(file_path) == -1) {
        perror("Error deleting treasure file");
        return;
    }

    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", dir_path, LOG_FILE);

    if (unlink(log_path) == -1) {
        perror("Error deleting log file");
        return;
    }

    if (rmdir(dir_path) == -1) {
        perror("Error deleting hunt directory");
        return;
    }

    printf("Hunt removed successfully.\n");
    log_operation(hunt_id, "remove_hunt");
}

void log_operation(const char *hunt_id, const char *operation) {
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

    fprintf(log_file, "[%s] %s\n", timestamp, operation);
    fclose(log_file);
}