#include "epidemy.h"

// simulation

void start_parallel_simulation_v1(Person_t *p, int n) // in the first version is implemented omp parallel for
{
    int time = TOTAL_SIMULATION_TIME;

    while (time--) // decrementing the total time of the simulation
    {
        // create the threads
        #pragma omp parallel num_threads(THREAD_NUMBER) default(none) shared(p, n)
        {
            // assign threads chunks of the array and move persons around
            #pragma omp for schedule(SCHEDULE, CHUNK_SIZE)
            for (int i = 0; i < n; i++)
            {
                movePerson(&p[i]);
            }
            // threads will sync here

            // compute the status
            #pragma omp for schedule(SCHEDULE, CHUNK_SIZE)
            for (int i = 0; i < n; i++)
            {
                computeFutureStatus(p, n, i);
            }
            //threads will synchronise here

            // update the future status
            #pragma omp for schedule(SCHEDULE, CHUNK_SIZE) nowait
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

void start_parallel_simulation_v2(Person_t *p, int n) // in the second version one thread will be creating the tasks
                                                // and the others will solve them
{
    int time = TOTAL_SIMULATION_TIME;

    while (time--) // decrementing the total time of the simulation
    {
        // create the threads
        #pragma omp parallel num_threads(THREAD_NUMBER) default(none) shared(p, n, THREAD_NUMBER)
        {
        #pragma omp single // a single thread will create tasks, others will do work(tasks)
        {
            for (int split = 0; split < THREAD_NUMBER-1; split++) // split the whole array in THREAD_NUMBER-1 pieces
            {
                int start = split * n / (THREAD_NUMBER - 1);
                int end = (split == THREAD_NUMBER-2) ? n : start + n / (THREAD_NUMBER - 1); // assure there are all the items processed

                #pragma omp task firstprivate(start, end) // create tasks on every piece
                // and create a copy of local start and end for every task
                {
                    for (int i = start; i < end; i++)
                    {
                        movePerson(&p[i]);
                    }
                }
            }
            // waiting time is needed for synchronisation between threads and all tasks should be finished
            #pragma omp taskwait

            // compute the status
            for (int split = 0; split < THREAD_NUMBER-1; split++) // split the whole array in THREAD_NUMBER-1 pieces
            {
                int start = split * n / (THREAD_NUMBER - 1);
                int end = (split == THREAD_NUMBER-2) ? n : start + n / (THREAD_NUMBER - 1);; // assure there are all the items processed

                #pragma omp task firstprivate(start, end) // create tasks
                {
                    for (int i = start; i < end; i++)
                    {
                        computeFutureStatus(p, n, i);
                    }
                }
            }

            // sync
            #pragma omp taskwait

            // update the future status
            for (int split = 0; split < THREAD_NUMBER-1; split++) // split the whole array in THREAD_NUMBER-1 pieces
            {
                int start = split * n / (THREAD_NUMBER - 1);
                int end = (split == THREAD_NUMBER-2) ? n : start + n / (THREAD_NUMBER - 1);; // assure there are all the items processed

                #pragma omp task firstprivate(start, end) // create tasks
                {
                    for (int i = start; i < end; i++)
                    {
                        p[i].currentStatus = p[i].futureStatus;
                    }
                }
            }
            // debug purpose only: print array after each round
#ifdef DEBUG
            printPersonArray(p, n);
#endif
        }
        }
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

    // measure the runtime of the 1st parallel algorithm
#ifdef PARALLEL_MEASUREMENTS

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

#endif

    start_parallel_simulation_v1(personArray, n);

#ifdef PARALLEL_MEASUREMENTS

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (double) (finish.tv_sec - start.tv_sec);
    elapsed += (double) (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printStats(elapsed, n, 1);

#endif

    writeData(personArray, n, 1); // print final data in the file

    // measure the runtime of the 2nd parallel algorithm
    personArray = readData(&n);

#ifdef PARALLEL_MEASUREMENTS

    clock_gettime(CLOCK_MONOTONIC, &start);

#endif

    start_parallel_simulation_v2(personArray, n);

#ifdef PARALLEL_MEASUREMENTS

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (double) (finish.tv_sec - start.tv_sec);
    elapsed += (double) (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printStats(elapsed, n, 0);

#endif

    writeData(personArray, n, 2); // print final data in the file

    free(personArray);
    return 0;
}