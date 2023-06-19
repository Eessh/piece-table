# piece-table
Piece Table text buffer library for C/C++ projects.

### Usage
```c
#include <stdio.h>
#include "piece_table.h"

int main() {
  piece_table* pt = piece_table_from_string("Hola\nCola\nAapara nee Gola!");
  
  // performing operations
  piece_table_insert(pt, 14, ", Hehe");
  piece_table_remove(pt, 2, 6);
  piece_table_replace(pt, 3, 2, "replaced_string");
  
  // undo & redo operations
  piece_table_undo(pt);
  piece_table_redo(pt);
  
  // getting text buffer as string
  char* text_buffer = piece_table_to_string(pt);
  printf("Buffer: %s\n", text_buffer);
  
  // freeing
  free(text_buffer);  // text_buffer returned by piece_table_to_string() should be freed!
  piece_table_free(pt);
  
  return 0;
}
```

### Project Integration
Just keep the header `piece_table.h` in the include path of the project, and the source file `piece_table.c` along with the other source files of the project.
