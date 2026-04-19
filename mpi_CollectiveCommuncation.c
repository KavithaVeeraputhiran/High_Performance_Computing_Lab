#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main() {
    int rank, size;
    int n = 8;           // Total array size
    int key;              // Number to find frequency
    int *array = NULL;    // Full array (only in master)
    int local_n;          // Size of sub-array
    int local_count = 0;
    int total_count = 0;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure the array can be divided evenly
    local_n = n / size;
    int local_array[local_n];

    // Master process input and array creation
    if (rank == 0) {
            scanf("%d",&key);
            array = (int *)malloc(n * sizeof(int));
            printf("Generated Array:\n");
            for (int i = 0; i < n; i++) {
                    array[i] = rand() % 10; // Generate random numbers from 0-9
                    printf("%d ", array[i]);
            }
            printf("key:%d ", key);
        printf("\n");
    }

    // Broadcast the key to all processes
    MPI_Bcast(&key, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter array parts to all processes
    MPI_Scatter(array, local_n, MPI_INT,local_array, local_n, MPI_INT,0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime(); // Start timing

    // Each process counts frequency in its sub-array
    for (int i = 0; i < local_n; i++) {
        if (local_array[i] == key)
            local_count++;
    }

    printf("The array is " );
    for (int i=0;i<local_n;i++)
            printf("%d ",local_array[i]);
    printf(" | ");
    printf(" the frequency of the key is %d  | ",local_count);

    // Reduce all local counts to master process
    MPI_Reduce(&local_count, &total_count, 1,
               MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime(); // End timing

    // Master prints result
    if (rank == 0) {
        printf("Frequency of %d = %d\n", key, total_count);
        printf("Total execution time: %f seconds\n", end_time - start_time);
        free(array);
    }

    // Each process prints its execution time
    printf("Execution time for process %d: %f seconds\n", rank, end_time - start_time);

    MPI_Finalize();
    return 0;

}
/*III-A1@cil30:~$ vi mpi_Collective.c
III-A1@cil30:~$ mpicc mpi_Collective.c -o ex3
III-A1@cil30:~$ mpiexec -np 4 ./ex3
3
Generated Array:
3 6 7 5 3 5 6 2 key:3
The array is 7 5  |  the frequency of the key is 0  | Execution time for process 1: 0.000005 seconds
The array is 3 5  |  the frequency of the key is 1  | Execution time for process 2: 0.000007 seconds
The array is 6 2  |  the frequency of the key is 0  | Execution time for process 3: 0.000007 seconds
The array is 3 6  |  the frequency of the key is 1  | Frequency of 3 = 2
Total execution time: 0.000025 seconds
Execution time for process 0: 0.000025 seconds*/