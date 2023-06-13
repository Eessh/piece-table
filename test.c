#include <stdio.h>
#include <stdlib.h>
#include "piece-table.h"

int main()
{
  piece_table* pt = piece_table_from_string("Hola\nCola\nGola");
  if(!pt)
  {
    printf("Cannot create piece_table!\n");
    return 1;
  }

  char* full_buffer = NULL;

  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  if(!piece_table_insert(pt, 14, ", Hehe"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  if(!piece_table_insert(pt, 20, ", Hehe"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  if(!piece_table_insert(pt, 0, "NEW_SHIT"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  if(!piece_table_remove(pt, 0, 8))
  {
    printf("Unable to remove!\n");
    return 1;
  }
  piece_table_log(pt);
  full_buffer = piece_table_to_string(pt);
  printf("Full Buffer: %s\n", full_buffer);
  free(full_buffer);

  full_buffer = piece_table_get_slice(pt, 2, 22);
  printf("\nSlice: %s\n", full_buffer);
  free(full_buffer);

  printf("char at 13: %c", piece_table_get_char_at(pt, 13));

  if(!piece_table_free(pt))
  {
    printf("Unable to free piece_tabe!\n");
    return 1;
  }
  return 0;
}
