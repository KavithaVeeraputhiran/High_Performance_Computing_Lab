#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

// function for randam gen matrix
void generateMatrix(int n, int **A) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            A[i][j] = rand() % 10;
}

// Function to print matrix
void printMatrix(int n, int **A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}
void printSharedMatrix(int n, int (*A)[n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}

int main() {
    srand(time(NULL));

    int n;
    printf("Enter size of the matrix:");
    scanf("%d",&n);
    int **A = malloc(n * sizeof(int *));
    int **B = malloc(n * sizeof(int *));
    int **C = malloc(n * sizeof(int *));
    int **C_serial=malloc(n*sizeof(int *));
    for(int i=0;i<n;i++)
        C_serial[i]=malloc(n*sizeof(int));

    for (int i = 0; i < n; i++) {
        A[i] = malloc(n * sizeof(int));
        B[i] = malloc(n * sizeof(int));
        C[i] = malloc(n * sizeof(int));
     }

    printf("Matrix size: %dx%d\n", n, n);

    generateMatrix(n, A);
    generateMatrix(n, B);

  //  printf("Matrix A:\n");
//    printMatrix(n, A);
    //printf("\nMatrix B:\n");
  //  printMatrix(n, B);

    struct timespec start, end;
    // ================= SERIAL EXECUTION =================
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C_serial[i][j] = 0;
            for (int k = 0; k < n; k++) {
                if(n<=3){
                printf("Serial: C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                        i, j, i, k, k, j);
                }
                C_serial[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double serial_time =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec) / 1000000;
if(n<=10){

    printf("\nSerial Result Matrix:\n");
    printMatrix(n, C_serial);
}
    // ================= PARALLEL EXECUTION =================
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * n * n, IPC_CREAT | 0666);
    int (*C_parallel)[n] = shmat(shmid, NULL, 0);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            for (int j = 0; j < n; j++) {
                C_parallel[i][j] = 0;
                for (int k = 0; k < n; k++) {
                    if(n<=3){
                        printf("Child %d: C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                            getpid(), i, j, i, k, k, j);
                       }
                        C_parallel[i][j] += A[i][k] * B[k][j];
                }
            }
            exit(0);
        }
    }

    for (int i = 0; i < n; i++)
        wait(NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double parallel_time =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec) / 1000000;
if(n<=10){
    printf("\nParallel Result Matrix:\n");
    printSharedMatrix(n, C_parallel);
}
    printf("Execution Time:\n");
    printf("Serial Time   : %lf seconds\n", serial_time);
    printf("Parallel Time : %lf seconds\n", parallel_time);

    shmdt(C_parallel);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
/*[23bcs011@mepcolinux ex1a]$cc mult.c -std=c99 -o multi
[23bcs011@mepcolinux ex1a]$./multi
Enter size of the matrix:50
Matrix size: 50x50
Execution Time:
Serial Time   : 2.000000 seconds
Parallel Time : 14.000000 seconds
[23bcs011@mepcolinux ex1a]$./multi
Enter size of the matrix:150
Matrix size: 150x150
Execution Time:
Serial Time   : 59.000000 seconds
Parallel Time : 6.000000 seconds
[23bcs011@mepcolinux ex1a]$./multi
Enter size of the matrix:500
Matrix size: 500x500
Execution Time:
Serial Time   : 1805.000000 seconds
Parallel Time : 27.000000 seconds*/