#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

#include <boost/asio.hpp> 
#include <boost/enable_shared_from_this.hpp>
#include "../byte_buffer.h"

namespace net {

	class tcp_session_data;

	class tcp_session  
		: public boost::enable_shared_from_this<tcp_session> 
	{
	public:
		typedef boost::shared_ptr<tcp_session>    ptr;
		typedef byte_buffer                       buffer_type;
		typedef boost::shared_ptr<buffer_type>    buffer_ptr;

	public:
		virtual tcp_session& connect();
		virtual tcp_session& send(boost::shared_ptr<buffer_type> snd_buf);
		virtual tcp_session& close();

		virtual bool io_service_stopped();

		tcp_session_data& data();

	public:
		tcp_session();
		virtual ~tcp_session();

	protected:
		virtual void start_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iter);
		virtual void handle_connect(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
		virtual void on_connected(boost::asio::ip::tcp::endpoint endpoint);
		
		virtual void start_read();
		virtual void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
		virtual uint32_t read_msg(uint8_t* buf, uint32_t buf_size, std::string& error);
		virtual uint32_t check_msg_len(uint8_t* buf, uint32_t buf_size);
		virtual uint32_t check_magic_key(uint8_t* buf, uint32_t buf_size);
		virtual void on_message(boost::shared_ptr<buffer_type> rcv_buf);
		
		virtual void start_write(boost::shared_ptr<buffer_type> snd_buf);
		virtual void handle_write(const boost::system::error_code& ec);

		virtual void start_close();
		virtual void on_closed();

		virtual void check_deadline(const boost::system::error_code& ec);
		virtual void send_heartbeat(const boost::system::error_code& ec);

		virtual void caught_error(const std::string& error);
		
	protected:
		boost::shared_ptr<tcp_session_data> m_data;
	};
}; // namespace net

#endif //__TCP_SESSION_H__