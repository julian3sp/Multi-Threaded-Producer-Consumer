
# Multi-Threaded Producer-Consumer with Barrier Synchronization  

## Overview  
This program implements a multi-threaded producer-consumer model using `pthread` in C++. The producer reads input from a file and adds data to a shared buffer, while multiple consumer threads process the data. Synchronization is handled using semaphores, and a barrier ensures all consumers finish processing together.  

## Features  
- **Multi-threaded producer-consumer model** using `pthread`.  
- **Semaphore synchronization** to control access to the shared buffer.  
- **Barrier synchronization** to ensure all consumers complete execution.  
- **Configurable buffer size** through command-line arguments.  
- **Reads input from a file** to simulate production.  

## Usage  
### Compilation  
To compile the program, run:  
```sh
gcc -o pcthreads proj3.cpp -pthread
```  

### Running the Program  
```sh
./pcthreads <buffer_size> < testData.txt
```  
- `<buffer_size>`: Size of the shared buffer (optional, defaults to 1).  
- `testData.txt`: Input file containing data for the producer.  

#### Example  
```sh
./pcthreads 5 < testData.txt
```  
Runs the program with a buffer size of **5**, processing input from `testData.txt`.  

## Implementation Details  
- **Producer** reads input and adds data to the buffer.  
- **Consumers** retrieve data and process it.  
- **Semaphores** regulate access to the buffer.  
- **Barrier synchronization** ensures all consumers finish together.  

## File Structure  
```
.
├── proj3.cpp        # Source code for the producer-consumer implementation
├── testData.txt     # Sample input file
├── pcthreads        # Compiled executable
└── README.md        # Project documentation
```  

## Possible Improvements  
- Allow the number of producers/consumers to be specified via command-line arguments.  
- Implement logging for debugging and performance analysis.  
- Optimize synchronization to reduce overhead.  

## Author  
- **Julian Espinal**  
- **julianrespinal24@gmail.com**  
- **[https://github.com/julian3sp/]**  
