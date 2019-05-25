#if !defined(KEY_BINDING_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define KEY_BINDING_H

#include "SDL.h"

#include <map> // @todo make custom hash table implementation

#include "editor.h"
#include "iml_types.h"
#include "base_commands.h"

typedef void (*Command_Function_Ptr)(struct Editor *editor); // :CustomCommandFunction

struct Bind_Helper {
    std::map<std::string, Command_Function_Ptr> bindings;
};

void bind(Editor *editor, std::string key, u16 modifiers, Command_Function_Ptr command_function_ptr);
void load_default_bindings(Editor *editor);
void handle_command(Editor *editor, SDL_Keycode keycode, u16 modifiers);
void handle_command(Editor *editor, u8 mouse_button);

#endif
