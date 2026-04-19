#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#include <time.h>
#include<math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>

const int MAX_STRING = 100;

/* Palindrome check function */
int isPalindrome(char str[]) {
    int i = 0, j = strlen(str) - 1;
    while (i < j) {
        if (str[i] != str[j])
            return 0;
        i++;
        j--;
    }
    return 1;
}

int main(void) {
    char message[MAX_STRING];
    int comm_sz;          /* Number of processes */
    int my_rank;          /* Process rank */
    MPI_Status status;

    /* List of strings */
    char *string_list[] = {
        "hello",
        "level",
        "world",
        "madam",
        "computer",
        "radar"
    };
    int list_size = 6;
    clock_t start_time,end_time;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Non-master processes */
    if (my_rank != 0) {

        /* Pick string from list */
        strcpy(message, string_list[my_rank % list_size]);

        if (my_rank % 2 == 1) {
            /* Odd rank → Type 1 (UPPERCASE only) */
            MPI_Send(message, strlen(message) + 1,
                     MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        } else {
            /* Even rank → Type 2 (Palindrome only) */
            MPI_Send(message, strlen(message) + 1,
                     MPI_CHAR, 0, 2, MPI_COMM_WORLD);
        }
    }
    /* Master process */
    else {
        char recv_msg[MAX_STRING];

//      clock_gettime(CLOCK_MONOTONIC, &start);
        start_time = clock();
        for (int q = 1; q < comm_sz; q++) {
            MPI_Recv(recv_msg, MAX_STRING, MPI_CHAR,
                     MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == 1) {
                /* Convert ONLY Type 1 to uppercase */
                for (int i = 0; recv_msg[i]; i++)
                    recv_msg[i] = toupper(recv_msg[i]);

                printf("From process %d | Type 1 | Uppercase: %s\n",
                       status.MPI_SOURCE, recv_msg);
            }

            else if (status.MPI_TAG == 2) {
                    /* No conversion here */
                if (isPalindrome(recv_msg))
                    printf("From process %d | Type 2 | Palindrome: %s\n",
                           status.MPI_SOURCE, recv_msg);
                else
                    printf("From process %d | Type 2 | Not Palindrome: %s\n",
                           status.MPI_SOURCE, recv_msg);
            }
            end_time= clock();
            double time=((double)(end_time-start_time)/CLOCKS_PER_SEC);
         printf("Exec Time : %lf seconds\n",time);

        }
    }

    MPI_Finalize();
    return 0;
}
/*III-A1@cil30:~$ mpicc mpi_ptp.c -o ptp_mpi
III-A1@cil30:~$ mpiexec -n 6 ./ptp_mpi
From process 3 | Type 1 | Uppercase: MADAM
Exec Time : 0.000026 seconds
From process 4 | Type 2 | Not Palindrome: computer
Exec Time : 0.000034 seconds
From process 1 | Type 1 | Uppercase: LEVEL
Exec Time : 0.000053 seconds
From process 5 | Type 1 | Uppercase: RADAR
Exec Time : 0.000130 seconds
From process 2 | Type 2 | Not Palindrome: world
Exec Time : 0.000136 seconds
*/