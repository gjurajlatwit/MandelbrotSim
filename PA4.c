#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>

int is_prime(int n);

int main(int argc, char** argv){

    int numranks, rank;
    int n=10000000;
    int numprimes = 0;
    //put start and end in one array so that I only had to send one message at a time
    int* start_end = malloc(sizeof(int)*2);
    int num_cycles = 0;
    int count;
    double start_time,end_time;
    

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status stat;

    int buffer_size = n/(numranks-1) ;

    //Master part
    if (rank == 0)
    {
        int finished_workers = 0; 
        start_end[0] = 0;
        start_end[1] =start_end[0]+buffer_size-1;


        start_time = MPI_Wtime();
        //sending an initial range of values to each worker
        for (int i=1; i < numranks;i++)
        {
            //adds to the count of finished workers before we are sending -1 as the starter value
            if (start_end[0] == -1)
                finished_workers++;

            //sending initial data
            MPI_Send(start_end,2,MPI_INT,i,0,MPI_COMM_WORLD);

            //checking if we have any more data to send
            if (start_end[0] != -1)
            {
                start_end[0]+=buffer_size;

                //making sure that end value never exceeds n
                start_end[1] = ((start_end[1] + buffer_size)*(start_end[1] + buffer_size <= n)) + n*(start_end[1] > n);
            }

            //turns start into an invalid int when it exceeds n
            if (start_end[0] > n)
            {
                start_end[0] = -1;
            }
            
        }

        //while there are still ranks working
        while (finished_workers < (numranks-1))
        {
            if (start_end[0] == -1)
                finished_workers++;

            //wait to receive values from anyone
            MPI_Recv(&count,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&stat);

            //add to the running total
            numprimes+= count;

            int worker = stat.MPI_SOURCE;

            MPI_Send(start_end,2,MPI_INT,worker,0,MPI_COMM_WORLD);

            if (start_end[0] != -1)
            {
                start_end[0]+=buffer_size;

                //making sure that end value never exceeds n
                start_end[1] = ((start_end[1] + buffer_size)*(start_end[1] + buffer_size <= n)) + n*(start_end[1] > n);
            }

            //turns start into an invalid int when it exceeds n
            if (start_end[0] > n)
            {
                start_end[0] = -1;
            }

            
        }
    }
    //Worker part
    else
    {
        //receive first start and end points
         MPI_Recv(start_end,2,MPI_INT,0,0,MPI_COMM_WORLD,&stat);

        while (start_end[0] != -1) 
        {
            //keep count
            count = 0;

            //compute number of primes in range
            for (int i = start_end[0]; i <= start_end[1]; i++)
            {
                if (is_prime(i) == 1)
                    count++;
            }

            //send resut back
            MPI_Send(&count,1,MPI_INT,0,0,MPI_COMM_WORLD);
            
            //receive next start and end points
            MPI_Recv(start_end,2,MPI_INT,0,0,MPI_COMM_WORLD,&stat);

            if (start_end[0] == -1)
                break;

        } // check for invalid start and redo
    }
    
    end_time = MPI_Wtime();


    if (rank == 0)
        printf("Number of Primes: %d\n Time in seconds: %f \nNumber of Workers: %d",numprimes,(end_time-start_time),(numranks-1));

    MPI_Finalize();

    free(start_end);

    return 0;
}

int is_prime(int n)
{
    /* handle special cases */
    if      (n == 0) return 0;
    else if (n == 1) return 0;
    else if (n == 2) return 1;

    int i;
    for(i=2;i<=(int)(sqrt((double) n));i++)
        if (n%i==0) return 0;

    return 1;
}
