#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
int **A, **B;
int **sum;
int **product;
int SIZE;
double get_elapsed_ms(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}
void* add_matrices(void* arg)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            sum[i][j] = A[i][j] + B[i][j];
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double *elapsed = malloc(sizeof(double));
    *elapsed = get_elapsed_ms(start, end);
    printf("\n[Thread 1] Matrix Addition Complete.\n");
    pthread_exit(elapsed);
}
void* multiply_matrices(void* arg)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            product[i][j] = 0;
            for (int k = 0; k < SIZE; k++)
            {
                product[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double *elapsed = malloc(sizeof(double));
    *elapsed = get_elapsed_ms(start, end);
    printf("[Thread 2] Matrix Multiplication Complete.\n");
    pthread_exit(elapsed);
}
int main()
{
    pthread_t thread1, thread2;
    double *time1, *time2;
    printf("Enter matrix size (N for NxN): ");
    scanf("%d", &SIZE);
    srand(time(NULL));
    A = malloc(SIZE * sizeof(int *));
    B = malloc(SIZE * sizeof(int *));
    sum = malloc(SIZE * sizeof(int *));
    product = malloc(SIZE * sizeof(int *));
    for (int i = 0; i < SIZE; i++)
    {
        A[i] = malloc(SIZE * sizeof(int));
        B[i] = malloc(SIZE * sizeof(int));
        sum[i] = malloc(SIZE * sizeof(int));
        product[i] = malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++)
        {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }
    pthread_create(&thread1, NULL, add_matrices, NULL);
    pthread_create(&thread2, NULL, multiply_matrices, NULL);
    pthread_join(thread1, (void**)&time1);
    pthread_join(thread2, (void**)&time2);
    printf("\n--- Execution Times ---");
    printf("\nAddition Thread: %.4f ms", *time1);
    printf("\nMultiplication Thread: %.4f ms\n", *time2);
    if (SIZE < 6)
    {
        printf("\n--- Matrix A ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", A[i][j]);
            printf("\n");
        }
        printf("\n--- Matrix B ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", B[i][j]);
            printf("\n");
        }
        printf("\n--- Result of Addition ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", sum[i][j]);
            printf("\n");
        }
        printf("\n--- Result of Multiplication ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", product[i][j]);
            printf("\n");
        }
    }
    else
    {
        printf("\nMatrix size is %dx%d. Skipping matrix display (Size >= 6x6).\n", SIZE, SIZE);
    }
    for (int i = 0; i < SIZE; i++)
    {
        free(A[i]); free(B[i]); free(sum[i]); free(product[i]);
    }
    free(A); free(B); free(sum); free(product);
    free(time1); free(time2);
    return 0;
}
/*Enter matrix size (N for NxN): 5

[Thread 1] Matrix Addition Complete.
[Thread 2] Matrix Multiplication Complete.

--- Execution Times ---
Addition Thread: 0.0014 ms
Multiplication Thread: 0.0036 ms

--- Matrix A ---
3 2 4 2 4
3 6 0 4 9
4 4 1 8 6
4 1 5 9 7
9 2 3 4 7

--- Matrix B ---
6 3 0 0 2
8 1 7 2 9
9 2 3 2 2
0 7 5 7 4
0 5 4 9 3

--- Result of Addition ---
9 5 4 2 6
11 7 7 6 18
13 6 4 10 8
4 8 10 16 11
9 7 7 13 10

--- Result of Multiplication ---
70 53 52 62 52
66 88 98 121 103
65 104 95 120 96
77 121 95 138 84
97 98 71 101 79*/