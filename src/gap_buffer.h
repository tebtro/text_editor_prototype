#if !defined(GAP_BUFFER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define GAP_BUFFER_H

#include <iostream>
#include "iml_types.h"

#define DEFAULT_GAP_SIZE 20

struct Gap_Buffer {
    char *point;   // needed? we have a cursor in our main file
    char *buffer;
    char *buffer_end;

    char *gap_start;
    char *gap_end;

    u64 gap_size;


    b32 init_buffer(u64 size);
    void destroy();
    u64 sizeof_buffer();
    u64 sizeof_gap();
    b32 copy_bytes(char *destination, char *source, u64 length);
    void expand_buffer(u64 size);
    void expand_gap(u64 size);

    void move_gap_to_point();

    u64 point_offset();
    void set_point(u64 offset);
    char get_char();
    char next_char();
    char previous_char();
    void insert_char(char c);
    void put_char(char c);
    void replace_char(char c);
    void delete_chars(u64 length);
    void insert_string(char *string, u64 length);

    void print_buffer();
    b32 save_buffer_to_file(FILE *file);
};

Gap_Buffer make_gap_buffer(int gap_size = DEFAULT_GAP_SIZE);
Gap_Buffer make_gap_buffer(FILE *file, int gap_size = DEFAULT_GAP_SIZE);

#endif
