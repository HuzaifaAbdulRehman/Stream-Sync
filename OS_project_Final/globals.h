#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "producer.h"

// Maximum number of consumers allowed
#define MAX_CONSUMERS 10
#define MAX_LOG_ENTRIES 1000

// Number of consumers (will be set at runtime)
extern int NUM_CONSUMERS;

// Structure Definitions
typedef struct {
    int priority;
    sem_t priority_sem;
} ConsumerPriority;

typedef struct {
    int total_chunks_processed;
    int total_pause_events;
    int total_resume_events;
    double total_processing_time;
} PerformanceStats;

typedef struct {
    time_t timestamp;
    int consumer_id;
    char event_type[32];
    char details[256];
} LogEntry;

// Function Declarations
// Buffer Management
BufferSlot* getBuffer();
int* getInPtr();
int* getOutPtr();
pthread_mutex_t* getMutex();
sem_t* getEmptySem();
char (*getFifoPaths())[64];
ConsumerPriority* getConsumerPriorities();
void setConsumerPriority(int consumer_id, int priority);

// Pause Control
void pause_consumers();
void resume_consumers();
int is_consumer_paused();

// Performance and Logging
void init_logging();
void log_event(int consumer_id, const char* event_type, const char* details);
void print_performance_stats();
void cleanup_logging();
PerformanceStats* get_performance_stats();

// Initialization
void init_arrays();
void set_num_consumers(int num);

#endif /* GLOBALS_H */

