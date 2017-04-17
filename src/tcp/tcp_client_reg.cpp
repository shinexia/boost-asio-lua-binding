#include "tcp_client.h"
#include "tcp_session_data.h"
#include "tcp_client_data.h"
#include "../lua_util.h"


using namespace net;

typedef struct {
	tcp_client* pT;
}userdataType;

static const char* packageName = "net.tcp.client";

static int net_tcp_client_new(lua_State* L) {
	tcp_client *obj = new tcp_client();  // call constructor for T objects 

	obj->data().set_lua_state(L);

	userdataType *ud = static_cast<userdataType*>(lua_newuserdata(L, sizeof(userdataType)));
	ud->pT = obj;  // store pointer to object in userdata

	luaL_getmetatable(L, packageName);  // lookup metatable in Lua registry  
	lua_setmetatable(L, -2);

	//std::cout << "tcp client created." << std::endl;

	return 1;  // userdata containing pointer to T object  
}

static tcp_client* net_tcp_client_check(lua_State *L, int narg) {
	userdataType *ud = static_cast<userdataType*>(luaL_checkudata(L, narg, packageName));
	if (!ud) {
		luaL_argerror(L, narg, packageName);
		return nullptr;
	}
	return ud->pT;  // pointer to T object  
}

static int net_tcp_client_gc(lua_State* L) {
	userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
	tcp_client *obj = ud->pT;
	if (obj) {
		delete obj;  // call destructor for T objects  
	}
	return 0;
}

static int net_tcp_client_setHost(lua_State* L) {
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s && lua_isstring(L, -1)) {
		std::string host = lua_tostring(L, -1);

		s->session_data()
			.host(host);
	}

	return 0;
}

static int net_tcp_client_setPort(lua_State* L) {
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s && lua_isnumber(L, -1)) {
		uint32_t port = lua_tointeger(L, -1);

		s->session_data()
			.port(port);
	}

	return 0;
}

static int net_tcp_client_connect(lua_State* L) {
	tcp_client* s = net_tcp_client_check(L, 1);
	if (s) {
		s->session().connect();
	}
	
	return 0;
}

static int net_tcp_client_send(lua_State* L)
{
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s && lua_isstring(L, -1)) {
		size_t len = 0;

		luaL_checktype(L, -1, LUA_TSTRING);
		const char* jsonp = lua_tolstring(L, -1, &len);

		s->send(jsonp, len);
	}

	return 0;
}

static int net_tcp_client_close(lua_State* L)
{
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s) {
		s->close();
	}

	return 0;
}

static int net_tcp_client_onMessage(lua_State* L)
{
	tcp_client* s = net_tcp_client_check(L, 1);

	//luautil_dump_stack(L);
	if (s && lua_isfunction(L, -1)) {
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		s->data().set_on_message_ref(ref);
	}
	return 0;
}

static int net_tcp_client_onConnected(lua_State* L)
{
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s && lua_isfunction(L, -1)) {
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		s->data().set_on_connected_ref(ref);
	}

	return 0;
}

static int net_tcp_client_onClosed(lua_State* L)
{
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s && lua_isfunction(L, -1)) {
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		s->data().set_on_closed_ref(ref);
	}

	return 0;
}

static int net_tcp_client_onError(lua_State* L)
{
	tcp_client* s = net_tcp_client_check(L, 1);

	if (s && lua_isfunction(L, -1)) {
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		s->data().set_on_error_ref(ref);
	}

	return 0;
}

static const luaL_Reg tcp_client_lib_m[] = {
	{ "new", net_tcp_client_new },
	{ "__gc", net_tcp_client_gc },
	{ NULL, NULL },
};

static const luaL_Reg tcp_client_lib_f[] = {
	{ "setHost", net_tcp_client_setHost },
	{ "setPort", net_tcp_client_setPort },
	{ "connect", net_tcp_client_connect },
	{ "send", net_tcp_client_send },
	{ "close", net_tcp_client_close },
	{ "onMessage", net_tcp_client_onMessage },
	{ "onConnected", net_tcp_client_onConnected },
	{ "onClosed", net_tcp_client_onClosed },
	{ "onError", net_tcp_client_onError },
	{ NULL, NULL },
};

int luaopen_net_tcp_client(lua_State* L)
{
	//create metatable
	luaL_newmetatable(L, packageName);
	int metatable = lua_gettop(L);

	for (const luaL_Reg *l = tcp_client_lib_m; l->name; l++) {
		lua_pushstring(L, l->name);
		lua_pushcfunction(L, l->func);
		lua_settable(L, metatable);
	}

	lua_pushstring(L, "__NAME");
	lua_pushstring(L, packageName);
	lua_settable(L, metatable);

	//metatable.__index = methodtable
	lua_pushliteral(L, "__index");

	lua_newtable(L);
	int methods = lua_gettop(L);

	for (const luaL_Reg *l = tcp_client_lib_f; l->name; l++) {
		lua_pushstring(L, l->name);
		lua_pushcfunction(L, l->func);
		lua_settable(L, methods);
	}
	lua_settable(L, metatable);

	return 1;
}

int register_net_tcp_client(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, luaopen_net_tcp_client);
	lua_setfield(L, -2, packageName);

	lua_pop(L, 2);

	return 0;
}