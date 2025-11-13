#include <stdio.h>
#include <pthread.h>    // For pthreads
#include <stdlib.h>     // For malloc/free
#include <unistd.h>     // For sleep
#include <string.h>     // For string operations
#include <sys/stat.h>   // For mkfifo
#include <termios.h>    // For terminal control
#include <fcntl.h>      // For fcntl means O_NONBLOCK
#include <sys/select.h> // For select
#include "producer.h"   // Must come first to get BufferSlot definition
#include "globals.h"
#include "consumer.h"

#define GLOBAL_EVENT_ID -1 // Special ID for global events

void setup_terminals()
{
    char (*fifo_paths)[64] = getFifoPaths();
    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        sprintf(fifo_paths[i], "/tmp/consumer_fifo_%d", i);
        mkfifo(fifo_paths[i], 0666);

        char cmd[256];
        sprintf(cmd, "gnome-terminal --title=\"Consumer %d\" -- bash -c 'echo \"Consumer %d Ready\"; cat %s; exec bash'",
                i + 1, i + 1, fifo_paths[i]);
        system(cmd);
    }
    sleep(2); // Give more time for terminals to launch
}

void *command_handler(void *arg)
{
    struct termios old_term, new_term;
    int *should_exit = (int *)arg;
    int is_global_paused = 0; // Track global pause state

    // Save old terminal settings
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    // Set stdin to non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    printf("\n=== COMMAND INTERFACE ===\n");
    printf("Commands:\n");
    printf("  'p' - Pause consumers\n");
    printf("  'r' - Resume consumers\n");
    printf("  'q' - Quit program\n");
    printf("=======================\n\n");

    while (!*should_exit)
    {
        // Set up select for 1 second timeout
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

        if (result > 0 && FD_ISSET(STDIN_FILENO, &readfds))
        {
            char c = getchar();
            if (c == '\n' || c == '\r' || c == ' ' || c == '\t')
            {
                continue;
            }

            switch (c)
            {
            case 'p':
                if (!is_global_paused)
                {
                    pause_consumers();
                    printf("=== CONSUMERS PAUSED ===\n");
                    log_event(GLOBAL_EVENT_ID, "PAUSE", "All consumers paused");
                    is_global_paused = 1;
                }
                break;
            case 'r':
                if (is_global_paused)
                {
                    resume_consumers();
                    printf("=== CONSUMERS RESUMED ===\n");
                    log_event(GLOBAL_EVENT_ID, "RESUME", "All consumers resumed");
                    is_global_paused = 0;
                }
                break;
            case 'q':
                *should_exit = 1;
                printf("Quitting...\n");
                // Print performance stats before exiting
                printf("\n=== FINAL PERFORMANCE ANALYSIS ===\n");
                print_performance_stats();
                // Force exit after cleanup
                exit(0);
                break;
            default:
                printf("Invalid command. Use 'p' to pause, 'r' to resume, or 'q' to quit\n");
            }
        }
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    fcntl(STDIN_FILENO, F_SETFL, flags);
    return NULL;
}

int main()
{
    int num_consumers;

    // Get number of consumers from user
    printf("Enter number of consumers (1-10): ");
    scanf("%d", &num_consumers);

    // Validate input
    if (num_consumers < 1 || num_consumers > MAX_CONSUMERS)
    {
        printf("Invalid number of consumers. Please enter a number between 1 and 10.\n");
        return 1;
    }

    // Set number of consumers
    set_num_consumers(num_consumers);

    // Initialize system
    init_logging(); // Initialize logging system
    init_arrays();  // Initialize arrays
    pthread_mutex_init(getMutex(), NULL);
    sem_init(getEmptySem(), 0, BUFFER_SIZE);

    // Create FIFOs and terminals

    // Start threads
    pthread_t prod;
    pthread_t consumers[MAX_CONSUMERS];
    pthread_t cmd_handler;
    int ids[MAX_CONSUMERS];
    int should_exit = 0;
    int all_chunks_processed = 0;
    int Choice = 0;
    do
    {
        printf("Enter Choice For Story (1-3) = ");
        scanf(" %d", &Choice);
        if (Choice < 1 || Choice > 3)
            printf("Invalid Choice\n");
    } while (Choice > 3 || Choice < 1);
    setup_terminals();
    pthread_create(&prod, NULL, producer, &Choice);
    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &ids[i]);
    }
    pthread_create(&cmd_handler, NULL, command_handler, &should_exit);

    // Wait for threads
    pthread_join(prod, NULL);
    all_chunks_processed = 1;
    pthread_join(cmd_handler, NULL);
    for (int i = 0; i < NUM_CONSUMERS; ++i)
        pthread_join(consumers[i], NULL);

    // Print final performance stats
    if (all_chunks_processed)
    {
        printf("\n=== FINAL PERFORMANCE ANALYSIS ===\n");
        print_performance_stats();
    }

    // Cleanup
    for (int i = 0; i < NUM_CONSUMERS; ++i)
        unlink(getFifoPaths()[i]);
    pthread_mutex_destroy(getMutex());
    sem_destroy(getEmptySem());
    cleanup_logging(); // Cleanup logging system

    return 0;
}