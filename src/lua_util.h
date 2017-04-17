#ifndef __LUA_UTIL_H__
#define __LUA_UTIL_H__


#include "lua.hpp"
#include <iostream>


extern int luautil_ref_function(lua_State* L);
extern int luautil_unref_function(lua_State* L, int ref);

extern int luautil_call_ref(lua_State* L, int ref);
extern int luautil_call_ref(lua_State* L, int ref, std::string json);
extern int luautil_call_ref(lua_State* L, int ref, const char* jsonp, size_t len);

extern void luautil_dump_stack(lua_State* l);


#endif // !__LUA_UTIL_H__
