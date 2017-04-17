#include "tcp_client_data.h"
#include "tcp_session_data.h"
#include "../lua_util.h"


namespace net {

	tcp_client_data::tcp_client_data()
		: m_on_message_ref(LUA_REFNIL)
		, m_on_connected_ref(LUA_REFNIL)
		, m_on_closed_ref(LUA_REFNIL)
		, m_on_error_ref(LUA_REFNIL)
	{

	}

	tcp_client_data::~tcp_client_data()
	{
		std::cout << "tcp client data destructed." << std::endl;

		lua_State* L = m_lua_state;

		if (L) {
			if (m_on_message_ref != LUA_REFNIL) {
				luautil_unref_function(L, m_on_message_ref);
			}
			if (m_on_connected_ref != LUA_REFNIL) {
				luautil_unref_function(L, m_on_connected_ref);
			}
			if (m_on_closed_ref != LUA_REFNIL) {
				luautil_unref_function(L, m_on_closed_ref);
			}
			if (m_on_error_ref != LUA_REFNIL) {
				luautil_unref_function(L, m_on_error_ref);
			}
		}
	}

	tcp_session::buffer_ptr tcp_client_data::make_buf(const char* jsonp, size_t len)
	{
		tcp_session::buffer_ptr buf(new tcp_session::buffer_type(4 + len));

		buf->putInt(4 + len);
		buf->putBytes((uint8_t*)jsonp, len);

		return buf;
	}

	tcp_session::buffer_ptr tcp_client_data::make_buf(std::string& json)
	{
		tcp_session::buffer_ptr buf(new tcp_session::buffer_type(json.length() + 4));

		buf->putInt(4 + json.length());
		buf->putBytes((uint8_t*)json.data(), json.length());

		return buf;
	}

	tcp_session::buffer_ptr tcp_client_data::make_heartbeat_buf()
	{
		char hb[] = "heartbeat";

		tcp_session::buffer_ptr buf(new tcp_session::buffer_type(4 + sizeof(hb)));
		buf->putInt(4 + sizeof(hb));
		buf->putBytes((uint8_t*)hb, sizeof(hb));

		return buf;
	}

	void tcp_client_data::on_message(tcp_session::buffer_ptr buf)
	{
		std::string msg(buf->getRawBuf().begin(), buf->getRawBuf().end());
		std::cout << "response:" <<  msg << std::endl;

		if (m_on_message_ref != LUA_REFNIL)
		{
			/*ptr p = this->shared_from_this();
			cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				lua_State* L = cocos2d::LuaEngine::getInstance()->getLuaStack()->getLuaState();
				luautil_call_ref(L, p->m_on_message_ref, (const char*)(buf->data()), buf->size());
			});*/

			luautil_call_ref(m_lua_state, m_on_message_ref, (const char*)(buf->data()), buf->size());
		}
	}

	void tcp_client_data::on_connected(const std::string endpoint)
	{
		std::cout << "tcp connected to:" << endpoint << std::endl;
		if (m_on_connected_ref != LUA_REFNIL)
		{
			luautil_call_ref(m_lua_state, m_on_connected_ref, endpoint);
		}
	}

	void tcp_client_data::on_closed()
	{
		std::cout << "tcp client closed." << std::endl;
		if (m_on_closed_ref != LUA_REFNIL)
		{
			luautil_call_ref(m_lua_state, m_on_closed_ref);
		}
	}

	void tcp_client_data::on_error(const std::string error)
	{
		std::cout << "error:" << error << std::endl;
		if (m_on_error_ref != LUA_REFNIL)
		{
			luautil_call_ref(m_lua_state, m_on_error_ref, error);
		}
	}

	void tcp_client_data::set_on_connected_ref(int ref)
	{
		if (m_on_connected_ref != LUA_REFNIL) {
			luautil_unref_function(m_lua_state, m_on_connected_ref);
		}
		m_on_connected_ref = ref;
	}

	void tcp_client_data::set_on_message_ref(int ref)
	{
		if (m_on_message_ref != LUA_REFNIL) {
			luautil_unref_function(m_lua_state, m_on_message_ref);
		}
		m_on_message_ref = ref;
	}

	void tcp_client_data::set_on_closed_ref(int ref)
	{
		if (m_on_closed_ref != LUA_REFNIL) {
			luautil_unref_function(m_lua_state, m_on_closed_ref);
		}
		m_on_closed_ref = ref;

	}

	void tcp_client_data::set_on_error_ref(int ref)
	{
		if (m_on_error_ref != LUA_REFNIL) {
			luautil_unref_function(m_lua_state, m_on_error_ref);
		}
		m_on_error_ref = ref;
	}

	void tcp_client_data::set_lua_state(lua_State* L)
	{
		m_lua_state = L;
	}

}// namespace net