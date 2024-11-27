#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define M 6
#define N 9
// Suppose M*N matrix 

void populateMatrix(int matrix[][N])
{

  for(int row = 0; row < M; row++)
  {
    for(int col = 0; col < N; col++)
    {
          matrix[row][col] = 1;
    }
  }
}

int sequentialSum(int matrix[][N])
{
  int row;
  int col;
  int total = 0;

  for(int row = 0; row < M; row++)
  {
    for(int col = 0; col < N; col++)
    {
         total = total +  matrix[row][col];
    }
  }
 return total;
}


int main(int argc, char **argv)
{
  int nprocs;
  int rank;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  double elapsed_time;  /* Time to find, count solutions */
  
  int matrix[M][N];
  populateMatrix(matrix);
     /* Start timer */
  MPI_Barrier (MPI_COMM_WORLD);

  //Sequential Time measurement 
  elapsed_time = - MPI_Wtime();
  int seq_total;
  if(rank == 0)
  {
     seq_total = sequentialSum(matrix);
     printf("sequential sum is %d \n", seq_total);
      /* Stop timer */
     elapsed_time += MPI_Wtime();
     printf ("Sequential Execution time %8.6f\n", elapsed_time);
     fflush(stdout);
  }

  //Parallel Time measurement 
  double Parallel_time = - MPI_Wtime();
  
  int localSum = 0;
  int row,col;

  for(int i = rank; i < M*N; i+=nprocs ){
     row=i/N;
     col=i%N;
     //printf("\n i=%d value of row=%d and col=%d",i, row,col);
     localSum += matrix[row][col];

  }
  
  printf("local Sum by rank = %d is %d \n", rank, localSum);
  fflush(stdout);
  
  int globalSum = 0;
  MPI_Reduce(&localSum, &globalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);  
  
   /* Stop timer */
  Parallel_time  += MPI_Wtime();

  
   if(rank == 0)
   {
      printf("global sum is %d \n", globalSum);
      printf ("Parallel Execution time %8.6f\n", Parallel_time );
      fflush(stdout);
   }

   MPI_Finalize();
  return 0;
}
