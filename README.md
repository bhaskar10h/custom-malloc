  ## CUSTOM MALLOC IN C

  This project implements a **custom memory allocator** in C,
  providing functionality similar to `malloc` and `free`.
  It focuses on understanding memory management,
  fragmentation handling, and thread safety.

    
  
  ## Memory Layout
  
  The allocator uses a **doubly linked list of memory chunks**.
  Each chunk stores metadata (`size`, `available`, `prev`, `next`)
  and points to the next chunk.

  ```text
  Heap Memory Layout:

  [HEAD] -> [Chunk 1] -> [Chunk 2] -> [Chunk 3] -> NULL
             | size=50  | size=100 | size=200
             | available=0 | available=1 | available=0
             | prev=NULL   | prev=Chunk1 | prev=Chunk2
             | next=Chunk2 | next=Chunk3 | next=NULL

  Allocation Example:
  1.  Request 30 bytes
      -> Chunk 1 is split:
         [Chunk 1a: size=30, available=0] -> [Chunk 1b: size=20, available=1] -> Chunk 2 ...

  Free Example:
  1.  Free Chunk 1a
      -> Merge with next free chunk 1b
         [Chunk 1: size=50, available=1] -> Chunk 2 ...

  Linked List Representation:

  head
   |
   v
  +-----------+-----------+-----------+
  | size      | available | next      |
  +-----------+-----------+-----------+
  | 50        | 0         | Chunk2    |
  +-----------+-----------+-----------+
  | 100       | 1         | Chunk3    |
  +-----------+-----------+-----------+
  | 200       | 0         | NULL      |
  +-----------+-----------+-----------+

  ```
  ---

  ## Features

  - [x] **Allocate** memory on the heap with `owner_memory`.
  - [x] **Free** memory with `owner_free`.
  - [x] **Split** large memory chunks to satisfy smaller allocation requests.
  - [x] **Merge** adjacent free memory chunks to reduce fragmentation.
  - [x] **Thread-safe** using Windows `CRITICAL_SECTION`.
  - [x] **Debug utilities** to print the memory block list (`print_list`).

  ---

  ## Memory Layout

  The allocator uses a **doubly linked list of memory chunks**.
  Each chunk stores metadata (`size`, `available`, `prev`, `next`) and
  points to the next chunk.

  ---

  ## Installation

   Use **MinGW** or **Visual Studio**:
   1. Option 1: Single Command
      ```bash
      gcc main.c owner_memory.c -o main.exe
      ```
    
  2. Option 2: Seperate Compilation
     ```bash
     gcc -c owner_memory.c
     gcc -c main.c
     gcc main.o owner_memory.o -o main.exe 
     ```
  ---
  
  ## Usage

  ```c
  #include "owner_memory.h"

  int main() {
    int *arr = (int *)owner_memory(sizeof(int));
    owner_free(arr);
    return 0;
  }
  ```
  ---
  
  ## References
  
  - Tsoding Daily [writing My Own Malloc in C](https://www.youtube.com/watch?v=sZ8GJ1TiMdk)
  - Hirsch Daniel [coding malloc in C from Scratch](https://youtu.be/_HLAWI84aFA?si=SgK5fVIbkHZxOzxk)
