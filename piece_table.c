#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "piece-table.h"

typedef enum buffer_type
{
  ORIGINAL,
  ADD
} buffer_type;

typedef struct piece
{
  buffer_type buffer;
  unsigned int start_position;
  unsigned int length;

  struct piece* next;
} piece;

struct piece_table
{
  char* original_buffer;
  char* add_buffer;

  piece* pieces_head;
};

/// Piece API
piece* piece_new(const buffer_type buffer,
                 const unsigned int start_position,
                 const unsigned int length);
bool piece_free(piece* p);

/// Helpers
bool recursively_free_pieces(piece* p);
bool insert_piece_after(piece* p, piece* after);
bool split_piece_at(piece* p, const unsigned int offset);

/// Piece API Implementation
piece* piece_new(const buffer_type buffer,
                 const unsigned int start_position,
                 const unsigned int length)
{
  piece* p = (piece*)calloc(1, sizeof(piece));
  if(!p)
  {
    return NULL;
  }

  p->buffer = buffer;
  p->start_position = start_position;
  p->length = length;
  p->next = NULL;

  return p;
}

bool piece_free(piece* p)
{
  if(!p)
  {
    return false;
  }

  free(p);
  return true;
}

/// Helpers Implementation
bool recursively_free_pieces(piece* p)
{
  if(!p)
  {
    return false;
  }

  if(p->next)
  {
    if(!recursively_free_pieces(p->next))
    {
      return false;
    }
  }

  free(p);

  return true;
}

bool insert_piece_after(piece* p, piece* after)
{
  if(!after)
  {
    return false;
  }

  if(!p)
  {
    return false;
  }

  p->next = after->next;
  after->next = p;

  return true;
}

bool split_piece_at(piece* p, const unsigned int offset)
{
  if(!p)
  {
    return false;
  }

  if(!insert_piece_after(piece_new(p->buffer, offset, p->length - offset), p))
  {
    return false;
  }
  p->length = offset;

  return true;
}

/// Piece Table API Implementation
piece_table* piece_table_new()
{
  piece_table* table = (piece_table*)calloc(1, sizeof(piece_table));
  if(!table)
  {
    return NULL;
  }

  table->original_buffer = NULL;
  table->add_buffer = NULL;
  table->pieces_head = NULL;

  return table;
}

piece_table* piece_table_from_string(const char* string)
{
  if(!string)
  {
    return NULL;
  }

  piece_table* table = piece_table_new();
  if(!table)
  {
    return NULL;
  }

  table->original_buffer = strdup(string);
  table->pieces_head = piece_new(ORIGINAL, 0, strlen(string));

  return table;
}

bool piece_table_insert(piece_table* table,
                        const unsigned int position,
                        const char* string)
{
  if(!table)
  {
    return false;
  }

  if(!string)
  {
    return false;
  }

  unsigned int remaining_offset = position;
  piece* p = table->pieces_head;
  while(p)
  {
    if(remaining_offset <= p->length)
    {
      break;
    }
    remaining_offset -= p->length;
    p = p->next;
  }

  unsigned int add_buffer_length =
    table->add_buffer ? strlen(table->add_buffer) : 0;
  unsigned int string_length = strlen(string);
  unsigned int new_add_buffer_length = add_buffer_length + string_length + 1;

  if(!table->add_buffer)
  {
    table->add_buffer = (char*)calloc(string_length + 1, sizeof(char));
    if(!table->add_buffer)
    {
      return false;
    }
    strcat(table->add_buffer, string);
    table->add_buffer[string_length + 1] = '\0';
  }
  else
  {
    table->add_buffer =
      (char*)realloc(table->add_buffer, sizeof(char) * new_add_buffer_length);
    strcat(table->add_buffer, string);
    table->add_buffer[new_add_buffer_length] = '\0';
  }

  // If we are inserting at end of any piece
  if(remaining_offset == p->length)
  {
    // inserting at end
    // just increase the length of the piece
    // p->length += strlen(string);
    // or insert a new piece (works best for undo & redo)
    if(!insert_piece_after(piece_new(ADD, add_buffer_length, string_length), p))
    {
      return false;
    }
  }
  else
  {
    if(!split_piece_at(p, remaining_offset))
    {
      return false;
    }
    if(!insert_piece_after(piece_new(ADD, add_buffer_length, string_length), p))
    {
      return false;
    }
  }

  return true;
}

char* piece_table_to_string(const piece_table* table)
{
  if(!table)
  {
    return NULL;
  }

  unsigned int string_length = 0;
  piece* p = table->pieces_head;
  while(p)
  {
    string_length += p->length;
    p = p->next;
  }
  printf("string_length: %d\n", string_length);

  char* string = (char*)calloc((string_length + 1), sizeof(char));
  if(!string)
  {
    return NULL;
  }

  p = table->pieces_head;
  unsigned int string_back = 0;
  while(p)
  {
    char* source_string =
      p->buffer == ORIGINAL ? table->original_buffer : table->add_buffer;
    memcpy(string + string_back, source_string + p->start_position, p->length);
    string_back += p->length;
    p = p->next;
  }
  string[string_length] = '\0';

  return string;
}

bool piece_table_free(piece_table* table)
{
  if(!table)
  {
    return false;
  }

  free(table->original_buffer);
  free(table->add_buffer);

  if(table->pieces_head && !recursively_free_pieces(table->pieces_head))
  {
    return false;
  }

  free(table);

  return true;
}

/// Loggers Implementation
void piece_table_log(piece_table* table)
{
  if(!table)
  {
    printf(
      "Error: piece_table_log(): can't log piece_table that points to NULL!");
    return;
  }

  printf(
    "Piece Table: {\n\toriginal_buffer: %s,\n\tadd_buffer: %s,\n\tpieces: [",
    table->original_buffer,
    table->add_buffer);

  piece* p = table->pieces_head;
  if(!p)
  {
    printf("]\n}\n");
    return;
  }

  while(p)
  {
    printf("\n\t\t{\n\t\t\tbuffer: %s,\n\t\t\tstart_position: "
           "%d,\n\t\t\tlength: %d\n\t\t}",
           p->buffer == ORIGINAL ? "ORIGINAL" : "ADD",
           p->start_position,
           p->length);
    p = p->next;
  }

  printf("\n\t]\n}\n");
}
