#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "piece-table.h"

typedef enum buffer_type
{
  ORIGINAL,
  ADD
} buffer_type;

typedef enum operation_type
{
  INSERT,
  REMOVE,
  REPLACE
} operation_type;

typedef struct piece
{
  buffer_type buffer;
  unsigned int start_position;
  unsigned int length;

  struct piece* next;
} piece;

typedef struct operation
{
  operation_type type;

  piece* prev_piece;
  piece* start_piece;
  piece* end_piece;
  piece* next_piece;

  struct operation* next;
} operation;

struct piece_table
{
  char* original_buffer;
  char* add_buffer;

  piece* pieces_head;

  operation* undo_stack_top;
  operation* redo_stack_top;
};

/// Piece API
piece* piece_new(const buffer_type buffer,
                 const unsigned int start_position,
                 const unsigned int length);
bool piece_free(piece* p);

/// Operation API
operation* operation_new(const operation_type type,
                         piece* prev_piece,
                         piece* start_piece,
                         piece* end_piece,
                         piece* next_piece);
bool operation_free(operation* op);

/// Helpers
bool recursively_free_pieces(piece* p);
bool insert_piece_after(piece* p, piece* after);
bool split_piece_at(piece* p, const unsigned int offset);
bool remove_slice_between_pieces(piece* starting_piece, piece* ending_piece);
bool remove_piece_from_table(piece_table* table, piece* p);
const char* operation_to_string(const operation* op);
bool push_operation_on_stack(operation** stack_top, operation* op);
bool pop_operation_from_stack(operation** stack_top);
bool move_operation_from_undo_to_redo_stack(piece_table* table);
bool move_operation_from_redo_to_undo_stack(piece_table* table);
bool recursively_free_operation_stack(operation* op);

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

/// Operation API Implementation
operation* operation_new(const operation_type type,
                         piece* prev_piece,
                         piece* start_piece,
                         piece* end_piece,
                         piece* next_piece)
{
  operation* op = (operation*)calloc(1, sizeof(operation));
  if(!op)
  {
    return NULL;
  }

  op->type = type;
  op->prev_piece = prev_piece;
  op->start_piece = start_piece;
  op->end_piece = end_piece;
  op->next_piece = next_piece;
  op->next = NULL;

  return op;
}

