#include "epidemy.h"

// simulation
void start_parallel_simulation(Person_t *p, int n)
{
    int time = TOTAL_SIMULATION_TIME;

    while (time--) // decrementing the total time of the simulation
    {
        // create the threads
        #pragma omp parallel num_threads(THREAD_NUMBER) default(none)
        {
            // assign threads chunks of the array and move persons around
            #pragma omp for schedule(static) chunksize(10)
            for (int i = 0; i < n; i++)
            {
                movePerson(&p[i]);
            }
            // threads will sync here

            // compute the status
            #pragma omp for schedule(static) chunksize(10)
            for (int i = 0; i < n; i++)
            {
                computeFutureStatus(p, n, i);
            }
            //threads will synchronise here

            // update the future status
            #pragma omp for schedule(static) chunksize(10)
            for (int i = 0; i < n; i++)
            {
                p[i].currentStatus = p[i].futureStatus;
            }
        }

// debug purpose only: print array after each round
#ifdef DEBUG
        printPersonArray(p, n);
#endif

    }
}

int main(int argc, char **argv)
{
    // check the given arguments to the program
    checkArguments(argc, argv);

    // read person array from the file and save the max coords from file
    Person_t *personArray = NULL;
    int n;
    personArray = readData(&n);

// for debug purpose only: print the data read from file - check for
#ifdef DEBUG

    printPersonArray(personArray, n);

#endif

    // measure the runtime of the parallel algorithm
#ifdef PARALLEL_MEASUREMENTS

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

#endif

    start_parallel_simulation(personArray, n);

#ifdef PARALLEL_MEASUREMENTS

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (double) (finish.tv_sec - start.tv_sec);
    elapsed += (double) (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printStats(elapsed, n);

#endif

    writeData(personArray, n, 1); // print final data in the file

    free(personArray);
    return 0;
}