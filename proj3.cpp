#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <queue>
#include <atomic>
#include <vector>
#include <cctype>

using namespace std;

sem_t psem, csem, mutex; // Semaphores for synchronization
sem_t barrier_sem;       // Semaphore for the barrier
sem_t barrier_mutex;     // Mutex for the barrier counter
int barrier_count = 0;   // Counter for the barrier

queue<int> buf; // Queue to act as the buffer
long wait_time_seconds = 0;
long wait_time_useconds = 0; // To measure producer's total wait time
int bufsize = 1;             // Buffer size
int totalConsumerCount = 1;  // Default consumer count
int count = 0;

// Global start time for the entire program
struct timeval global_start, global_end;

bool isNumber(const string &str)
{
    for (char const &c : str)
    {
        if (!isdigit(c))
        {
            return false;
        }
    }
    return true;
}

void *producer(void *arg)
{
    while (true)
    {
        struct timeval start, end;

        int input;
        if (cin >> input)
        {
            gettimeofday(&start, NULL); // Start measuring wait time

            sem_wait(&psem); // Wait for available space in the buffer

            gettimeofday(&end, NULL); // End measuring wait time

            // Calculate the wait time for this round
            long seconds = end.tv_sec - start.tv_sec;
            long microseconds = end.tv_usec - start.tv_usec;

            // Accumulate the total producer wait time
            wait_time_seconds += seconds;
            wait_time_useconds += microseconds;

            // Normalize wait_time_useconds (convert excess microseconds into seconds)
            if (wait_time_useconds >= 1000000)
            {
                wait_time_seconds += wait_time_seconds / 1000000;
                wait_time_useconds = wait_time_useconds % 1000000;
            }

            sem_wait(&mutex); // Lock the mutex to protect the buffer
            if (input != -1)
            {
                buf.push(input); // Place input in the buffer
                sem_post(&mutex);
                cout << "Producer produced: " << input << endl;

                sem_post(&csem); // Signal that an item is in the buffer
            }
            else
            {
                buf.push(-1); // Place -1 in the buffer to signal end
                sem_post(&mutex);
                cout << "Producer produced: " << input << endl;
                // Signal all consumers that production is done
                for (int i = 0; i < totalConsumerCount; i++)
                {
                    sem_post(&csem);
                }
                break;
            }
        }
        else
        {
            cout << "Error reading input!" << endl;
            sem_post(&psem); // semaphore is posted on error
            break;
        }
    }
    return NULL;
}

void barrier(int n)
{
    sem_wait(&barrier_mutex);
    count++;
    if (count == n)
    {
        // Last thread to arrive signals all threads to proceed
        for (int i = 0; i < n; i++)
        {
            sem_post(&barrier_sem);
        }
    }
    sem_post(&barrier_mutex);
    sem_wait(&barrier_sem);
}

void *consumer(void *arg)
{
    int total = 0;
    int consumerID = *((int *)arg); // Retrieve consumer ID from argument

    while (true)
    {
        sem_wait(&csem);  // Wait for values to be available in the buffer
        sem_wait(&mutex); // Lock the mutex to ensure safe access to buffer

        if (!buf.empty())
        {
            int value = buf.front();
            buf.pop();

            if (value != -1)
            {
                cout << "Consumer " << consumerID << " consumed: " << value << endl; // Output the consumed value
                total += value;                                                      // Increment this consumer's total
                sleep(1);                                                            // Sleep for one second
                sem_post(&psem);                                                     // Signal that a value was consumed, releasing space in the buffer
                sem_post(&mutex);                                                    // Unlock the mutex to allow other consumers/producer access
            }
            else
            {                     // -1
                buf.push(-1);     // Push termination signal back to the buffer
                sem_post(&csem);  // Signal the consumer that the termination signal is available
                sem_post(&mutex); // Unlock the mutex
                break;
            }
        }
        else
        {                     // empty buffer
            sem_post(&mutex); // Unlock mutex
        }
    }

    struct timeval end;
    gettimeofday(&end, NULL); // Get the end time for this consumer

    // Calculate elapsed time since the program started
    long elapsed_seconds = end.tv_sec - global_start.tv_sec;
    long elapsed_microseconds = end.tv_usec - global_start.tv_usec;

    // Output total consumed value and completion time for this consumer
    cout << "Consumer " << consumerID << " total: " << total << " (Completed at "
         << elapsed_seconds << " seconds and " << elapsed_microseconds << " microseconds)" << endl;

    // Block at barrier until all consumers are done
    barrier(totalConsumerCount);

    return NULL; // Return from the thread function
}