bool operation_free(operation* op)
{
  if(!op)
  {
    return false;
  }

  free(op);
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

  if(offset == 0)
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

const char* operation_to_string(const operation* op)
{
  if(!op)
  {
    return NULL;
  }

  switch(op->type)
  {
  case INSERT:
    return "INSERT";
  case REMOVE:
    return "REMOVE";
  case REPLACE:
    return "REPLACE";
  default:
    break;
  }

  return NULL;
}

bool push_operation_on_stack(operation** stack_top, operation* op)
{

  if(!op)
  {
    return false;
  }

  if(!*stack_top)
  {
    *stack_top = op;
    // op->next = NULL;
    return true;
  }

  op->next = *stack_top;
  *stack_top = op;

  return true;
}

bool pop_operation_from_stack(operation** stack_top)
{
  if(!*stack_top)
  {
    return false;
  }

  operation* temp = *stack_top;

  if(temp->next)
  {
    *stack_top = temp->next;
  }
  else
  {
    *stack_top = NULL;
  }

  // in operation is freed, operation on redo stack will
  // point to NULL, resulting in segmentation fault
  // operation_free(temp);

  return true;
}

bool move_operation_from_undo_to_redo_stack(piece_table* table)
{
  if(!table)
  {
    return false;
  }

  if(!table->undo_stack_top)
  {
    return false;
  }

  operation* op = table->undo_stack_top;
  table->undo_stack_top = op->next;

  if(!table->redo_stack_top)
  {
    table->redo_stack_top = op;
    op->next = NULL;
    return true;
  }

  op->next = table->redo_stack_top;
  table->redo_stack_top = op;

  return true;
}

bool move_operation_from_redo_to_undo_stack(piece_table* table)
{
  if(!table)
  {
    return false;
  }

  if(!table->redo_stack_top)
  {
    return false;
  }

  operation* op = table->redo_stack_top;
  table->redo_stack_top = op->next;

  if(!table->undo_stack_top)
  {
    table->undo_stack_top = op;
    op->next = NULL;
    return true;
  }

  op->next = table->undo_stack_top;
  table->undo_stack_top = op;

  return true;
}

bool recursively_free_operation_stack(operation* op)
{
  if(!op)
  {
    return false;
  }

  if(op->next)
  {
    if(!recursively_free_operation_stack(op->next))
    {
      return false;
    }
  }

  operation_free(op);

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
  table->undo_stack_top = NULL;
  table->redo_stack_top = NULL;

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

    // inserting undo operation for this insert
    operation* op = operation_new(INSERT, p, p->next, p->next, p->next->next);
    if(op)
    {
      push_operation_on_stack(&table->undo_stack_top, op);
    }
    else
    {
      printf("Unable to record INSERT operation onto undo stack");
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

      // inserting undo operation for this insert
      operation* op = operation_new(INSERT, NULL, new_p, new_p, p);
      if(op)
      {
        push_operation_on_stack(&table->undo_stack_top, op);
      }
      else
      {
        printf("Unable to record INSERT operation onto undo stack");
      }

      return true;
    }
    piece* temp = table->pieces_head;
    while(temp->next != p)
    {
      temp = temp->next;
    }
    temp->next = new_p;

    // inserting undo operation for this insert
    operation* op = operation_new(INSERT, temp, new_p, new_p, p);
    if(op)
    {
      push_operation_on_stack(&table->undo_stack_top, op);
    }
    else
    {
      printf("Unable to record INSERT operation onto undo stack");
    }

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

  // inserting undo operation for this insert
  operation* op = operation_new(INSERT, p, p->next, p->next, p->next->next);
  if(op)
  {
    push_operation_on_stack(&table->undo_stack_top, op);
  }
  else
  {
    printf("Unable to record INSERT operation onto undo stack");
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
    // removal happening at the end
    if(starting_piece_offset + length == p->length)
    {
      // we can just change the length of the piece
      // does not work for undo & redo
      // starting_piece->length -= length;

      // split piece at starting piece offset
      if(!split_piece_at(starting_piece, starting_piece_offset))
      {
        return false;
      }

      piece* op_starting_piece = starting_piece;
      if(starting_piece->length == 0)
      {
        piece* op_starting_piece = table->pieces_head;
        while(op_starting_piece->next != starting_piece)
        {
          op_starting_piece = op_starting_piece->next;
        }
        remove_piece_from_table(table, starting_piece);
      }

      // inserting undo operation for this remove
      operation* op = operation_new(REMOVE,
                                    op_starting_piece,
                                    op_starting_piece->next,
                                    op_starting_piece->next,
                                    op_starting_piece->next->next);
      if(op)
      {
        push_operation_on_stack(&table->undo_stack_top, op);
      }
      else
      {
        printf("Unable to record REMOVE operation!\n");
      }

      // virtually removing the piece to be removed
      // by connecting starting piece and piece next to removable piece
      op_starting_piece->next = op_starting_piece->next->next;

      return true;
    }

    // we need to split the node at starting_piece_offset+length
    // then adjust the length of the current piece
    // adjusting length, doesn't work for undo & redo
    // we need to split twice at starting_offset, starting_offset+length
    // the virtuall remove the middle piece
    // by connecting the starting end ending pieces among splitted pieces.
    if(!split_piece_at(starting_piece, starting_piece_offset + length))
    {
      return false;
    }
    // starting_piece->length -= length;
    if(starting_piece_offset != 0 &&
       !split_piece_at(starting_piece, starting_piece_offset))
    {
      return false;
    }

    piece* op_starting_piece = starting_piece;
    if(starting_piece_offset == 0 || starting_piece->length == 0)
    {
      // find piece before starting piece
      // it will be the previous piece for the operation
      if(starting_piece == table->pieces_head)
      {
        op_starting_piece = NULL;
      }
      else
      {
        op_starting_piece = table->pieces_head;
        while(op_starting_piece && op_starting_piece->next != starting_piece)
        {
          op_starting_piece = op_starting_piece->next;
        }
        remove_piece_from_table(table, starting_piece);
      }
    }

    operation* op = NULL;
    if(op_starting_piece)
    {
      op = operation_new(REMOVE,
                         op_starting_piece,
                         op_starting_piece->next,
                         op_starting_piece->next,
                         op_starting_piece->next->next);
    }
    else
    {
      op = operation_new(REMOVE,
                         op_starting_piece,
                         starting_piece,
                         starting_piece,
                         starting_piece->next);
    }
    if(op)
    {
      push_operation_on_stack(&table->undo_stack_top, op);
    }
    else
    {
      printf("Unable to record REMOVE operation!\n");
    }

    // virtually removing the piece to be removed
    // by connecting starting piece and piece next to removable piece
    if(op_starting_piece)
    {
      op_starting_piece->next = op_starting_piece->next->next;
    }
    else
    {
      table->pieces_head = starting_piece->next;
    }

    return true;
  }

  // here arise two cases:
  // - starting and ending pieces are next to each other
  // - some piece exist between starting and ending pieces
  // in either case we could just split pieces as:
  // - starting piece at starting piece offset (resulting in 1, 2)
  // - ending piece at ending piece offset (resulting in 3, 4)
  // then virtually remove pieces between: 1, 4
  // this removes complexity of handling pieces between 2, 3

  if(!split_piece_at(starting_piece, starting_piece_offset))
  {
    return false;
  }

  piece* op_starting_piece = starting_piece;
  if(starting_piece->length == 0)
  {
    op_starting_piece = table->pieces_head;
    while(op_starting_piece->next != starting_piece)
    {
      op_starting_piece = op_starting_piece->next;
    }
    remove_piece_from_table(table, starting_piece);
  }

  if(!split_piece_at(ending_piece, ending_piece_offset))
  {
    return false;
  }

  piece* op_ending_piece = ending_piece;
  if(ending_piece->length == 0)
  {
    op_ending_piece = table->pieces_head;
    while(op_ending_piece->next != ending_piece)
    {
      op_ending_piece = op_ending_piece->next;
    }
    remove_piece_from_table(table, ending_piece);
  }

  operation* op = operation_new(REMOVE,
                                op_starting_piece,
                                op_starting_piece->next,
                                op_ending_piece,
                                op_ending_piece->next);
  if(op)
  {
    push_operation_on_stack(&table->undo_stack_top, op);
  }
  else
  {
    printf("Unable to record REMOVE operation!\n");
  }

  // // remove all pieces lying between starting, ending pieces
  // // adjust length of starting piece
  // // adjust starting_position of ending piece
  // if(starting_piece->next != ending_piece)
  // {
  //   if(!remove_slice_between_pieces(starting_piece, ending_piece))
  //   {
  //     return false;
  //   }
  // }

  // starting_piece->length = starting_piece_offset;
  // if(starting_piece->length == 0)
  // {
  //   remove_piece_from_table(table, starting_piece);
  // }
  // ending_piece->start_position = ending_piece_offset;

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

bool piece_table_replace(piece_table* table,
                         const unsigned int position,
                         const unsigned int length,
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

  if(!piece_table_remove(table, position, length))
  {
    return false;
  }

  if(!piece_table_insert(table, position, string))
  {
    return false;
  }

  return true;
}

bool piece_table_undo(piece_table* table)
{
  if(!table)
  {
    return false;
  }

  if(!table->undo_stack_top)
  {
    return false;
  }

  // virtually removing the inserted piece
  // by connecting prev_piece, next_piece

  operation* op = table->undo_stack_top;
  if(!op->prev_piece)
  {
    table->pieces_head = op->next_piece;
  }
  else
  {
    op->prev_piece->next = op->next_piece;
  }

  // if(!push_operation_on_stack(&table->redo_stack_top, table->undo_stack_top))
  // {
  //   return false;
  // }

  // if(!pop_operation_from_stack(&table->undo_stack_top))
  // {
  //   return false;
  // }

  if(!move_operation_from_undo_to_redo_stack(table))
  {
    return false;
  }

  return true;
}

bool piece_table_redo(piece_table* table)
{
  if(!table)
  {
    return false;
  }

  if(!table->redo_stack_top)
  {
    return false;
  }

  // inserting pieces from start_piece to end_piece
  // between prev_piece and next_piece

  operation* op = table->redo_stack_top;
  if(!op->prev_piece)
  {
    table->pieces_head = op->start_piece;
  }
  else
  {
    op->prev_piece->next = op->start_piece;
    op->end_piece->next = op->next_piece;
  }

  if(!move_operation_from_redo_to_undo_stack(table))
  {
    return false;
  }

  return true;
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

  if(table->undo_stack_top &&
     !recursively_free_operation_stack(table->undo_stack_top))
  {
    return false;
  }

  if(table->redo_stack_top &&
     !recursively_free_operation_stack(table->redo_stack_top))
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

  // logging buffers
  printf(
    "Piece Table: {\n\toriginal_buffer: %s,\n\tadd_buffer: %s,\n\tpieces: [",
    table->original_buffer,
    table->add_buffer);

  // logging pieces
  if(!table->pieces_head)
  {
    // printf("]\n}\n");
    printf("],");
    // return;
  }
  else
  {
    piece* p = table->pieces_head;
    while(p)
    {
      printf("\n\t\t{\n\t\t\tbuffer: %s,\n\t\t\tstart_position: "
             "%d,\n\t\t\tlength: %d\n\t\t}",
             p->buffer == ORIGINAL ? "ORIGINAL" : "ADD",
             p->start_position,
             p->length);
      p = p->next;
    }

    // printf("\n\t]\n}\n");
    printf("\n\t],");
  }

  // logging undo stack
  printf("\n\tundo_stack: [");
  if(!table->undo_stack_top)
  {
    printf("],");
  }
  else
  {
    operation* op = table->undo_stack_top;
    while(op)
    {
      if(op->next)
      {
        printf("\n\t\t%s,", operation_to_string(op));
      }
      else
      {
        printf("\n\t\t%s", operation_to_string(op));
      }
      op = op->next;
    }
    printf("\n\t],");
  }

  // logging redo stack
  printf("\n\tredo_stack: [");
  if(!table->redo_stack_top)
  {
    printf("]\n}\n");
  }
  else
  {
    operation* op = table->redo_stack_top;
    while(op)
    {
      if(op->next)
      {
        printf("\n\t\t%s,", operation_to_string(op));
      }
      else
      {
        printf("\n\t\t%s", operation_to_string(op));
      }
      op = op->next;
    }
    printf("\n\t]\n}\n");
  }
}
