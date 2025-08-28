#include "owner_memory.h"

int main() {
  int *arr = (int *)owner_memory(sizeof(int));
  owner_free(arr);
  return 0;
}
