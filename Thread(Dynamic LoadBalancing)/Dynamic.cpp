#include <iostream>
#include <cmath>
#include <queue>
#include <mutex>
#include <thread>

using namespace std;

double sum = 0;              // Global sum 
queue<int> taskQueue;        // Shared queue for distributing tasks
mutex mtx;                   


double compute(int i)
{
    int start = i * (i + 1) / 2;
    int finish = start + i;
    double return_val = 0.0;
    for (int j = start; j <= finish; j++)
    {
        return_val += sin(j);
    }
    return return_val;
}


void DynamicTaskDistribution(int threadId, int chunkSize)
{
    while (true)
    {
        vector<int> tasksToProcess;

        // Lock the queue for safe access
        mtx.lock();
        if (taskQueue.empty())
        {
            mtx.unlock();
            return;
        }
        for (int i = 0; i < chunkSize && !taskQueue.empty(); ++i)
        {
            tasksToProcess.push_back(taskQueue.front());
            taskQueue.pop();
        }
        mtx.unlock();

        double localSum = 0.0;
        for (int i = 0; i < tasksToProcess.size(); ++i) {
            int task = tasksToProcess[i];
            localSum += compute(task);
        }
       tasksToProcess.clear(); 
       // cout << "ThreadId = " << threadId << " processed local sum = " << localSum << endl;
        {
            mtx.lock();
            sum += localSum;
            mtx.unlock();    
        }
    }
}

int main()
{
    chrono::time_point<chrono::system_clock> start;
    chrono::time_point<chrono::system_clock> end;
    int numThreads, chunkSize;
    printf("Enter number of threads: ");
    scanf("%d", &numThreads);
    printf("Enter the value of chunk size: ");
    scanf("%d", &chunkSize);

    thread* threads = new thread[numThreads];  

    // int N = 250000;  
    // int N = 200000; 
    int N = 100000; 

    //int N = 40000;  

    // Initialize the queue 
    for (int i = 0; i <= N; i++)
    {
        taskQueue.push(i);
    }

    // Start the timer for parallel execution
     start = chrono::system_clock::now();

 
    for (int id = 0; id < numThreads; id++)
    {
        threads[id] = thread(DynamicTaskDistribution, id, chunkSize);
    }

    // Wait for all threads to finish
    for (int id = 0; id < numThreads; id++)
    {
        threads[id].join();
    }

    // End the timer for parallel execution
    end = chrono::system_clock::now();
    chrono::duration<float> elapsed_parallel = end - start;

    // Output the parallel sum and execution time
    printf("\nParallel Sum = %lf\n", sum);
    cout << "\nExecution time in seconds (Parallel) = " << elapsed_parallel.count() << " seconds \n";
/*
    // Serial Execution (no threads)
    double Serialsum = 0.0;

    // Start the timer for serial execution
    start = chrono::system_clock::now();
    
    // Compute sum serially from 0 to N
    for (int i = 0; i <= N; i++)
    {
        Serialsum += compute(i);
    }

    // End the timer for serial execution
    end = chrono::system_clock::now();
    chrono::duration<float> elapsed_serial = end_time - start_time;

    // Output the serial sum and execution time
    printf("\nSerial Sum = %lf", Serialsum);
    printf("\nExecution time in seconds (Serial) = %f seconds\n", elapsed_serial.count());
*/
    delete[] threads;  // Clean up the thread array
    return 0;
}
