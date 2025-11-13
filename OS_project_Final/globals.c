#include <pthread.h>
#include <semaphore.h>
#include "producer.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LOG_ENTRIES 1000

// Define and initialize NUM_CONSUMERS
int NUM_CONSUMERS = 0;

// Function to set number of consumers
void set_num_consumers(int num)
{
    NUM_CONSUMERS = num;
}

// Define arrays with maximum size
BufferSlot buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
pthread_mutex_t mutex;
sem_t sem_empty;
ConsumerPriority consumer_priorities[MAX_CONSUMERS];
char fifo_paths[MAX_CONSUMERS][64];

// Pause control variables
int is_paused = 0;
pthread_mutex_t pause_mutex = PTHREAD_MUTEX_INITIALIZER;

// Logging and performance variables
static LogEntry log_entries[MAX_LOG_ENTRIES];
static int log_count = 0;
static PerformanceStats performance_stats = {0, 0, 0, 0.0};
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

BufferSlot *getBuffer() { return buffer; }
int *getInPtr() { return &in; }
int *getOutPtr() { return &out; }
pthread_mutex_t *getMutex() { return &mutex; }
sem_t *getEmptySem() { return &sem_empty; }
char (*getFifoPaths())[64] { return fifo_paths; }

ConsumerPriority *getConsumerPriorities() { return consumer_priorities; }

void setConsumerPriority(int consumer_id, int priority)
{
    if (consumer_id > 0 && consumer_id <= NUM_CONSUMERS)
    {
        consumer_priorities[consumer_id - 1].priority = priority;
        sem_init(&consumer_priorities[consumer_id - 1].priority_sem, 0, 0);
    }
}

void pause_consumers()
{
    pthread_mutex_lock(&pause_mutex);
    is_paused = 1;
    pthread_mutex_unlock(&pause_mutex);
}

void resume_consumers()
{
    pthread_mutex_lock(&pause_mutex);
    is_paused = 0;
    pthread_mutex_unlock(&pause_mutex);
}

int is_consumer_paused()
{
    int paused;
    pthread_mutex_lock(&pause_mutex);
    paused = is_paused;
    pthread_mutex_unlock(&pause_mutex);
    return paused;
}

// Initialize logging system
void init_logging()
{
    log_count = 0;
    memset(&performance_stats, 0, sizeof(performance_stats));
    memset(log_entries, 0, sizeof(log_entries));
}

// Log an event
void log_event(int consumer_id, const char *event_type, const char *details)
{
    pthread_mutex_lock(&log_mutex);
    
    if (log_count < MAX_LOG_ENTRIES)
    {
        LogEntry *entry = &log_entries[log_count++];
        entry->timestamp = time(NULL);
        entry->consumer_id = consumer_id;
        strncpy(entry->event_type, event_type, sizeof(entry->event_type) - 1);
        strncpy(entry->details, details, sizeof(entry->details) - 1);

        // Update performance stats based on event type
        // Only count events from actual consumers (not system events)
        if (consumer_id != -1) {
            if (strcmp(event_type, "CHUNK_PROCESSED") == 0)
            {
                // Each chunk is processed exactly once by each consumer
                performance_stats.total_chunks_processed++;
            }
            else if (strcmp(event_type, "PAUSE") == 0)
            {
                performance_stats.total_pause_events++;
            }
            else if (strcmp(event_type, "RESUME") == 0)
            {
                performance_stats.total_resume_events++;
            }
        }
    }
    
    pthread_mutex_unlock(&log_mutex);
}

// Print performance statistics
void print_performance_stats()
{
    pthread_mutex_lock(&log_mutex);
    
    printf("\n=== PERFORMANCE STATISTICS ===\n");
    printf("Total chunks processed: %d\n", performance_stats.total_chunks_processed);
    printf("Total pause events: %d\n", performance_stats.total_pause_events);
    printf("Total resume events: %d\n", performance_stats.total_resume_events);
    printf("Average processing time per chunk: %.2f ms\n",
           performance_stats.total_processing_time / 
           (performance_stats.total_chunks_processed ? performance_stats.total_chunks_processed : 1));

    printf("\n=== RECENT EVENTS ===\n");
    int start = (log_count > 10) ? log_count - 10 : 0;
    for (int i = start; i < log_count; i++)
    {
        char time_str[32];
        struct tm *timeinfo = localtime(&log_entries[i].timestamp);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
        
        if (log_entries[i].consumer_id == -1) {
            printf("[%s] SYSTEM: %s - %s\n",
                   time_str,
                   log_entries[i].event_type,
                   log_entries[i].details);
        } else {
            printf("[%s] Consumer %d: %s - %s\n",
                   time_str,
                   log_entries[i].consumer_id,
                   log_entries[i].event_type,
                   log_entries[i].details);
        }
    }
    
    pthread_mutex_unlock(&log_mutex);
}

// Get performance stats
PerformanceStats *get_performance_stats()
{
    return &performance_stats;
}

// Cleanup logging system
void cleanup_logging()
{
    pthread_mutex_destroy(&log_mutex);
}

// Initialize arrays based on NUM_CONSUMERS
void init_arrays()
{
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        sem_init(&consumer_priorities[i].priority_sem, 0, 0);
        consumer_priorities[i].priority = NUM_CONSUMERS - i; // Higher priority for lower IDs
    }
}