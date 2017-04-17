#ifndef __TCP_CLIENT_DATA_H__
#define __TCP_CLIENT_DATA_H__

#include "../stream_property.h"
#include "tcp_session.h"
#include "tcp_client.h"
#include "lua.hpp"

namespace net {
	class tcp_client_data
		: public boost::enable_shared_from_this<tcp_client_data>
	{
	public:
		typedef tcp_client_data                 data_type;
		typedef boost::shared_ptr<data_type>    ptr;

	public:
		tcp_session::buffer_ptr make_buf(const char* jsonp, size_t len);
		tcp_session::buffer_ptr make_buf(std::string& json);

		tcp_session::buffer_ptr make_heartbeat_buf();

		void on_connected(const std::string endpoint);
		void on_message(tcp_session::buffer_ptr buf);
		void on_closed();
		void on_error(const std::string error);

		void set_on_connected_ref(int ref);
		void set_on_message_ref(int ref);
		void set_on_closed_ref(int ref);
		void set_on_error_ref(int ref);
		void set_lua_state(lua_State* L);

	public:
		tcp_client_data();
		~tcp_client_data();

	private:
		int m_on_message_ref;
		int m_on_connected_ref;
		int m_on_closed_ref;
		int m_on_error_ref;

		lua_State* m_lua_state;

	};

}// namespace ft


#endif //__TCP_CLIENT_DATA_H__