int main(int argc, char *argv[])
{
    pthread_t idprod;
    vector<pthread_t> idcons(totalConsumerCount);
    vector<int *> consumerIDs(totalConsumerCount);

    if (argc > 1)
    {
        // Check and assign buffer size if provided
        if (isNumber(argv[1]))
        {
            bufsize = atoi(argv[1]);
        }

        // Check and assign consumer number if provided
        if (argc >= 3 && isNumber(argv[2]))
        {
            totalConsumerCount = atoi(argv[2]);
        }
    }
    // limits
    if (bufsize > 32)
    {
        cout << "Exceeded the limit of buffer size" << endl;
        return 1;
    }

    if (totalConsumerCount > 10)
    {
        cout << "Exceeded the limit of consumer number" << endl;
        return 1;
    }

    cout << "Buffer size is: " << bufsize << endl;
    cout << "Consumer number is: " << totalConsumerCount << endl;

    // Initialize semaphores
    if (sem_init(&csem, 0, 0) < 0)
    {
        perror("sem_init for csem");
        exit(1);
    }

    if (sem_init(&psem, 0, bufsize) < 0)
    {
        perror("sem_init for psem");
        exit(1);
    }

    if (sem_init(&mutex, 0, 1) < 0)
    {
        perror("sem_init for mutex");
        exit(1);
    }

    if (sem_init(&barrier_sem, 0, 0) < 0)
    {
        perror("sem_init for barrier_sem");
        exit(1);
    }

    if (sem_init(&barrier_mutex, 0, 1) < 0)
    {
        perror("sem_init for barrier_mutex");
        exit(1);
    }

    // Program time start
    gettimeofday(&global_start, NULL);

    // Create producer thread
    if (pthread_create(&idprod, NULL, producer, NULL) != 0)
    {
        perror("pthread_create");
        exit(1);
    }

    // Create consumer threads
    for (int i = 0; i < totalConsumerCount; i++)
    {
        consumerIDs[i] = new int(i); // Allocate memory
        if (pthread_create(&idcons[i], NULL, consumer, consumerIDs[i]) != 0)
        {
            perror("pthread_create");
            // Clean up already created consumer IDs
            for (int j = 0; j < i; j++)
            {
                delete consumerIDs[j];
            }
            exit(1);
        }
    }

    // Wait for threads to finish
    pthread_join(idprod, NULL);
    for (int i = 0; i < totalConsumerCount; i++)
    {
        pthread_join(idcons[i], NULL);
        delete consumerIDs[i]; // Free the memory allocated for the consumer ID
    }

    // Get the end time for the entire program
    gettimeofday(&global_end, NULL);
    long totalSeconds = global_end.tv_sec - global_start.tv_sec;
    long total_micro = global_end.tv_usec - global_start.tv_usec;

    // Print the total producer wait time
    cout << "Total producer wait time is " << wait_time_seconds << " seconds and "
         << wait_time_useconds << " microseconds." << endl;

    // Print the overall program time
    cout << "Program time is " << totalSeconds << " seconds and "
         << total_micro << " microseconds." << endl;

    // Clean up semaphores and exit
    sem_destroy(&psem);
    sem_destroy(&csem);
    sem_destroy(&mutex);
    sem_destroy(&barrier_sem);
    sem_destroy(&barrier_mutex);

    return 0;
}
