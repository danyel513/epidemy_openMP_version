#include "epidemy.h"

// starting simultation
void start_serial_simulation(Person_t* personArray, int n)
{
    int simulation_time = TOTAL_SIMULATION_TIME;
    while(simulation_time--)
    {
        // moving people around
        for(int i=0; i<n; i++)
        {
            movePerson(&personArray[i]);
        }

        // define their status - check for infections and immunity gaining
        for(int i=0; i<n; i++)
        {
            computeFutureStatus(personArray, n, i);
        }

        // update current status
        updateStatus(personArray, n);

        // for debug purpose only: check that people got infected
#ifdef DEBUG
        printPersonArray(personArray, n);
#endif
    }
}

int main(int argc, char *argv[])
{
    // check and save arguments
    checkArguments(argc, argv);

    // read person array from the file and save the max coords from file
    Person_t *personArray = NULL;
    int n;
    personArray = readData(&n);

    // set thread number to 1 - serial (overload the value sent as parameter)
    THREAD_NUMBER = 1;

// for debug purpose only: print the data read from file - check for
#ifdef DEBUG

    printPersonArray(personArray, n);

#endif

// measure the runtime of the serial algorithm
#ifdef SERIAL_MEASUREMENTS

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

#endif

    start_serial_simulation(personArray, n);

#ifdef SERIAL_MEASUREMENTS

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (double) (finish.tv_sec - start.tv_sec);
    elapsed += (double) (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printStats(elapsed, n);

#endif

    writeData(personArray, n);

    free(personArray);
    return 0;
}
