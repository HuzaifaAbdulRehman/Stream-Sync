# Stream-Sync

A multi-threaded Producer-Consumer synchronization system implemented in C, demonstrating concurrent programming concepts using POSIX threads, semaphores, and mutexes.

## Description

Stream-Sync is an Operating Systems project that implements the classic Producer-Consumer problem with multiple consumers. The system reads story files, divides them into chunks, and distributes them across multiple consumer threads for processing. Each consumer displays its output in a separate terminal window using FIFOs (named pipes) for inter-process communication.

### Key Features

- Multi-threaded architecture with configurable number of consumers (1-10)
- Shared buffer synchronization using semaphores and mutexes
- Inter-process communication via FIFOs
- Real-time pause/resume functionality for all consumers
- Performance tracking and logging system
- Priority-based consumer scheduling

## Technologies Used

- **Language**: C
- **Threading**: POSIX Threads (pthreads)
- **Synchronization**: Semaphores, Mutexes
- **IPC**: Named Pipes (FIFOs)
- **Terminal Control**: termios, fcntl
- **Build System**: Make

## Installation

### Prerequisites

- GCC compiler with pthread support
- Linux/Unix-based system (uses gnome-terminal)
- POSIX-compliant environment

### Build Steps

1. Clone the repository:
```bash
git clone https://github.com/HuzaifaAbdulRehman/Stream-Sync.git
cd Stream-Sync
```

2. Navigate to the project directory:
```bash
cd OS_project_Final
```

3. Compile the project:
```bash
make
```

4. Clean build artifacts (optional):
```bash
make clean
```

## Usage

1. Run the compiled executable:
```bash
./stream_sync
```

2. Enter the number of consumers (1-10) when prompted

3. Select a story file (1-3) to process

4. Use the command interface to control execution:
   - Press `p` to pause all consumers
   - Press `r` to resume all consumers
   - Press `q` to quit and view performance statistics

### Example

```bash
$ ./stream_sync
Enter number of consumers (1-10): 3
Enter Choice For Story (1-3) = 1

=== COMMAND INTERFACE ===
Commands:
  'p' - Pause consumers
  'r' - Resume consumers
  'q' - Quit program
=======================
```

## Folder Structure

```
Stream-Sync/
├── OS_project_Final/
│   ├── main.c                      # Main program entry point
│   ├── producer.c                  # Producer thread implementation
│   ├── producer.h                  # Producer header
│   ├── consumer.c                  # Consumer thread implementation
│   ├── consumer.h                  # Consumer header
│   ├── globals.c                   # Global functions and shared resources
│   ├── globals.h                   # Global declarations
│   ├── makefile                    # Build configuration
│   ├── Story1.txt                  # Sample story file 1
│   ├── Story2.txt                  # Sample story file 2
│   ├── Story3.txt                  # Sample story file 3
│   └── OS_Project_Report.docx.pdf  # Project documentation
└── README.md                       # This file
```

## How It Works

### Producer Thread
- Reads story files and divides content into chunks
- Places chunks into a shared circular buffer
- Signals consumers when data is available

### Consumer Threads
- Multiple consumers compete for chunks from the shared buffer
- Each consumer displays output in its own terminal window
- Synchronized access using mutexes and semaphores
- Can be paused/resumed dynamically

### Synchronization Mechanisms
- **Mutex**: Ensures exclusive access to the shared buffer
- **Semaphores**: Manages empty/full buffer slots
- **Priority Semaphores**: Controls consumer priority scheduling

## Performance Analysis

The system tracks and displays:
- Total chunks processed by each consumer
- Processing time statistics
- Pause/resume event counts
- Overall system performance metrics

## Notes

- The project requires a graphical environment for spawning terminal windows
- FIFO files are created in `/tmp/` directory
- Cleanup is handled automatically on exit
- The system uses non-blocking I/O for the command interface

## Credits

**Author**: Huzaifa Abdul Rehman
**Course**: Operating Systems
**Repository**: [Stream-Sync](https://github.com/HuzaifaAbdulRehman/Stream-Sync)

## License

This project is developed for educational purposes as part of an Operating Systems course.
