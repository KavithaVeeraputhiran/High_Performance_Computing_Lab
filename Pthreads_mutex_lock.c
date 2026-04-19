#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 20
#define WORD_LEN 50
#define THREADS 3

char buffer[BUFFER_SIZE][WORD_LEN];
char result[BUFFER_SIZE][WORD_LEN];

int count = 0;
int result_count = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;


char *dictionary[] = {"ate", "mat", "cat", "dog", "hello", "world", "hpc"};
int dict_size = sizeof(dictionary) / sizeof(dictionary[0]);

char *test_words[] = {"amat", "tree", "doge", "tree", "Linux", "hello", "car"};
int test_word_count = sizeof(test_words) / sizeof(test_words[0]);


int check_dictionary(const char *word) {
    char lower_word[WORD_LEN];
    for (int i = 0; word[i]; i++) {
        lower_word[i] = tolower(word[i]);
    }
    lower_word[strlen(word)] = '\0'; // Null-terminate the string

    for (int i = 0; i < dict_size; i++) {
        if (strcmp(dictionary[i], lower_word) == 0)
            return 1;
    }
    return 0;
}


void* spell_check(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        if (count == 0) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL); // Exit thread if no words left
        }

        char word[WORD_LEN];
        strcpy(word, buffer[count - 1]);
        count--;

        pthread_mutex_unlock(&mutex);

        int status = check_dictionary(word);

        pthread_mutex_lock(&mutex);
        if (result_count < BUFFER_SIZE) {
            sprintf(result[result_count++], "%s -> %s", word, status ? "Correct" : "Incorrect");
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    pthread_t threads[THREADS];

    // Initialize mutex and condition variable
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Load test words into buffer
    for (int i = 0; i < test_word_count && i < BUFFER_SIZE; i++) {
        strcpy(buffer[count++], test_words[i]);
    }

    /* Create threads */
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, spell_check, NULL);
    }

    /* Signal threads to start processing */
    pthread_cond_broadcast(&cond);

    /* Wait for threads */
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nSpell Check Results:\n");
    for (int i = 0; i < result_count; i++) {
        printf("%s\n", result[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}

/*
[23bcs011@mepcolinux ex6]$cc sem.c -pthread std=c99 -o sem
[23bcs011@mepcolinux ex6]$./sem
Spell Check Results:
car -> Incorrect
hello -> Correct
Linux -> Incorrect
tree -> Incorrect
doge -> Incorrect
tree -> Incorrect
amat -> Incorrect*/