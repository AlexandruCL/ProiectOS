#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define TREASURE_FILE "treasures.dat"

typedef struct {
    int id;
    char username[50];
    float latitude;
    float longitude;
    char clue[256];
    int value;
} Treasure;

void calculate_scores(const char *hunt_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "hunt/%s/%s", hunt_id, TREASURE_FILE);

    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening treasure file");
        return;
    }

    // Use a mapping for usernames and their scores
    struct {
        char username[50];
        int score;
    } user_scores[100]; // Assume max 100 users
    int user_count = 0;

    Treasure treasure;

    printf("Scores and Treasure Details for Hunt ID: %s\n", hunt_id);
    printf("--------------------------------------------------\n");

    while (fread(&treasure, sizeof(Treasure), 1, file)) {
        // Find or add the username to the user_scores array
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(user_scores[i].username, treasure.username) == 0) {
                user_scores[i].score += treasure.value;
                found = 1;
                break;
            }
        }
        if (!found) {
            strncpy(user_scores[user_count].username, treasure.username, sizeof(user_scores[user_count].username) - 1);
            user_scores[user_count].username[sizeof(user_scores[user_count].username) - 1] = '\0';
            user_scores[user_count].score = treasure.value;
            user_count++;
        }

        // Display treasure details
        printf("Treasure ID: %d\n", treasure.id);
        printf("  Username: %s\n", treasure.username);
        printf("  Value: %d\n", treasure.value);
        printf("--------------------------------------------------\n");
    }

    fclose(file);

    // Output scores
    printf("User Scores:\n");
    for (int i = 0; i < user_count; i++) {
        printf("  Username: %s, Score: %d\n", user_scores[i].username, user_scores[i].score);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    calculate_scores(argv[1]);
    return EXIT_SUCCESS;
}