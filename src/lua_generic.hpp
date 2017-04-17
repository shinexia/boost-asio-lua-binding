#include "lua.hpp"

template <typename T>
class lua_generic {
public:
	typedef struct { T *pT; } userdataType;
	typedef int (T::*Function)(lua_State *L);
	typedef struct { const char *name; Function func; } RegType;

	// get userdata from Lua stack and return pointer to T object  
	static T *check(lua_State *L, int narg) {
		userdataType *ud = static_cast<userdataType*>(luaL_checkudata(L, narg, T::package));
		if (!ud) {
			luaL_typerror(L, narg, T::package);
			return nullptr;
		}
		return ud->pT;  // pointer to T object  
	}

public:
	// member function dispatcher  
	static int thunk(lua_State *L)
	{
		// stack has userdata, followed by method args  
		T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'  
		lua_remove(L, 1);  // remove self so member function args start at index 1  

		// get member function from upvalue  
		RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
		return (obj->*(l->func))(L);  // call member function  
	}

	static int __new(lua_State* L) {
		T *obj = new T();  // call constructor for T objects  

		userdataType *ud = static_cast<userdataType*>(lua_newuserdata(L, sizeof(userdataType)));
		ud->pT = obj;  // store pointer to object in userdata¡¢  

		luaL_getmetatable(L, T::package);  // lookup metatable in Lua registry  
		lua_setmetatable(L, -2);
		return 1;  // userdata containing pointer to T object  
	}

	static int __gc(lua_State* L) {
		userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
		T *obj = ud->pT;
		if (obj) {
			delete obj;  // call destructor for T objects  
		}
		return 0;
	}
};