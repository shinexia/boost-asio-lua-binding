#include "lua.hpp"
#include "src/register_all_tcp_client.h"

#include <iostream>

int main() {
    /* initialize Lua */
    // create new Lua state

    lua_State *L;
    L = luaL_newstate();

    /*static const luaL_Reg lualibs[] =
    {
        { "base", luaopen_base },
        { "io", luaopen_io },
        { NULL, NULL }
    };

    const luaL_Reg *lib = lualibs;
    for (; lib->func != NULL; lib++)
    {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }*/

    luaL_openlibs(L);

    register_all_tcp_client(L);

    // run the Lua script
    int error = luaL_dofile(L, "hello.lua");

    if (error) {
        fprintf(stderr, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    // close the Lua state
    lua_close(L);

    std::getchar();

    return 0;
}