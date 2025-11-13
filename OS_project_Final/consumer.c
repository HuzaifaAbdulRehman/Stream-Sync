#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include "consumer.h"
#include "globals.h"

void* consumer(void* arg) 
{
    int id = *(int*)arg;
    BufferSlot* buffer = getBuffer();
    int* out = getOutPtr();
    pthread_mutex_t* mutex = getMutex();
    sem_t* empty = getEmptySem();
    char (*fifo_paths)[64] = getFifoPaths();
    ConsumerPriority* priorities = getConsumerPriorities();
    PerformanceStats* stats = get_performance_stats();
    int was_paused = 0;  // Track if consumer was previously paused

    int fd = open(fifo_paths[id - 1], O_WRONLY);
    if (fd < 0) {
        perror("open FIFO");
        pthread_exit(NULL);
    }

    // Set priority based on consumer ID (lower ID = higher priority)
    setConsumerPriority(id, NUM_CONSUMERS - id + 1);
    char details[256];
    sprintf(details, "Priority set to %d", NUM_CONSUMERS - id + 1);
    log_event(id, "PRIORITY_SET", details);
    dprintf(fd, "\n[Consumer %d] Priority %d Ready To Play\n", 
        id, priorities[id - 1].priority);
    while (1) {
        // Check if paused first
        if (is_consumer_paused()) {
            if (!was_paused) {  // Only log if we weren't already paused
                dprintf(fd, "\n[Consumer %d] PAUSED - Waiting for resume command...\n", id);
                fflush(NULL);
                log_event(id, "PAUSE", "Consumer paused");
                was_paused = 1;
            }
            sleep(1);  // Sleep while paused
            continue;
        } else if (was_paused) {  // We were paused but now resumed
            dprintf(fd, "\n[Consumer %d] RESUMED - Continuing processing...\n", id);
            fflush(NULL);
            log_event(id, "RESUME", "Consumer resumed");
            was_paused = 0;
        }

        // Wait for our turn based on priority
        sem_wait(&priorities[id - 1].priority_sem);
        
        pthread_mutex_lock(mutex);

        BufferSlot* slot = &buffer[*out];
        if (slot->data == NULL) {
            pthread_mutex_unlock(mutex);
            break;
        }
        // Process the chunk
        clock_t start = clock();
        dprintf(fd, "%s",slot->data);
        fflush(NULL);  // Ensure output is written immediately
        slot->read_count++;

        if (slot->read_count >= NUM_CONSUMERS) {
            free(slot->data);
            slot->data = NULL;
            slot->read_count = 0;
            *out = (*out + 1) % BUFFER_SIZE;
            sem_post(empty);
        }

        pthread_mutex_unlock(mutex);
        
        // Calculate processing time and update stats
        clock_t end = clock();
        double processing_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
        stats->total_processing_time += processing_time;
        
        sprintf(details, "Processed chunk in %.2f ms", processing_time);
        log_event(id, "CHUNK_PROCESSED", details);
        
        // Signal next consumer in priority order
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            if (priorities[i].priority == priorities[id - 1].priority - 1) {
                sem_post(&priorities[i].priority_sem);
                break;
            }
        }
        
        usleep(100000);  // Small delay between chunks (100ms)
    }

    dprintf(fd, "[Consumer %d] Exiting.\n", id);
    fflush(NULL);
    close(fd);
    return NULL;
}