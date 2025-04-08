#include <stdio.h>
#include <cuda.h>

#define THREADS_PER_BLOCK 512
#define MAX 50000
#define COEFFICIENT 1 // Example coefficient value

// Function to initialize coefficients array
void initialize(int coeffArr[], int size) {
    for (int i = 0; i < size; i++) {
        coeffArr[i] = COEFFICIENT;
    }
}

// Device function to compute power iteratively
__device__ double power(double base, int exp) {
    double result = 1.0;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

// Kernel function to evaluate polynomial terms
__global__ void evaluatePolynomial(double x, int *coeffArr, double *outputTerms, int numCoefficients) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < numCoefficients) {
        double termValue = power(x, idx); // Compute x^degree
        outputTerms[idx] = coeffArr[idx] * termValue; // Multiply by coefficient
    }
}

int main() {
    double x =.9999; // Example x value
    int *coeffArr = new int[MAX];
    int numCoefficients = MAX;

    initialize(coeffArr, numCoefficients);

    // Host memory for output terms
    double *outputTerms = (double *)malloc(numCoefficients * sizeof(double));

    // Device memory pointers
    int *d_coeffArr;
    double *d_outputTerms;

    // Allocate device memory
    cudaMalloc((void **)&d_coeffArr, numCoefficients * sizeof(int));
    cudaMalloc((void **)&d_outputTerms, numCoefficients * sizeof(double));

    // Copy coefficients to device
    cudaMemcpy(d_coeffArr, coeffArr, numCoefficients * sizeof(int), cudaMemcpyHostToDevice);

    // Launch kernel
    int blocks = (numCoefficients + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    evaluatePolynomial<<<blocks, THREADS_PER_BLOCK>>>(x, d_coeffArr, d_outputTerms, numCoefficients);

    // Copy results back to host
    cudaMemcpy(outputTerms, d_outputTerms, numCoefficients * sizeof(double), cudaMemcpyDeviceToHost);

    // Compute final polynomial value on host
    double finalResult = 0;
    for (int i = 0; i < numCoefficients; i++) {
        finalResult += outputTerms[i];
    }

    // Print results
    printf("Final polynomial value: %.2f\n", finalResult);

    // Free memory
    free(outputTerms);
    cudaFree(d_coeffArr);
    cudaFree(d_outputTerms);
    delete[] coeffArr;

    return 0;
}
