/*Using Fixed chunk partitioning strategy, write a multi-threaded program in C++ for Polynomial Evaluation. */
// compile: g++ -std=c++11 -lpthread -o FP .cpp

// run: ./hello
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

void ParallelPolySum(int coeffArr[], int threadId, int arraySize, int numThreads, double x)
{
    int start = (threadId * arraySize) / numThreads;
    int end = ((threadId + 1) * arraySize) / numThreads;

    if (threadId == numThreads - 1)
        end = arraySize;

    // printf("threadId=%d,arraySize=%d, start = %d, end = %d \n",threadId,arraySize,start,end);

    double answer = 0;

    for (int i = start; i < end; i++)
    {
        double powerX = power(x, i);
        answer += coeffArr[i] * powerX;
    }

    partialSumArr[threadId] = answer;
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
    // start the timer for Serial Execution
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
        threads[id] = thread(ParallelPolySum, coeffArr, id, arraySize, numThreads, x);
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
    cout << "Execution time in seconds = " << elapsed.count() << " seconds \n";

    // Clean up memory
    delete[] threads;
    delete[] coeffArr;
    delete[] partialSumArr;

    return 0;
}
