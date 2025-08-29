#include "owner_memory.h"
#include <stdio.h>
#include <windows.h>

chunkStatus *head = NULL;
chunkStatus *lastVisited = NULL;
CRITICAL_SECTION lock;

void *break_point0 = NULL;

/*
- find_chunk -> finding the chunk of the requested size or more by the user

-Parameters:
    chunkStatus *head_ptr -> points the first block of heap memory
    unsigned int size -> size requested by the user

- Returns:
    A ptr which points to the block which fits the request
                  or NULL, in case if there is no block to return
*/
chunkStatus *find_chunk(chunkStatus *head_ptr, unsigned int size) {
  chunkStatus *ptr = head_ptr;
  while (ptr) {
    if (ptr->size >= (size + STRUCT_SIZE) && ptr->available == 1)
      return ptr;
    lastVisited = ptr;
    ptr = ptr->next;
  }
  return NULL;
}

/*
- split_chunk -> splits a single big memory block into two chunks.
                The first chunk will be exactly the size requested by the user,
                and the second chunk will take whatever is left over.

- Parameters:
    chunkStatus* ptr  -> pointer to the memory block that needs splitting
    unsigned int size -> the size the user wants for the first chunk

- Returns:
    Nothing. The function updates the memory list directly.
*/
void split_chunk(chunkStatus *ptr, unsigned int size) {
  chunkStatus *new_chunk = (chunkStatus *)((char *)ptr + STRUCT_SIZE + size);
  new_chunk->size = ptr->size - size - STRUCT_SIZE;
  new_chunk->available = 1;
  new_chunk->next = ptr->next;
  new_chunk->prev = ptr;

  if (new_chunk->next)
    new_chunk->next->prev = new_chunk;

  ptr->size = size;
  ptr->available = 0;
  ptr->next = new_chunk;
}

/*
- increase_allocation -> grows the heap by moving the program break forward.
   Basically, it makes more memory available for use.

- Parameters:
    chunkStatus* ptr  -> pointer to the block of memory that’s being worked on
    unsigned int size -> how much extra memory we want

- Returns:
    Nothing. The function just updates the list to reflect the change.
*/
chunkStatus *increase_allocation(chunkStatus *last_visited_ptr,
                                 unsigned int size) {
  SIZE_T mem_size = MULTIPLIER * (size + STRUCT_SIZE);
  void *mem =
      VirtualAlloc(NULL, mem_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!mem)
    return NULL;

  chunkStatus *current_break = (chunkStatus *)mem;
  current_break->size = mem_size - STRUCT_SIZE;
  current_break->available = 0;
  current_break->next = NULL;
  current_break->prev = last_visited_ptr;

  if (last_visited_ptr)
    last_visited_ptr->next = current_break;

  if (current_break->size > size)
    split_chunk(current_break, size);

  return current_break;
}

/*
- merge_chunk_prev -> combines a freed memory chunk with the previous chunk
   if the previous one is also free, to avoid fragmentation.

- Parameters:
     chunkStatus* freed -> pointer to the memory block that’s being freed

- Returns:
     Nothing. The function updates the memory list directly.
*/
void merge_chunk_prev(chunkStatus *freed) {
  chunkStatus *prev = freed->prev;
  if (prev && prev->available == 1) {
    prev->size += freed->size + STRUCT_SIZE;
    prev->next = freed->next;
    if (freed->next)
      freed->next->prev = prev;
  }
}

/*
- merge_chunk_next: combines a freed memory chunk with the next chunk
   if the next one is also free, to reduce fragmentation.

- Parameters:
     chunkStatus* freed -> pointer to the memory block being freed

- Returns:
     Nothing. The function updates the memory list directly.
*/
void merge_chunk_next(chunkStatus *freed) {
  chunkStatus *next = freed->next;
  if (next && next->available == 1) {
    freed->size += next->size + STRUCT_SIZE;
    freed->next = next->next;
    if (next->next)
      next->next->prev = freed;
  }
}

/*
- print_list: prints the entire linked list. Useful for debugging.

- Parameters:
     chunkStatus* headptr -> pointer to the start of the list

- Returns:
     Nothing. Just prints the list.
*/
void print_list(chunkStatus *head_ptr) {
  int i = 0;
  chunkStatus *ptr = head_ptr;
  while (ptr) {
    printf("[%d] ptr: %p\n", i, ptr);
    printf("[%d] ptr->size: %zu\n", i, (size_t)ptr->size);
    printf("[%d] ptr->available: %d\n", i, ptr->available);
    printf("[%d] ptr->next: %p\n", i, ptr->next);
    printf("[%d] ptr->prev: %p\n", i, ptr->prev);
    printf("-----------------------------------------------------\n");
    i++;
    ptr = ptr->next;
  }
}

/*
- owner_malloc -> allocates a block of memory on the heap of the requested size.
   The returned block is always padded to start and end on a word boundary.

- Parameters:
     unsigned int size -> number of bytes to allocate

- Returns:
     A pointer to the allocated memory block, or NULL if allocation fails.
     (Note: unlike the system malloc, you don’t need to set errno.)
*/
void *owner_memory(unsigned int _size) {
  unsigned int size = ALIGN(_size);
  chunkStatus *ptr;

  EnterCriticalSection(&lock);

  if (!head) {
    // First-time allocation
    ptr = increase_allocation(NULL, size);
    if (!ptr) {
      LeaveCriticalSection(&lock);
      return NULL;
    }
    head = ptr;
    break_point0 = ptr;
    LeaveCriticalSection(&lock);
    return ((char *)ptr + STRUCT_SIZE);
  }

  // Find a free chunk
  chunkStatus *free_chunk = find_chunk(head, size);
  if (!free_chunk) {
    free_chunk = increase_allocation(lastVisited, size);
    if (!free_chunk) {
      LeaveCriticalSection(&lock);
      return NULL;
    }
    LeaveCriticalSection(&lock);
    return ((char *)free_chunk + STRUCT_SIZE);
  }

  if (free_chunk->size > size)
    split_chunk(free_chunk, size);

  LeaveCriticalSection(&lock);
  return ((char *)free_chunk + STRUCT_SIZE);
}

/*
- owner_free -> unallocates memory that has been allocated with owner_malloc.

- Parameters:
     void *ptr -> pointer to the first byte of the memory block to free

- Returns:
     0 if the memory was successfully freed, 1 otherwise.
     (Note: the system free doesn’t return errors, but this version does.)
*/
unsigned int owner_free(void *ptr) {
  if (!ptr)
    return 1;

  EnterCriticalSection(&lock);
  chunkStatus *to_free = (chunkStatus *)((char *)ptr - STRUCT_SIZE);

  to_free->available = 1;
  merge_chunk_next(to_free);
  merge_chunk_prev(to_free);

  LeaveCriticalSection(&lock);
  return 0;
}
