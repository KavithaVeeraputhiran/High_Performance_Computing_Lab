#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define SIZE 20
#define MAXLINE 100
#define MAX_FILES 10

char buffer[SIZE][MAXLINE];
int count = 0;
int finished_producers = 0;
int num_files = 0;
omp_lock_t lock;
char *filenames[MAX_FILES];
FILE *outFile;

void producer(int id) {
    FILE *fp = fopen(filenames[id], "r");
    if (fp == NULL) {
        printf("Error: Cannot open %s\n", filenames[id]);
        return;
    }
    char line[MAXLINE];
    while (fgets(line, MAXLINE, fp)) {
        int inserted = 0;
        while (!inserted) {
            omp_set_lock(&lock);
            if (count < SIZE) {
                strcpy(buffer[count], line);
                #pragma omp atomic
                count++;
                inserted = 1;
            }
            omp_unset_lock(&lock);
        }
        #pragma omp critical
        {
            printf("Producer %d --> %s", id, line);
        }
    }
    fclose(fp);
    #pragma omp atomic
    finished_producers++;
}

void consumer(int id) {
    while (1) {
        char line[MAXLINE];
        int taken = 0;

        omp_set_lock(&lock);

        // Check termination condition
        if (finished_producers == num_files && count == 0) {
            omp_unset_lock(&lock);
            break;
        }

        // Try to take item
        if (count > 0) {
            #pragma omp atomic
            count--;
            strcpy(line, buffer[count]);
            taken = 1;
        }

        omp_unset_lock(&lock);

        if (taken) {
            char temp[MAXLINE];
            strcpy(temp, line);
            char *token = strtok(temp, " \t\n");
            while (token != NULL) {
                #pragma omp critical
                {
                    fprintf(outFile, "Consumer %d: %s\n", id, token);
                    printf("Consumer %d: %s\n", id, token);
                }
                token = strtok(NULL, " \t\n");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <file1> <file2> ...\n", argv[0]);
        return 1;
    }
    num_files = argc - 1;
    int producers = num_files;
    int consumers = 2;
    int i;
    for (i = 0; i < num_files; i++) {
        filenames[i] = argv[i + 1];
    }

    outFile = fopen("output.txt", "w");
    if (outFile == NULL) {
        printf("Error: Cannot create output.txt\n");
        return 1;
    }

    omp_init_lock(&lock);

    #pragma omp parallel num_threads(producers + consumers)
    {
        int id = omp_get_thread_num();
        if (id < producers) {
            producer(id);
        } else {
            consumer(id);
        }
    }

    fclose(outFile);
    omp_destroy_lock(&lock);
    printf("Output written to output.txt\n");
    return 0;
}
