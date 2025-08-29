// Defining Defines & Marcos
#define SYSTEM_MALLOC 1
#define STRUCT_SIZE 24
#define MULTIPLIER 10
#define ALIGN_SIZE 8
#define ALIGN(size) (((size) + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1))

typedef struct chunkStatus {
  unsigned int size;
  int available;
  struct chunkStatus *next;
  struct chunkStatus *prev;
  char end[1];
} chunkStatus;

chunkStatus *find_chunk(chunkStatus *head_ptr, unsigned int size);
chunkStatus *increase_allocation(chunkStatus *tail_ptr, unsigned int size);
void split_chunk(chunkStatus *ptr, unsigned int size);
void merge_chunk(chunkStatus *freed);
void print_list(chunkStatus *head_ptr);
void *owner_memory(unsigned int _size);
unsigned int owner_free(void *ptr);
