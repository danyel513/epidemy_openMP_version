#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

// define the number of threads (there will be THREAD_NUMBER of producers and THREAD_NUMBER of consumers)
#define THREAD_NUMBER 2

#define BUFFER_SIZE 8 // number of blocks from the buffer
#define BLOCK_SIZE 1024 // size of a block -> 8 x 1024 = size of the whole buffer

// define the buffer as a struct
typedef struct {
    char data1[BLOCK_SIZE];  //
    char data2[BLOCK_SIZE];  //  --> 2 chunks of data to be compared by consumers
    int size1; // real size of the data read from file 1
    int size2; // read size of the data read from file 2
    int eof;   // marks the end of the file
} Buffer_t;

// circular buffer
Buffer_t buffer[BUFFER_SIZE];
int in = 0, out = 0; // as a circular buffer we move both the begining and the ending pointer (position)
int count = 0; // the number of the elements saved in buffer

// Mutex lock and the conditional var
pthread_mutex_t lock;
pthread_cond_t notEmpty;
pthread_cond_t notFull;

void initialize(void) // init mutex and variables
{
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&notEmpty, NULL);
    pthread_cond_init(&notFull, NULL);
}

void destroy(void) // destroy mutex and cond. var.
{
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&notEmpty);
    pthread_cond_destroy(&notFull);
}

// FLAGS
int diffFiles = 0; // flag checked by main (signals that differences were found)
int stopProcessing = 0; // flag checked by producers and consumers (signals that consumers found a difference)

// the "routine" functions given as params will be divided in 2:
// 1. the producer threads will read chunks of data from both files and put them in buffer
void *producer(void *args)
{
    FILE **files = (FILE **)args; // unpack the name of the files sent as argument to the function
    FILE *file1 = files[0];
    FILE *file2 = files[1];

    while (1)
    {
        pthread_mutex_lock(&lock);

        // check if consumers found difference
        if (stopProcessing)
        {
            pthread_mutex_unlock(&lock);
            break;
        }

        // wait for consumers to process data in case that the buffer is full
        while (count == BUFFER_SIZE && !stopProcessing)
        {
            pthread_cond_wait(&notFull, &lock); // give the lock so the operating system will know to unlock the resource
                                        // and wait for the consumers to signal that the buffer is not full anymore
        }

        // check again to see if consumers did not find any diffrences while thread was "asleep"
        if (stopProcessing)
        {
            pthread_mutex_unlock(&lock);
            break;
        }

        // save address to the buffer element (pointer to the first element of the buffer)
        Buffer_t *item = &buffer[in];
        item->size1 = (int) fread(item->data1, 1, BLOCK_SIZE, file1); // fread will return the amount of data read
        item->size2 = (int) fread(item->data2, 1, BLOCK_SIZE, file2);

        // if the sizes are different -> the files are different
        if (item->size1 != item->size2)
        {
            diffFiles = 1;
            stopProcessing = 1;
            pthread_cond_signal(&notEmpty); // signal the consumers that the buffer is not empty
            pthread_mutex_unlock(&lock);
            break;
        }

        // if the size of the data read is smaller than the whole size of the block
        // it means that we reached the end of the file
        item->eof = (item->size1 < BLOCK_SIZE) || (item->size2 < BLOCK_SIZE);

        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_cond_signal(&notEmpty); // signal that consumers finished placing data in buffer
        pthread_mutex_unlock(&lock);

        if (item->eof) break; // exit loop if eof was reached
    }

    return NULL;
}

// 2. the consumer threads will take the data from the buffer and compare it bitwise
void *consumer(void *args)
{
    while (1)
    {
        pthread_mutex_lock(&lock);

        // wait untill the buffer is not empty anymore
        while (count == 0 && !stopProcessing)
        {
            pthread_cond_wait(&notEmpty, &lock);
        }

        // check if the threads should stop processing (diffrence found)
        if (stopProcessing)
        {
            pthread_mutex_unlock(&lock);
            break;
        }

        // pointer to the last item of the buffer
        Buffer_t *item = &buffer[out];

        // compare bitwise
        if (memcmp(item->data1, item->data2, item->size1) != 0)
        {
            diffFiles = 1; // flag the diffrence
            stopProcessing = 1;     // flag the end of the work
            pthread_cond_signal(&notFull); // signal the producer that information was taken from buffer and been proccesed
            pthread_mutex_unlock(&lock);
            return NULL;
        }

        if (item->eof)
        {
            pthread_mutex_unlock(&lock);
            break;
        }

        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_cond_signal(&notFull); // Semnalizam ca exista spatiu liber in buffer
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s file1 file2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // open files for reading only
    FILE *file1 = fopen(argv[1], "rb");
    FILE *file2 = fopen(argv[2], "rb");

    if (!file1 || !file2)
    {
        perror("Error opening files");
        exit(EXIT_FAILURE);
    }

    // initialize mutex lock and cond. variables
    initialize();

    // use THREAD_AMOUNT threads on each
    pthread_t prod_thread[THREAD_NUMBER], cons_thread[THREAD_NUMBER];

    // start producer threads
    FILE *files[] = {file1, file2};

    for(int thread=0; thread < THREAD_NUMBER; thread++)
        pthread_create(&prod_thread[thread], NULL, producer, files);

    // start consumer threads
    for(int thread=0; thread < THREAD_NUMBER; thread++)
        pthread_create(&cons_thread[thread], NULL, consumer, NULL);

    // wait for the threads to end tasks
    for(int thread=0; thread < THREAD_NUMBER; thread++)
        pthread_join(prod_thread[thread], NULL);
    for(int thread=0; thread < THREAD_NUMBER; thread++)
        pthread_join(cons_thread[thread], NULL);

    if (diffFiles)
    {
        printf("Files are different.\n");
    }
    else
    {
        printf("Files are identical.\n");
    }

    // close files and destroy mutex and cond var.
    fclose(file1);
    fclose(file2);
    destroy();
    return 0;
}
