#include "register_all_tcp_client.h"
#include "byte_buffer_reg.h"
#include "tcp/tcp_client_reg.h"

int register_all_tcp_client(lua_State* L)
{
	register_byte_buffer(L);
	register_net_tcp_client(L);

	return 0;
}