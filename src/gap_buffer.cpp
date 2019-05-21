/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <sys/stat.h>

#include "gap_buffer.h"

u32 Gap_Buffer::init_buffer(u64 size) {
    if (buffer) std::free(buffer);

    buffer = (char *) malloc(size);
    if (!buffer) return 0;

    point = buffer;
    buffer_end = buffer + size;

    gap_start = buffer;
    gap_end   = buffer_end;
    
    return 1;
}

void Gap_Buffer::destroy() {
    if (buffer) std::free(buffer);
}

b32 Gap_Buffer::copy_bytes(char *destination, char *source, u64 length) {
    if ((destination == source) || (length == 0))   return true;

    // if we're moving the character toward the fron of the buffer
    if (source > destination) {
        // check that we don't go beyond the buffer
        if ((source + length) >= buffer_end)   return false;

        for (; length > 0; length--) {
            *(destination++) = *(source++);
        }
    } else {
        // To prevent overwriting characters we still
        // need to move, go tho the back and copy forward.
        source += length;
        destination += length;

        for (; length > 0; length--) {
            // decrement first, cause we start one byte beyond where we want
            *(--destination) = *(--source);
        }
    }

    return true;
}

u64 Gap_Buffer::sizeof_buffer() {
    return (buffer_end - buffer) - (gap_end - gap_start);
}

u64 Gap_Buffer::sizeof_gap() {
    return gap_end - gap_start;
}

void Gap_Buffer::expand_buffer(u64 size) {
    if (((buffer_end - buffer) + size) > sizeof_buffer()) {
        char *old_buffer = buffer;

        int new_buffer_size = (buffer_end - buffer) + size + gap_size;

        buffer = (char *) realloc(buffer, new_buffer_size);

        point      += buffer - old_buffer;
        buffer_end += buffer - old_buffer;
        gap_start  += buffer - old_buffer;
        gap_end    += buffer - old_buffer;
    }
}

void Gap_Buffer::expand_gap(u64 size) {
    if (size > sizeof_gap()) {
        size += gap_size;
        expand_buffer(size);
        copy_bytes(gap_end + size, gap_end, buffer_end - gap_end);

        gap_end += size;
        buffer_end += size;
    }
}

void Gap_Buffer::move_gap_to_point() {
    if (point == gap_start) return;
    if (point == gap_end) {
        point = gap_start;
        return;
    }

    // move gap towards the left
    if (point < gap_start) {
        // move the point over by gapsize.
        copy_bytes(point + (gap_end - gap_start), point, gap_start - point);
        gap_end -= (gap_start - point);
        gap_start = point;
    } else {
        // since point is after the gap, find distance
        // between gap_end and point and that's how
        // much we move from gap_end to gap_start
        copy_bytes(gap_start, gap_end, point - gap_end);
        gap_start += point - gap_end;
        gap_end = point;
        point = gap_start;
    }
}

u64 Gap_Buffer::point_offset() {
    if (point > gap_end) {
        return ((point - buffer) - (gap_end - gap_start));
    } else {
        return (point - buffer);
    }
}

void Gap_Buffer::set_point(u64 offset) {
    point = buffer + offset;
    if (point > gap_start) {
        point += gap_end - gap_start;
    }
}

char Gap_Buffer::get_char() {
    if (point == gap_start)   point = gap_end;
    return *point;
}

char Gap_Buffer::next_char() {
    if (point == gap_start) {
        point = gap_end;
        return *point;
    }
    return *(++point);
}

char Gap_Buffer::previous_char() {
    if (point == gap_end)   point = gap_start;
    return *(--point);
}

void Gap_Buffer::insert_char(char c) {
    if (point != gap_start)   move_gap_to_point();
    if (gap_start == gap_end)   expand_gap(1); // @todo should this be a higher value?
    *(gap_start++) = c;
}

void Gap_Buffer::put_char(char c) {
    insert_char(c);
    *point++;
}

void Gap_Buffer::replace_char(char c) {
    if (point == gap_start)   point = gap_end;
    if (point == buffer_end) {
        expand_buffer(1); // @todo should this be a higher value?
        buffer_end++;
    }
    *point = c;
}

void Gap_Buffer::delete_chars(u64 length) {
    if (point != gap_start)   move_gap_to_point();
    gap_end += length;
}

void Gap_Buffer::insert_string(char *string, u64 length) {
    move_gap_to_point();
    if (length > sizeof_gap())   expand_gap(length); // @todo should this be a higher value?
    do {
        put_char(*(string++));
    } while (length--);
}

void Gap_Buffer::print_buffer() {
    char *temp = buffer;
    while (temp < buffer_end) {
        if ((temp >= gap_start) && (temp < gap_end)) {
            std::cout << "_";
            temp++;
        } else {
            std::cout << *(temp++);
        }
    }
    std::cout << std::endl;
}

b32 Gap_Buffer::save_buffer_to_file(FILE *file, u64 bytes) {
    if (!bytes) return true;
    if (point == gap_start) {
        point = gap_end;
    }
    if ((gap_start > point) && (gap_start < (point + bytes)) && (gap_start != gap_end)) {
        if (gap_start - point != fwrite(point, 1, gap_start - point, file)) {
            return false;
        }
        if ((bytes - (gap_start - point)) != fwrite(gap_end, 1, bytes - (gap_start - point), file)) {
            return true;
        }

        return true;
    } else {
        return bytes == fwrite(point, 1, bytes, file);
    }
}

Gap_Buffer Gap_Buffer::make_gap_buffer(int gap_size) {
    Gap_Buffer gap_buffer = {};
    gap_buffer.gap_size = gap_size;
    gap_buffer.init_buffer(gap_size);
    return gap_buffer;
}

Gap_Buffer Gap_Buffer::make_gap_buffer(FILE *file, int gap_size) {
    struct stat file_stat;
    fstat(fileno(file), &file_stat);
    u64 file_size = file_stat.st_size;

    Gap_Buffer gap_buffer = {};
    gap_buffer.init_buffer(file_size + gap_size);

    gap_buffer.move_gap_to_point();
    gap_buffer.expand_gap(file_size);
    u64 amount = fread(gap_buffer.gap_start, 1, file_size, file);
    gap_buffer.gap_start += amount;
    
    return gap_buffer; 
}
