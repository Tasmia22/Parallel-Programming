
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<math.h>

#define MAX 50000// Degree of Polynomial
#define COEFFICIENT 1
#define TASK_TAG 100
#define TERMINATION_TAG 200
#define REQUEST_TAG 300
#define EPSILON 1e-6

double power(double x, int degree)
{
    if (degree == 0)
        return 1;
    if (degree == 1)
        return x;
    return x * power(x, degree - 1);
}

void initialize(int coeffArr[])
{
    for (int i = 0; i < MAX; i++)
    {
        coeffArr[i] = COEFFICIENT;
    }
}

double sequential(int coeffArr[], double x)
{
    double answer = 0;
    for (int i = 0; i < MAX; i++)
    {
        double powerX = power(x, i);
        answer += coeffArr[i] * powerX;
    }
    return answer;
}

double evaluatePolynomialChunk(int coeffArr[], double x, int start, int end)
{
    double partialSum = 0;
    int k=0;
    for (int i = start; i < end; i++)
    {
        double powerX = power(x, i);
        partialSum += coeffArr[k] * powerX;
        k++;
        
    }
    return partialSum;
}

double master(int numProcs, int chunkSize, double x, int coeffArr[])
{
    int taskCounter = 0;
    double globalSum = 0;
    MPI_Status status;

    // Send initial tasks to all workers to start them off
    for (int i = 1; i < numProcs; i++)
    {
        int start = taskCounter * chunkSize;
        int end = (taskCounter + 1) * chunkSize;
        if (start >= MAX)
        {
            // If no more tasks, send termination immediately
            MPI_Send(NULL, 0, MPI_INT, i, TERMINATION_TAG, MPI_COMM_WORLD);
        }
        else
        {
            if (end > MAX)
                end = MAX; // Prevent overflow
            MPI_Send(coeffArr + start, chunkSize, MPI_INT, i, TASK_TAG, MPI_COMM_WORLD);
            MPI_Send(&x, 1, MPI_DOUBLE, i, TASK_TAG, MPI_COMM_WORLD);
            MPI_Send(&start, 1, MPI_INT, i, TASK_TAG, MPI_COMM_WORLD);
            MPI_Send(&end, 1, MPI_INT, i, TASK_TAG, MPI_COMM_WORLD);
            //printf("Master sent initial chunk [%d, %d] to worker %d\n", start, end, i);
            taskCounter++;
        }
    }

    int activeWorkers = numProcs - 1; // excluded the master
    while (activeWorkers > 0)
    {
        double partialSum = 0.0;

        // Receive either a work request or partial result from any worker
        MPI_Recv(&partialSum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int workerRank = status.MPI_SOURCE;

        if (status.MPI_TAG == TASK_TAG)
        {
            globalSum += partialSum;
            //printf("Master received partial sum %.5f from worker %d\n", partialSum, workerRank);
        }

        // Handle work request from worker
        else if (status.MPI_TAG == REQUEST_TAG)
        {
            int start = taskCounter * chunkSize;
            int end = (taskCounter + 1) * chunkSize;

            if (start < MAX)
            {
                if (end > MAX)
                    end = MAX;
                MPI_Send(coeffArr + start, chunkSize, MPI_INT, workerRank, TASK_TAG, MPI_COMM_WORLD);
                MPI_Send(&x, 1, MPI_DOUBLE, workerRank, TASK_TAG, MPI_COMM_WORLD);
                MPI_Send(&start, 1, MPI_INT, workerRank, TASK_TAG, MPI_COMM_WORLD);
                MPI_Send(&end, 1, MPI_INT, workerRank, TASK_TAG, MPI_COMM_WORLD);
                //printf("Master sent chunk [%d, %d] to worker %d\n", start, end, workerRank);

                taskCounter++;
            }
            else
            {
                MPI_Send(NULL, 0, MPI_INT, workerRank, TERMINATION_TAG, MPI_COMM_WORLD);
                activeWorkers--;
            }
        }
    }

    printf("Final result of polynomial evaluation: %.2f\n", globalSum);
    return globalSum;
}

void worker(int rank, int chunkSize)
{
    MPI_Status status;
    double x, partialSum;
    int start, end;
    int coeffChunk[chunkSize];

    while (1)
    {
        // Receive the initial chunk or wait for more work
        MPI_Recv(coeffChunk, chunkSize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TERMINATION_TAG)
        {
            printf("Worker %d received termination signal\n", rank);
            break;
        }

        MPI_Recv(&x, 1, MPI_DOUBLE, 0, TASK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&start, 1, MPI_INT, 0, TASK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_INT, 0, TASK_TAG, MPI_COMM_WORLD, &status);

        // Evaluate the polynomial for this chunk
        partialSum = evaluatePolynomialChunk(coeffChunk, x, start, end);
        // printf("Worker %d processed chunk [%d, %d] with partial sum = %f\n", rank, start, end, partialSum);

        // Send the partial result back to the master
        MPI_Send(&partialSum, 1, MPI_DOUBLE, 0, TASK_TAG, MPI_COMM_WORLD);

        // After completing work, request more
        MPI_Send(&partialSum, 1, MPI_DOUBLE, 0, REQUEST_TAG, MPI_COMM_WORLD);
    }
}

int main(int argc, char **argv)
{
    int rank, numProcs;
    int chunkSize = 77;
    double x = 0.9999;
    double elapsed_time; /* Time to find, count solutions */

    int coeffArr[MAX];
    initialize(coeffArr);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if (rank == 0)
    {
        // Sequential Time measurement
        elapsed_time = -MPI_Wtime();
        double seqsum = sequential(coeffArr, x);
        printf("Sequential Sum: %f\n", seqsum);
        /* Stop timer */
        elapsed_time += MPI_Wtime();
        printf("Sequential Execution time %8.6f\n", elapsed_time);
        fflush(stdout);
        
        // Parallel Time measurement
        double Parallel_time = -MPI_Wtime();
        double parallelSum = master(numProcs, chunkSize, x, coeffArr);
        Parallel_time += MPI_Wtime();
        printf("Parallel Sum: %f\n", parallelSum);
        printf("Parallel Execution time %8.6f\n", Parallel_time);
        fflush(stdout);
        
        

        if (fabs(seqsum - parallelSum) < EPSILON)
        {
            printf("Correct: Parallel and Sequential Results Match\n");
        }
        else
        {
            printf("Error: Results Do Not Match\n");
        }
    }
    else
    {
        worker(rank, chunkSize);
    }

    MPI_Finalize();
    return 0;
}
