// commands.hpp
#pragma once
#include <stdint.h>

using CmdFn0 = void(*)();               // no-arg command
struct Command {
    const char* name;                   // e.g., "clear"
    CmdFn0      fn;
};

// forward decls
void print_bar();
void bios_clear_screen();
void cmd_help();

static Command g_cmds[] = {
    { "foo",   &print_bar },
    { "clear", &bios_clear_screen },
    { "help",  &cmd_help },
};
static constexpr int g_cmd_count = sizeof(g_cmds) / sizeof(g_cmds[0]);
