#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct piece_table piece_table;

  /// Piece Table API
  piece_table* piece_table_new();
  piece_table* piece_table_from_string(const char* string);
  bool piece_table_insert(piece_table* table,
                          const unsigned int position,
                          const char* string);
  bool piece_table_remove(piece_table* table,
                          const unsigned int position,
                          const unsigned int length);
  char* piece_table_to_string(const piece_table* table);
  int piece_table_get_length(const piece_table* table);
  char piece_table_get_char_at(const piece_table* table,
                               const unsigned int position);
  char* piece_table_get_slice(const piece_table* table,
                              const unsigned int position,
                              const unsigned int length);
  char* piece_table_get_line(const piece_table* table, const unsigned int line);
  bool piece_table_free(piece_table* table);

  /// Loggers
  void piece_table_log(piece_table* table);

#ifdef __cplusplus
}
#endif

#endif
