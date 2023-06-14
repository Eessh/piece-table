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
bool remove_slice_between_pieces(piece* starting_piece, piece* ending_piece);
bool remove_piece_from_table(piece_table* table, piece* p);

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

bool remove_slice_between_pieces(piece* starting_piece, piece* ending_piece)
{
  if(!starting_piece)
  {
    return false;
  }

  if(!ending_piece)
  {
    return false;
  }

  piece* p = starting_piece;
  while(p->next != ending_piece)
  {
    p = p->next;
  }
  p->next = NULL;
  p = starting_piece->next;
  starting_piece->next = ending_piece;

  if(!recursively_free_pieces(p))
  {
    return false;
  }

  return true;
}

bool remove_piece_from_table(piece_table* table, piece* p)
{
  if(!table)
  {
    return false;
  }

  if(!p)
  {
    return false;
  }

  piece* temp = table->pieces_head;
  if(temp == p)
  {
    // deleting the head
    table->pieces_head = temp->next;
    piece_free(temp);
    return true;
  }

  while(temp->next != p)
  {
    temp = temp->next;
  }
  temp->next = p->next;
  piece_free(p);

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
  unsigned int new_add_buffer_length = add_buffer_length + string_length;

  if(!table->add_buffer)
  {
    table->add_buffer = strdup(string);
  }
  else
  {
    table->add_buffer =
      (char*)realloc(table->add_buffer, sizeof(char) * new_add_buffer_length);
    memcpy(table->add_buffer + add_buffer_length,
           string,
           sizeof(char) * string_length);
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
    return true;
  }

  if(remaining_offset == 0)
  {
    // insert new ADD buffer piece before current piece
    piece* new_p = piece_new(ADD, add_buffer_length, string_length);
    new_p->next = p;
    if(p == table->pieces_head)
    {
      table->pieces_head = new_p;
      return true;
    }
    piece* temp = table->pieces_head;
    while(temp->next != p)
    {
      temp = temp->next;
    }
    temp->next = new_p;
    return true;
  }

  if(!split_piece_at(p, remaining_offset))
  {
    return false;
  }
  if(!insert_piece_after(piece_new(ADD, add_buffer_length, string_length), p))
  {
    return false;
  }

  return true;
}

bool piece_table_remove(piece_table* table,
                        const unsigned int position,
                        const unsigned int length)
{
  if(!table)
  {
    return false;
  }

  piece* starting_piece = NULL;
  piece* ending_piece = NULL;
  unsigned int starting_piece_offset = 0, ending_piece_offset = 0;

  starting_piece_offset = position;
  piece* p = table->pieces_head;
  while(p)
  {
    if(starting_piece_offset <= p->length)
    {
      break;
    }
    starting_piece_offset -= p->length;
    p = p->next;
  }
  starting_piece = p;

  p = table->pieces_head;
  ending_piece_offset = position + length;
  while(p)
  {
    if(ending_piece_offset <= p->length)
    {
      break;
    }
    ending_piece_offset -= p->length;
    p = p->next;
  }
  ending_piece = p;

  // removal happening in same piece
  if(starting_piece == ending_piece)
  {
    // removal happening in the same piece
    if(starting_piece_offset + length == p->length)
    {
      // we can just change the length of the piece
      starting_piece->length -= length;
      if(starting_piece->length == 0)
      {
        remove_piece_from_table(table, starting_piece);
      }
      return true;
    }
    // we need to split the node at starting_piece_offset+length
    // then adjust the length of the current piece
    if(!split_piece_at(starting_piece, starting_piece_offset + length))
    {
      return false;
    }
    starting_piece->length -= length;
    if(starting_piece->length == 0)
    {
      remove_piece_from_table(table, starting_piece);
    }
    return true;
  }

  // remove all pieces lying between starting, ending pieces
  // adjust length of starting piece
  // adjust starting_position of ending piece
  if(starting_piece->next != ending_piece)
  {
    if(!remove_slice_between_pieces(starting_piece, ending_piece))
    {
      return false;
    }
  }

  starting_piece->length = starting_piece_offset;
  if(starting_piece->length == 0)
  {
    remove_piece_from_table(table, starting_piece);
  }
  ending_piece->start_position = ending_piece_offset;

  return true;
}

char piece_table_get_char_at(const piece_table* table,
                             const unsigned int position)
{
  if(!table)
  {
    return '\0';
  }

  unsigned int remaining_offset = position;
  piece* p = table->pieces_head;
  while(p)
  {
    if(remaining_offset <= p->length)
    {
      return (p->buffer == ORIGINAL
                ? table->original_buffer
                : table->add_buffer)[p->start_position + remaining_offset];
    }
    remaining_offset -= p->length;
    p = p->next;
  }

  return '\0';
}

