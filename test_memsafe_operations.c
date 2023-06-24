#include <stdio.h>
#include <stdlib.h>
#include "piece-table.h"

int main()
{
  // Creating piece table from string
  piece_table* pt = piece_table_from_string("Hola\nCola\nGola");
  if(!pt)
  {
    printf("Cannot create piece_table!\n");
    return 1;
  }

  // Temporary char buffer
  char* full_buffer = NULL;

  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // Inserting
  if(!piece_table_insert(pt, 14, ", Hehe"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // MemSafe undo
  if(!piece_table_memsafe_undo(pt))
  {
    printf("Unable to memsafe undo!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // Removing
  if(!piece_table_memsafe_remove(pt, 2, 8))
  {
    printf("Unable to memsafe remove!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // MemSafe undo
  if(!piece_table_memsafe_undo(pt))
  {
    printf("Unable to memsafe undo!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // Replacing
  if(!piece_table_memsafe_replace(pt, 2, 5, "REPLACED_STRING"))
  {
    printf("Unable to memsafe replace!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // MemSafe undo
  if(!piece_table_memsafe_undo(pt))
  {
    printf("Unable to memsafe undo!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // Memsafe redo
  if(!piece_table_memsafe_redo(pt))
  {
    printf("Unable to memsafe redo!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  // MemSafe undo
  if(!piece_table_memsafe_undo(pt))
  {
    printf("Unable to memsafe undo!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  if(!piece_table_free(pt))
  {
    printf("Unable to free piece_table!\n");
    return 1;
  }

  return 0;
}