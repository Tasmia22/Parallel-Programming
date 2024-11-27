#include <iostream>
#include <cmath>
#include <queue>
#include <mutex>
#include <thread>

using namespace std;

double sum = 0;              
queue<int> taskQueue;        
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

void DecreasingChunks(int threadId, int numThreads)
{
    while (true)
    {
        vector<int> tasksToProcess;

        // Lock the queue for safe access
        
            mtx.lock();

            // If no tasks are left, exit
            if (taskQueue.empty())
            {
                mtx.unlock();
                return;
            }
    
            int taskCount = taskQueue.size();
            int chunkSize = taskCount / numThreads;

            // Ensure at least 1 task is processed
            if (chunkSize == 0) chunkSize=1;
            cout << "ThreadId = " << threadId << ", Fetching " << chunkSize << " tasks" << endl;
            
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

// Driver Code
int main()
{
    int numThreads;
    printf("Enter number of threads: ");
    scanf("%d", &numThreads);
    thread* threads = new thread[numThreads];  

    int N = 40000;  // Number of tasks (i values)
    //int N = 100000;

    for (int i = 0; i <=N; i++)
    {
        taskQueue.push(i);
    }

    // Start the timer for parallel execution
    chrono::time_point<chrono::system_clock> start_time = chrono::system_clock::now();

 
    for (int id = 0; id < numThreads; id++)
    {
        threads[id] = thread(DecreasingChunks, id, numThreads);
    }

    // Wait for all threads to finish
    for (int id = 0; id < numThreads; id++)
    {
        threads[id].join();
    }


    // End the timer for parallel execution
    chrono::time_point<chrono::system_clock> end_time = chrono::system_clock::now();
    chrono::duration<float> elapsed_parallel = end_time - start_time;

    // Output the parallel sum and execution time
    printf("\nParallel Sum = %lf\n", sum);
    cout << "\nExecution time in seconds (Parallel) = " << elapsed_parallel.count() << " seconds \n";


    // Serial Execution 
    double Serialsum = 0.0;

    // Start the timer for serial execution
    start_time = chrono::system_clock::now();
    
    for (int i = 0; i <=N; i++)
    {
        Serialsum += compute(i);
    }

    // End the timer for serial execution
    end_time = chrono::system_clock::now();
    chrono::duration<float> elapsed_serial = end_time - start_time;

    // Output the serial sum and execution time
    printf("\nSerial Sum = %lf", Serialsum);
    printf("\nExecution time in seconds (Serial) = %f seconds\n", elapsed_serial.count());

    delete[] threads;
    return 0;
}
