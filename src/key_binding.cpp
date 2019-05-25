/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <sstream>
#include "key_binding.h"
#include "base_commands.h"

/*
  KMOD_NONE     0 (no modifier is applicable)
  KMOD_LSHIFT   the left Shift key is down
  KMOD_RSHIFT   the right Shift key is down
  KMOD_LCTRL    the left Ctrl (Control) key is down
  KMOD_RCTRL    the right Ctrl (Control) key is down
  KMOD_LALT     the left Alt key is down
  KMOD_RALT     the right Alt key is down
  KMOD_LGUI     the left GUI key (often the Windows key) is down
  KMOD_RGUI     the right GUI key (often the Windows key) is down
  KMOD_NUM      the Num Lock key (may be located on an extended keypad) is down
  KMOD_CAPS     the Caps Lock key is down
  KMOD_MODE     the AltGr key is down
  KMOD_CTRL     (KMOD_LCTRL | KMOD_RCTRL)
  KMOD_SHIFT    (KMOD_LSHIFT | KMOD_RSHIFT)
  KMOD_ALT      (KMOD_LALT | KMOD_RALT)
  KMOD_GUI      (KMOD_LGUI | KMOD_RGUI)
  KMOD_RESERVED reserved for future use
*/
std::stringstream modifiers_to_string(u16 modifiers) {
#if 0
    printf("modifiers: \n");
    if (modifiers & KMOD_NONE)     printf("    NONE\n");

    if (modifiers & KMOD_LSHIFT)   printf("    LSHIFT\n");
    if (modifiers & KMOD_RSHIFT)   printf("    RSHIFT\n");
    if (modifiers & KMOD_LCTRL)    printf("    LCTRL\n");
    if (modifiers & KMOD_RCTRL)    printf("    RCTRL\n");
    if (modifiers & KMOD_LALT)     printf("    LALT\n");
    if (modifiers & KMOD_RALT)     printf("    RALT\n");
    if (modifiers & KMOD_LGUI)     printf("    LGUI\n");
    if (modifiers & KMOD_RGUI)     printf("    RGUI\n");

    if (modifiers & KMOD_NUM)      printf("    NUM\n");
    if (modifiers & KMOD_CAPS)     printf("    CAPS\n");
    if (modifiers & KMOD_MODE)     printf("    MODE\n");
    if (modifiers & KMOD_CTRL)     printf("    CTRL\n");
    if (modifiers & KMOD_SHIFT)    printf("    SHIFT\n");
    if (modifiers & KMOD_ALT)      printf("    ALT\n");
    if (modifiers & KMOD_GUI)      printf("    GUI\n");
    if (modifiers & KMOD_RESERVED) printf("    RESERVED\n");
#endif

    Array<char *> modifiers_array = {};
    
    if (modifiers & KMOD_LSHIFT && !(modifiers & KMOD_RSHIFT))   modifiers_array.push("SHIFT"); // printf("    SHIFT\n");
    else if (modifiers & KMOD_RSHIFT && !(modifiers & KMOD_LSHIFT))   modifiers_array.push("SHIFT"); // printf("    SHIFT\n");
    else if (modifiers & KMOD_SHIFT)   modifiers_array.push("SHIFT"); // printf("    SHIFT\n");

    if (modifiers & KMOD_LCTRL && !(modifiers & KMOD_RCTRL))   modifiers_array.push("CTRL"); // printf("    CTRL\n");
    else if (modifiers & KMOD_RCTRL && !(modifiers & KMOD_LCTRL))   modifiers_array.push("CTRL"); // printf("    CTRL\n");
    else if (modifiers & KMOD_CTRL)   modifiers_array.push("CTRL"); // printf("    CTRL\n");

    if (modifiers & KMOD_LALT && !(modifiers & KMOD_RALT))   modifiers_array.push("ALT"); // printf("    ALT\n");
    else if (modifiers & KMOD_RALT && !(modifiers & KMOD_LALT))   modifiers_array.push("ALT"); // printf("    ALT\n");
    else if (modifiers & KMOD_ALT)   modifiers_array.push("ALT"); // printf("    ALT\n");

    std::stringstream ss;
    for (int i = 0; i < modifiers_array.count; i++) {
        ss << modifiers_array.array[i] << "-";
    }
    
    return ss;
}

// @todo no check for lower or upper case, or just convert it to an enum or something
void bind(Editor *editor, std::string key, u16 modifiers, Command_Function_Ptr command_function_ptr) {
    std::stringstream key_binding =  modifiers_to_string(modifiers);
    key_binding << key;
    // printf("bind key: %s\n", key_binding.str().c_str());

    editor->bind_context->bindings[key_binding.str()] = command_function_ptr;
}

void load_default_bindings(Editor *editor) {
    bind(editor, "Backspace", KMOD_NONE, backspace_char);
    bind(editor, "Return", KMOD_NONE, newline);
    bind(editor, "Up",     KMOD_NONE, move_up);
    bind(editor, "Right",  KMOD_NONE, move_right);
    bind(editor, "Down",   KMOD_NONE, move_down);
    bind(editor, "Left",   KMOD_NONE, move_left);

    bind(editor, "H", KMOD_ALT, split_current_window_horizontally);
    bind(editor, "V", KMOD_ALT, split_current_window_vertically);
}

void execute_command(Editor *editor, std::string key) {
    if (editor->bind_context->bindings.size() == 0)   return;
    std::map<std::string, Command_Function_Ptr>::iterator it;
    it = editor->bind_context->bindings.find(key);
    if (it == editor->bind_context->bindings.end())   return;

    Command_Function_Ptr command_function_ptr = it->second;
    if (!command_function_ptr)   return;

    command_function_ptr(editor);
}

void handle_command(Editor *editor, SDL_Keycode keycode, u16 modifiers) {
    if (keycode == SDLK_LSHIFT)       return;
    if (keycode == SDLK_RSHIFT)       return;
    if (keycode == SDLK_LCTRL)        return;
    if (keycode == SDLK_RCTRL)        return;
    if (keycode == SDLK_LALT)         return;
    if (keycode == SDLK_RALT)         return;
    if (keycode == SDLK_LGUI)         return;
    if (keycode == SDLK_RGUI)         return;
    if (keycode == SDLK_NUMLOCKCLEAR) return;
    if (keycode == SDLK_CAPSLOCK)     return;
    if (keycode == SDLK_MODE)         return;

    // printf("handle_command: keycode: %d, keycode name: %s\n", keycode, SDL_GetKeyName(keycode));
    std::stringstream modifiers_string =  modifiers_to_string(modifiers);
    modifiers_string << SDL_GetKeyName(keycode);
    // printf("key: %s\n", modifiers_string.str().c_str());

    
    execute_command(editor, modifiers_string.str());
}
