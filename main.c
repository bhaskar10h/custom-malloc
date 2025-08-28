#include "owner_memory.h"

int main() {
  int *a = (int *)owner_memory(sizeof(int));
  owner_free(a);
  return 0;
}
