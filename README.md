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

### API Docs
- ```c
  piece_table* piece_table_new();
  ```
  - Returns a new piece_table.
  - Returns `NULL` if unable to allocate memory.
- ```c
  piece_table* piece_table_from_string(const char* string);
  ```
  - Returns a new piece_table.
  - Returns `NULL` if unable to allocate memory.
- ```c
  bool piece_table_insert(piece_table* pt, const unsigned int position, const char* string);
  ```
  - Inserts the `string` at the `position` of the text buffer.
  - Returns `true` if insert happens successfully.
- ```c
  bool piece_table_remove(piece_table* pt, const unsigned int position, const unsigned int length);
  ```
  - Removes string of `length` starting from `position`.
  - Returns `true` if remove happens successfully.
- ```c
  bool piece_table_replace(piece_table* pt, const unsigned int position, const unsigned int length, const char* string);
  ```
  - Replaces string of length starting from position with given `string`.
  - Returns `true` if replace happens successfully.
- ```c
  bool piece_table_undo(piece_table* pt);
  ```
  - Discards the last change made to the text buffer.
  - Returns `true` if undo happens successfully.
- ```c
  bool piece_table_redo(piece_table* pt);
  ```
  - Discards the last undo operation.
  - Returns `true` if redo happens successfully.
- ```c
  char piece_table_get_char_at(const piece_table* pt, const unsigned int position);
  ```
  - Gives the character at given `position`.
  - Returns character `\0' if `position` is out of bounds.
- ```c
  char* piece_table_get_line(const piece_table* pt, const unsigned int line);
  ```
  - Gives the contents of the `line`.
  - Returns `NULL` if `line` is out of bounds.
- ```c
  char* piece_table_get_slice(const piece_table* pt, const unsigned int position, const unsigned int length);
  ```
  - Gives the contents of text buffer starting from `position` and of length: `length`.
  - Returns `NULL` if `position` or `length` is out of bounds.
- ```c
  int piece_table_get_length(const piece_table* pt);
  ```
  - Gives the length of text buffer.
  - Returns `-1` if `pt` is `NULL`.
- ```c
  char* piece_table_to_string(const piece_table* pt);
  ```
  - Gives the whole text buffer.
  - Returns `NULL` if unable to allocate memory for text buffer to return.
- ```c
  bool piece_table_free(piece_table* pt);
  ```
  - Frees resources used by piece_table.
  - Returns `false` if `pt` is `NULL`.
- ```c
  bool piece_table_log(const piece_table* pt);
  ```
  - Logs the internal moving parts of the text buffer.
  - Returns `false` if `pt` is `NULL`.
