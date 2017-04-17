#ifndef __TCP_SESSION_DATA_H__
#define __TCP_SESSION_DATA_H__

#include "../stream_property.h"
#include "tcp_session.h"
#include <deque>

namespace net {

	class tcp_session_data
		: public boost::enable_shared_from_this<tcp_session_data> 
	{
	public:
		typedef tcp_session_data                               data_type;
		typedef boost::shared_ptr<data_type>                   ptr;

		typedef std::function<void(std::string)>               on_connected_handler_type;
		typedef std::function<void(tcp_session::buffer_ptr)>   on_message_handler_type;
		typedef std::function<void(void)>                      on_closed_handler_type;
		typedef std::function<void(std::string)>               on_error_handler_type;


	public:
		STREAM_PROPERTY(bool, connected);
		STREAM_PROPERTY(bool, connecting);

		STREAM_PROPERTY(std::string, host);
		STREAM_PROPERTY(uint32_t, port);

		STREAM_PROPERTY(boost::shared_ptr<boost::asio::io_service>, io_service);
		STREAM_PROPERTY(boost::shared_ptr<boost::asio::ip::tcp::socket>, socket);
		STREAM_PROPERTY(boost::shared_ptr<boost::asio::io_service::strand>, strand);
		STREAM_PROPERTY(boost::shared_ptr<boost::asio::deadline_timer>, deadline);
		STREAM_PROPERTY(boost::shared_ptr<boost::asio::deadline_timer>, heartbeat_timer);

		STREAM_PROPERTY(uint32_t, connect_timeout);
		STREAM_PROPERTY(uint32_t, read_timeout);
		STREAM_PROPERTY(uint32_t, heartbeat_interval);

		STREAM_PROPERTY(uint32_t, magic_key);

		STREAM_PROPERTY(tcp_session::buffer_ptr, heartbeat_buffer);

		STREAM_CONST_PROPERTY(uint32_t, cache_size);

		STREAM_PROPERTY(uint8_t*, cache_buffer);
		STREAM_PROPERTY(uint32_t, cache_write_position);
		STREAM_PROPERTY(uint32_t, cache_read_position);

		STREAM_PROPERTY(uint32_t, header_length);
		STREAM_PROPERTY(uint32_t, read_skip_length);

		STREAM_PROPERTY(std::deque<tcp_session::buffer_ptr>, outbox);

		STREAM_PROPERTY(on_connected_handler_type, on_connected_handler);
		STREAM_PROPERTY(on_closed_handler_type, on_closed_handler);
		STREAM_PROPERTY(on_message_handler_type, on_message_handler);
		STREAM_PROPERTY(on_error_handler_type, on_error_handler);

	public:
		tcp_session_data(uint32_t read_cache_size = 8192);
		~tcp_session_data();
	};
}// namespace net


#endif //__TCP_SESSION_DATA_H__