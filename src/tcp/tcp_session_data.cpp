#include "tcp_session_data.h"
#include <iostream>

namespace net {

	tcp_session_data::tcp_session_data(uint32_t read_cache_size)
		:m_cache_size(read_cache_size)
		,m_cache_buffer(nullptr)
		,m_cache_write_position(0)
		,m_cache_read_position(0)
		,m_connected(false)
		,m_connecting(false)
		,m_connect_timeout(60)
		,m_read_timeout(60)
		,m_heartbeat_interval(30)
		,m_heartbeat_buffer(nullptr)
		,m_magic_key(0)
		,m_header_length(0)
		,m_read_skip_length(0)
		,m_on_connected_handler(nullptr)
		,m_on_message_handler(nullptr)
		,m_on_closed_handler(nullptr)
		,m_on_error_handler(nullptr)
	{
		m_cache_buffer = (uint8_t*)malloc(m_cache_size * sizeof(uint8_t));
	}

	tcp_session_data::~tcp_session_data()
	{
		std::cout << "tcp session data destructed." << std::endl;

		free(m_cache_buffer);

		m_io_service.reset();
		m_socket.reset();
		m_strand.reset();
		m_deadline.reset();
		m_heartbeat_timer.reset();
		m_heartbeat_buffer.reset();
		m_outbox.clear();
	}


}//namespace net