char* piece_table_get_slice(const piece_table* table,
                            const unsigned int position,
                            const unsigned int length)
{
  if(!table)
  {
    return NULL;
  }

  piece* starting_piece = NULL;
  piece* ending_piece = NULL;
  unsigned int starting_piece_offset = 0, ending_piece_offset = 0;

  starting_piece_offset = position;
  piece* p = table->pieces_head;
  while(p)
  {
    if(starting_piece_offset <= p->length)
    {
      break;
    }
    starting_piece_offset -= p->length;
    p = p->next;
  }
  starting_piece = p;

  p = table->pieces_head;
  ending_piece_offset = position + length;
  while(p)
  {
    if(ending_piece_offset <= p->length)
    {
      break;
    }
    ending_piece_offset -= p->length;
    p = p->next;
  }
  ending_piece = p;

  char* slice = (char*)calloc(length + 1, sizeof(char));
  if(!slice)
  {
    return NULL;
  }

  if(starting_piece == ending_piece)
  {
    memcpy(slice,
           (starting_piece->buffer == ORIGINAL ? table->original_buffer
                                               : table->add_buffer) +
             starting_piece_offset,
           starting_piece->length - starting_piece_offset);
    slice[length] = '\0';
    return slice;
  }

  unsigned int destination_copy_offset = 0;
  memcpy(slice,
         (starting_piece->buffer == ORIGINAL ? table->original_buffer
                                             : table->add_buffer) +
           starting_piece_offset,
         starting_piece->length - starting_piece_offset);

  destination_copy_offset += starting_piece->length - starting_piece_offset;
  p = starting_piece->next;

  while(p != ending_piece)
  {
    char* source =
      p->buffer == ORIGINAL ? table->original_buffer : table->add_buffer;
    memcpy(
      slice + destination_copy_offset, source + p->start_position, p->length);
    destination_copy_offset += p->length;
    p = p->next;
  }

  memcpy(slice + destination_copy_offset,
         (ending_piece->buffer == ORIGINAL ? table->original_buffer
                                           : table->add_buffer) +
           ending_piece->start_position,
         sizeof(char) * ending_piece_offset);

  slice[length] = '\0';
  return slice;
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

int piece_table_get_length(const piece_table* table)
{
  if(!table)
  {
    return -1;
  }

  int length = 0;
  piece* p = table->pieces_head;
  while(p)
  {
    length += p->length;
    p = p->next;
  }

  return length;
}

char* piece_table_get_line(const piece_table* table, const unsigned int line)
{
  if(!table)
  {
    return NULL;
  }

  unsigned int new_line_tokens = 0, line_length = 0;
  char* string = NULL;

  piece* p = table->pieces_head;
  piece* starting_piece = p;
  piece* ending_piece = NULL;
  unsigned int starting_piece_offset = 0, ending_piece_offset = 0;

  bool found_ending_piece = false;
  while(p)
  {
    char* text =
      p->buffer == ORIGINAL ? table->original_buffer : table->add_buffer;
    for(unsigned int i = 0; i < p->length; i++)
    {
      if(text[p->start_position + i] != '\n')
      {
        continue;
      }

      new_line_tokens++;

      if(new_line_tokens == line - 1)
      {
        starting_piece = p;
        starting_piece_offset = i + 1;
      }

      if(new_line_tokens == line)
      {
        ending_piece = p;
        ending_piece_offset = i - 1;
        found_ending_piece = true;
        break;
      }
    }

    if(found_ending_piece)
    {
      break;
    }

    if(p->next)
    {
      p = p->next;
    }
    else
    {
      break;
    }
  }

  if(!found_ending_piece)
  {
    // line is the last line of buffer
    // hence lies in the last piece
    ending_piece = p;
    ending_piece_offset = p->length;
  }

  if(starting_piece == ending_piece)
  {
    // line lies in the same piece
    line_length = ending_piece_offset - starting_piece_offset + 1;
    string = (char*)calloc(line_length + 1, sizeof(char));
    memcpy(string,
           (starting_piece->buffer == ORIGINAL ? table->original_buffer
                                               : table->add_buffer) +
             starting_piece->start_position + starting_piece_offset,
           line_length);
    return string;
  }

  // calculating line length
  line_length += starting_piece->length - starting_piece_offset;
  p = starting_piece->next;
  while(p != ending_piece)
  {
    line_length += p->length;
    p = p->next;
  }
  line_length += ending_piece_offset;

  string = (char*)calloc(line_length + 1, sizeof(char));

  // copying string from starting piece
  memcpy(string,
         (starting_piece->buffer == ORIGINAL ? table->original_buffer
                                             : table->add_buffer) +
           starting_piece->start_position + starting_piece_offset,
         starting_piece->length - starting_piece_offset);

  unsigned int string_copy_offset =
    starting_piece->length - starting_piece_offset;
  p = starting_piece->next;
  while(p != ending_piece)
  {
    memcpy(
      string + string_copy_offset,
      (p->buffer == ORIGINAL ? table->original_buffer : table->add_buffer) +
        p->start_position,
      p->length);
    string_copy_offset += p->length;
    p = p->next;
  }

  // copying string from ending piece
  memcpy(string + string_copy_offset,
         (ending_piece->buffer == ORIGINAL ? table->original_buffer
                                           : table->add_buffer) +
           ending_piece->start_position,
         ending_piece_offset);

  string[line_length] = '\0';
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
