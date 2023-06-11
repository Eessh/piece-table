#include <stdio.h>
#include "piece-table.h"

int main()
{
  piece_table* pt = piece_table_from_string("Hola\nCola\nGola");
  if(!pt)
  {
    printf("Cannot create piece_table!\n");
    return 1;
  }

  piece_table_log(pt);
  if(!piece_table_insert(pt, 14, ", Hehe"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);
  if(!piece_table_insert(pt, 20, ", Hehe"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);
  if(!piece_table_insert(pt, 0, "NEW_SHIT"))
  {
    printf("Unable to insert!\n");
    return 1;
  }
  piece_table_log(pt);

  printf("Full Buffer: %s\n", piece_table_to_string(pt));

  if(!piece_table_free(pt))
  {
    printf("Unable to free piece_tabe!\n");
    return 1;
  }
  return 0;
}
