#if !defined(BASE_COMMANDS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define BASE_COMMANDS_H

#include "gap_buffer.h"
#include "editor.h"

#define COMMAND_SIG(function_name) void function_name(struct Editor *editor) // :CustomCommandFunction

COMMAND_SIG(newline);
COMMAND_SIG(backspace_char);
COMMAND_SIG(move_up);
COMMAND_SIG(move_down);
COMMAND_SIG(move_left);
COMMAND_SIG(move_right);

#endif