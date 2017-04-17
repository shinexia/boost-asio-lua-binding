#include "lua_util.h"
#include <sstream>  
#include <string>  
#include <iostream>

void luautil_dump_stack(lua_State* l)
{
	int i;
	int top = lua_gettop(l);

	std::ostringstream oss;

	oss << "total in stack " << top << "\n";

	for (i = 1; i <= top; i++)
	{  /* repeat for each level */
		int t = lua_type(l, i);
		switch (t) {
		case LUA_TSTRING:  /* strings */
			oss << "string: " << lua_tostring(l, i) << "\n";
			break;
		case LUA_TBOOLEAN:  /* booleans */
			oss << "boolean " << (lua_toboolean(l, i) ? "true" : "false") << "\n";
			break;
		case LUA_TNUMBER:  /* numbers */
			oss << "number: " << lua_tonumber(l, i) << "\n";
			break;
		default:  /* other values */
			oss << "" << lua_typename(l, t) << "\n";
			break;
		}
		oss << "  ";  /* put a separator */
	}
	std::cout << oss.str() << std::endl;  /* end the listing */
}

int luautil_ref_function(lua_State* L)
{
	if (lua_isfunction(L, -1)) // ensure argument (which is on top of the stack) is a function
	{
		int refId = luaL_ref(L, LUA_REGISTRYINDEX);
		return refId;
	}
	std::cout << "not a function." <<std::endl;
	return 0;
}

int luautil_unref_function(lua_State* L, int ref)
{
	luaL_unref(L, LUA_REGISTRYINDEX, ref);
	return 0;
}

int luautil_call_ref(lua_State* L, int ref)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	lua_pcall(L, 0, 0, 0);
	lua_pop(L, 1);

	return 0;
}

int luautil_call_ref(lua_State* L, int ref, std::string json)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	lua_pushlstring(L, json.c_str(), json.length());

	lua_pcall(L, 1, 0, 0);

	lua_pop(L, 2);

	return 0;
}

int luautil_call_ref(lua_State* L, int ref, const char* jsonp, size_t len)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	lua_pushlstring(L, jsonp, len);

	lua_pcall(L, 1, 0, 0);

	lua_pop(L, 2);

	return 0;
}