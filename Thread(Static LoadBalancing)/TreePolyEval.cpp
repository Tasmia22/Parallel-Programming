#include <iostream>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <mutex>  

using namespace std;

std::mutex mtx; 
#define MAX 50000
#define COEFFICIENT 1

double result[MAX];
double globalSum = 0;

int chunk;

double power(double x, int degree)
{
    if (degree == 0) return 1;
    if (degree == 1) return x;
    return x * power(x, degree - 1);
}

void initialize(int coeffArr[])
{
    for (int i = 0; i < MAX; i++)
    {
        coeffArr[i] = COEFFICIENT;
    }
}

template<class ItemType>
void ParallelPolyEv(ItemType coeffArr[], int first, int last, int chunkSize, double x)
{
    
    if (first < last)
    {
        if ((last - first) > chunkSize)  // If the chunk is large enough, split further
        {
            //printf("\n Parallel");
            int middle = (first + last) / 2;
            
            // Launch threads to handle left and right portions of the array
            thread left(ParallelPolyEv<ItemType>, coeffArr,  first, middle, chunkSize, x);

            thread right(ParallelPolyEv<ItemType>, coeffArr,  middle + 1, last, chunkSize, x);

            // Wait for both halves to finish
            left.join();
            right.join();
        }
        else // If the chunk is small, calculate linearly
        {
 
           double localSum=0;
           for (int i = first; i <= last; i++)
            {
                double powerX = power(x, i);        
                localSum=localSum+( coeffArr[i] * powerX);
            }
mtx.lock();
globalSum+=localSum;
mtx.unlock();
          
        }
    }
}

int main(int argc, const char * argv[]) {
    chrono::time_point<chrono::system_clock> start;
    chrono::time_point<chrono::system_clock> end;
    int *coeffArr = new int[MAX];
    
    // Initialize the coefficient array
    for (int i = 0; i < MAX; i++) {
        coeffArr[i] = COEFFICIENT;
    }
    
    cout << "Enter chunk size (<= " << MAX << "): ";
    cin >> chunk;
    double x = 0.9999;  // value for polynomial power calculation

    // Measure time for parallel array addition
    start = chrono::system_clock::now();                          // Record start time
    ParallelPolyEv<int>(coeffArr, 0, MAX-1, chunk, x); // Perform parallel addition
    end = chrono::system_clock::now();                            // Record end time
    
    chrono::duration<float> elapsed = end - start;  // Calculate elapsed time
    cout << "\nParallel Array Addition Done\n";
    printf("\nResult is %f",globalSum);
    cout << "\nExecution time in seconds (Parallel) = " << elapsed.count() << "\n";

    
    delete[] coeffArr;
    
    return 0;
}
