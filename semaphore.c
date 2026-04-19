#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5
#define TOTAL_TASKS 30
#define WORKER_COUNT 5

typedef struct {
    int a;
    int b;
    char op;
    int valid;
} task;

static task work_buffer[BUFFER_SIZE];
static int in = 0;
static int out = 0;

static sem_t empty;
static sem_t full;
static pthread_mutex_t mutex;
static FILE *fp;


static int buffer_pop(task *dest)
{
    sem_wait(&full);
    pthread_mutex_lock(&mutex);

    *dest = work_buffer[out];
    out = (out + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&mutex);
    sem_post(&empty);

    return dest->valid;
}

static void buffer_push(const task *src)
{
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    work_buffer[in] = *src;
    in = (in + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&mutex);
    sem_post(&full);
}

typedef struct {
    int id;
} worker_arg_t;

void *worker_thread(void *arg)
{
    worker_arg_t wa = *(worker_arg_t *)arg;
    free(arg);
    int id = wa.id;
    while (1) {
        task t;
        if (!buffer_pop(&t)) break;

        if (id == 5) {
            switch (t.op) {
                case '+': {
                    int r = t.a + t.b;
                    printf("[Thread %d] Addition: %d + %d = %d\n", id, t.a, t.b, r);
                    if (fp) fprintf(fp, "[Thread %d] %d + %d = %d\n", id, t.a, t.b, r);
                    break;
                }
                case '-': {
                    int r = t.a - t.b;
                    printf("[Thread %d] Subtraction: %d - %d = %d\n", id, t.a, t.b, r);
                    if (fp) fprintf(fp, "[Thread %d] %d - %d = %d\n", id, t.a, t.b, r);
                    break;
                }
                case '*': {
                    int r = t.a * t.b;
                    printf("[Thread %d] Multiplication: %d * %d = %d\n", id, t.a, t.b, r);
                    if (fp) fprintf(fp, "[Thread %d] %d * %d = %d\n", id, t.a, t.b, r);
                    break;
                }
                case '/': {
                    if (t.b == 0) {
                        printf("[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                        if (fp) fprintf(fp, "[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                    } else {
                        float r = (float)t.a / t.b;
                        printf("[Thread %d] Division: %d / %d = %.2f\n", id, t.a, t.b, r);
                        if (fp) fprintf(fp, "[Thread %d] %d / %d = %.2f\n", id, t.a, t.b, r);
                    }
                    break;
                }
                default:
                    break;
            }
        } else {
            char myop = (id == 1) ? '+' : (id == 2) ? '-' : (id == 3) ? '*' : '/';
            if (t.op == myop) {
                if (myop == '+') {
                    int r = t.a + t.b;
                    printf("[Thread %d] Addition: %d + %d = %d\n", id, t.a, t.b, r);
                    if (fp) fprintf(fp, "[Thread %d] %d + %d = %d\n", id, t.a, t.b, r);
                } else if (myop == '-') {
                    int r = t.a - t.b;
                    printf("[Thread %d] Subtraction: %d - %d = %d\n", id, t.a, t.b, r);
                    if (fp) fprintf(fp, "[Thread %d] %d - %d = %d\n", id, t.a, t.b, r);
                } else if (myop == '*') {
                    int r = t.a * t.b;
                    printf("[Thread %d] Multiplication: %d * %d = %d\n", id, t.a, t.b, r);
                    if (fp) fprintf(fp, "[Thread %d] %d * %d = %d\n", id, t.a, t.b, r);
                } else { /* '/' */
                    if (t.b == 0) {
                        printf("[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                        if (fp) fprintf(fp, "[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                    } else {
                        float r = (float)t.a / t.b;
                        printf("[Thread %d] Division: %d / %d = %.2f\n", id, t.a, t.b, r);
                        if (fp) fprintf(fp, "[Thread %d] %d / %d = %.2f\n", id, t.a, t.b, r);
                    }
                }
            } else {
                /* If op doesn't match and worker isn't dispatcher, re-enqueue task for others.
                   To avoid busy requeueing causing starvation, re-push at end of buffer. */
                buffer_push(&t);
                /* small yield to avoid tight loop */
                usleep(10000);
            }
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t workers[WORKER_COUNT];
    int i;

    if (sem_init(&empty, 0, BUFFER_SIZE) != 0) {
        perror("sem_init empty");
        return 1;
    }
    if (sem_init(&full, 0, 0) != 0) {
        perror("sem_init full");
        sem_destroy(&empty);
        return 1;
    }
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        sem_destroy(&empty);
        sem_destroy(&full);
        return 1;
    }

    fp = fopen("result_file.txt", "w");
    if (!fp) {
        perror("fopen");
        pthread_mutex_destroy(&mutex);
        sem_destroy(&empty);
        sem_destroy(&full);
        return 1;
    }

    for (i = 0; i < WORKER_COUNT; ++i) {
        worker_arg_t *wa = malloc(sizeof(worker_arg_t));
        if (!wa) { perror("malloc"); return 1; }
        wa->id = i + 1;
        if (pthread_create(&workers[i], NULL, worker_thread, wa) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    char ops[] = {'+','-','*','/'};
    srand((unsigned)time(NULL));

    for (i = 0; i < TOTAL_TASKS; ++i) {
        task t;
        t.valid = 1;
        t.a = rand() % 100;
        t.b = rand() % 50; /* allow zero */
        t.op = ops[rand() % 4];

        buffer_push(&t);
        printf("Server Generated: %d %c %d\n", t.a, t.op, t.b);
        sleep(1);
    }

      for (i = 0; i < WORKER_COUNT; ++i) {
        task term = {0,0,0,0};
        buffer_push(&term);
    }

    for (i = 0; i < WORKER_COUNT; ++i) {
        pthread_join(workers[i], NULL);
    }

    fclose(fp);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}


/*TestCase:
[Thread 3] 42 * 28 = 1176
[Thread 5] 46 * 22 = 1012
[Thread 5] 37 / 44 = 0.84
[Thread 5] 52 - 30 = 22
[Thread 3] 10 * 35 = 350
[Thread 5] 22 - 5 = 17
[Thread 3] 68 * 16 = 1088
[Thread 5] 50 / 4 = 12.50
[Thread 4] 92 / 49 = 1.88
[Thread 5] 75 * 12 = 900
[Thread 4] 29 / 5 = 5.80
[Thread 5] 36 - 3 = 33
[Thread 2] 82 - 40 = 42
[Thread 5] 41 + 42 = 83
[Thread 4] 74 / 4 = 18.50
[Thread 1] 5 + 28 = 33
[Thread 5] 62 * 49 = 3038
[Thread 3] 48 * 1 = 48
[Thread 2] 48 - 43 = 5
[Thread 1] 11 + 20 = 31
[Thread 5] 62 - 49 = 13
[Thread 1] 41 + 37 = 78
[Thread 5] 94 + 20 = 114
[Thread 1] 43 + 13 = 56
[Thread 4] 75 / 38 = 1.97
[Thread 5] 91 / 45 = 2.02
[Thread 1] 63 + 9 = 72
[Thread 4] 54 / 8 = 6.75
[Thread 5] 96 * 8 = 768
[Thread 1] 42 + 19 = 61*/