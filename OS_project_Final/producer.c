#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "producer.h"
#include "globals.h"

#define MAX_LINE_LENGTH 256

void *producer(void *arg)
{
    int choice = *(int *)arg;
    BufferSlot *buffer = getBuffer();
    int *in = getInPtr();
    pthread_mutex_t *mutex = getMutex();
    sem_t *empty = getEmptySem();
    ConsumerPriority *priorities = getConsumerPriorities();
    char *input;
    // Open the input file using open()
    switch (choice)
    {
    case 1:
        input = "Story1.txt";
        break;
    case 2:
        input = "Story2.txt";
        break;
    case 3:
        input = "Story3.txt";
        break;
    }
    int fd = open(input, O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    // Initialize priority semaphores for all consumers
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        sem_init(&priorities[i].priority_sem, 0, 0);
    }

    char buf;
    char line[MAX_LINE_LENGTH];
    int line_index = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &buf, 1)) > 0)
    {
        if (buf == '\n' || line_index == MAX_LINE_LENGTH - 1)
        {
            line[line_index] = '\0'; // End current line

            sem_wait(empty);
            pthread_mutex_lock(mutex);

            char *chunk = malloc(strlen(line) + 1);
            strcpy(chunk, line);

            buffer[*in].data = chunk;
            buffer[*in].read_count = 0;

            pthread_mutex_unlock(mutex);

            // Signal the highest priority consumer first
            for (int j = 0; j < NUM_CONSUMERS; j++)
            {
                if (priorities[j].priority == NUM_CONSUMERS)
                {
                    sem_post(&priorities[j].priority_sem);
                    break;
                }
            }

            sleep(1); // Give time for consumers to process
            *in = (*in + 1) % BUFFER_SIZE;

            // Reset line
            line_index = 0;
        }
        else
        {
            if (line_index < MAX_LINE_LENGTH - 1)
            {
                line[line_index++] = buf;
            }
        }
    }

    close(fd);

    // Send termination signal
    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        sem_wait(empty);
        pthread_mutex_lock(mutex);
        buffer[*in].data = NULL;
        buffer[*in].read_count = NUM_CONSUMERS;
        pthread_mutex_unlock(mutex);

        // Signal all consumers to check for termination
        for (int j = 0; j < NUM_CONSUMERS; j++)
        {
            sem_post(&priorities[j].priority_sem);
        }

        *in = (*in + 1) % BUFFER_SIZE;
    }

    return NULL;
}
