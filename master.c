#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_WORDS 8192
#define WORD_LENGTH 24

// This function applies the ROT13 cipher to the buffer
void applyROT13(char *message) {
    char *ptr = message;
    while (*ptr != '\0') {
        char c = *ptr;
        if ((c >= 'a' && c <= 'z')) {
            *ptr = (((c - 'a') + 13) % 26) + 'a';
        } else if ((c >= 'A' && c <= 'Z')) {
            *ptr = (((c - 'A') + 13) % 26) + 'A';
        }
        ptr++;
    }
}

// This function counts the words in the buffer
void countWords(char *buffer, char words[][WORD_LENGTH], int counts[]) {
    char word[WORD_LENGTH];
    int index = 0, offset = 0;
    memset(words, 0, sizeof(words[0]) * NUM_WORDS);
    memset(counts, 0, sizeof(counts[0]) * NUM_WORDS);

    while (sscanf(buffer + offset, "%23s%n", word, &index) == 1) {
        // The buffer message is decrypted using the ROT13 cipher
        applyROT13(word);
        for (int i = 0; i < NUM_WORDS; i++) {
            if (strcmp(words[i], word) == 0) {
                counts[i]++;
                break;
            }
            if (strlen(words[i]) == 0) {
                strcpy(words[i], word);
                counts[i]++;
                break;
            }
        }
        offset += index;
        if (offset >= strlen(buffer)) break;
    }
}

// Main function
// The program reads a file and divides it into two buffers
// The Slave process counts the words in the first buffer
// The Slave1 process counts the words in the second buffer
int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char *file_path = "input.txt";
    FILE *file = fopen(file_path, "rb");

    if (file == NULL) {
        perror("Error opening file");
        MPI_Finalize();
        return 1;
    }

    // Divides the file into two buffers
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    long half_size = file_size / 2;
    fseek(file, 0, SEEK_SET);

    char *bufferA = malloc(half_size + 1);
    char *bufferB = malloc(file_size - half_size + 1);
    fread(bufferA, 1, half_size, file);
    fread(bufferB, 1, file_size - half_size, file);
    bufferA[half_size] = '\0';
    bufferB[file_size - half_size] = '\0';
    fclose(file);

    //Distibutes the buffers to the Slave processes
    char words[NUM_WORDS][WORD_LENGTH];
    int counts[NUM_WORDS] = {0};
    // The master sends the first buffer to the Slave process
    if (rank == 0) {
        countWords(bufferA, words, counts);
        MPI_Send(bufferB, strlen(bufferB) + 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        // The master sends the results to the Slave1 process
    } else if (rank == 1) {
        MPI_Recv(bufferB, file_size - half_size + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        countWords(bufferB, words, counts);
        MPI_Send(counts, NUM_WORDS, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    // The master receives the results from the Slave process
    if (rank == 0) {
        int counts_slave[NUM_WORDS];
        MPI_Recv(counts_slave, NUM_WORDS, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Combina los conteos
        for (int i = 0; i < NUM_WORDS; i++) counts[i] += counts_slave[i];

        // Determina la palabra con mayor frecuencia
        int maxIndex = 0;
        for (int i = 1; i < NUM_WORDS; i++) {
            if (counts[i] > counts[maxIndex]) maxIndex = i;
        }
        printf("Trend word is: %s with %d repetitions.\n", words[maxIndex], counts[maxIndex]);
    }

    // Free memory and finalize MPI
    free(bufferA);
    free(bufferB);
    MPI_Finalize();
    return 0;
}
