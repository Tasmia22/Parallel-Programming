#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

#define MAX 50000
#define COEFFICIENT 1
double *partialSumArr = NULL;

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
    int maxDegree = MAX;
    int i;
    double answer = 0;
    for (i = 0; i < maxDegree; i++)
    {
        double powerX = power(x, i);
        // printf("%f ", powerX);
        answer = answer + coeffArr[i] * powerX;
    }
    return answer;
}

void ParallelCyclicPoly(int coeffArr[], int threadId, int arraySize, int numThreads, double x)
{
    // round-robin method to do findSum
    double localSum = 0;
    for (int i = threadId; i < arraySize; i = i + numThreads)
    {
        double powerX = power(x, i);
        localSum += coeffArr[i] * powerX;
    }
    //printf("threadId = %d, localSum = %f \n", threadId, localSum);

    partialSumArr[threadId] = localSum;
}

int main()
{
    chrono::time_point<chrono::system_clock> start;
    chrono::time_point<chrono::system_clock> end;

    int numThreads;
    printf("Enter number of threads: ");
    scanf("%d", &numThreads);

    thread *threads = new thread[numThreads];
    partialSumArr = new double[numThreads];

    int *coeffArr = new int[MAX];
    initialize(coeffArr);

    double x = 0.9999;
    int arraySize = MAX;
    start = chrono::system_clock::now();
    double Sq = sequential(coeffArr, x);
    /* End timer */
    end = chrono::system_clock::now(); // Record end time

    chrono::duration<float> elapsedS = end - start;
    printf("\nSequential Sum= %f", Sq);
    printf("\nExecution time in seconds For Serial execution= %f seconds", elapsedS.count());

    // start the timer
    start = chrono::system_clock::now();

    for (int id = 0; id < numThreads; id++)
    {
        threads[id] = thread(ParallelCyclicPoly, coeffArr, id, arraySize, numThreads, x);
    }

    // Wait for child threads to finish
    for (int id = 0; id < numThreads; id++)
    {
        threads[id].join();
    }

    // Main thread adds the partial sums to get the final result
    double finalSum = 0;
    for (int id = 0; id < numThreads; id++)
    {
        finalSum += partialSumArr[id];
    }
    /* End timer */
    end = chrono::system_clock::now(); // Record end time

    chrono::duration<float> elapsed = end - start;

    printf("\nParallel Sum by main thread = %lf\n", finalSum);
    cout << "\nExecution time in seconds = " << elapsed.count() << " seconds \n";

    // Clean up memory
    delete[] threads;
    delete[] coeffArr;
    delete[] partialSumArr;

    return 0;
}
