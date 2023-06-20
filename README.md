# piece-table
A simple text buffer using [Piece Table](https://en.wikipedia.org/wiki/Piece_table) as the underlying data structure.

### Usage
```c
#include <stdio.h>
#include "piece_table.h"

int main() {
  piece_table* pt = piece_table_from_string("Hola\nCola\nAapara nee Gola!");
  
  // performing operations
  piece_table_insert(pt, 14, ", Hehe");
  piece_table_remove(pt, 2, 4);
  piece_table_replace(pt, 3, 2, "replaced_string");
  
  // undo & redo operations
  piece_table_undo(pt);
  piece_table_redo(pt);

  // getting character at some position
  printf("charatcter at position 2: %c\n", piece_table_get_char_at(pt, 2));

  // getting line content
  char* line_content = piece_table_get_line(pt, 1);
  printf("line-1 content: %s\n", line_content);
  
  // getting text buffer as string
  char* text_buffer = piece_table_to_string(pt);
  printf("Buffer: %s\n", text_buffer);
  
  // freeing
  free(line_content);  // line_content returned by piece_table_get_line() should be freed!
  free(text_buffer);  // text_buffer returned by piece_table_to_string() should be freed!
  piece_table_free(pt);
  
  return 0;
}
```

### Project Integration
Just keep the header `piece_table.h` in the include path of the project, and the source file `piece_table.c` along with the other source files of the project.
