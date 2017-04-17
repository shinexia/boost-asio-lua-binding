#include "tcp_session.h"
#include "tcp_session_data.h"
#include <boost/lexical_cast.hpp> 
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

using boost::asio::io_service;
using boost::asio::ip::tcp;

namespace net {
	tcp_session::tcp_session()
		:m_data(new tcp_session_data())
	{
		
	}

	tcp_session::~tcp_session()
	{
		std::cout << "tcp session destructed." << std::endl;
		m_data.reset();
	}

	// public
	tcp_session& tcp_session::connect()
	{
		//std::cout << "host:" << m_data->host() << ", port:" << m_data->port() << std::endl;

		memset(m_data->cache_buffer(), 0, 8 * sizeof(uint32_t)); // reset first 64 bit with 0

		m_data->io_service().reset(new boost::asio::io_service());
		m_data->socket().reset(new boost::asio::ip::tcp::socket(*m_data->io_service()));
		m_data->strand().reset(new boost::asio::io_service::strand(*m_data->io_service()));
		m_data->deadline().reset(new boost::asio::deadline_timer(*m_data->io_service()));
		m_data->heartbeat_timer().reset(new boost::asio::deadline_timer(*m_data->io_service()));

		boost::asio::ip::tcp::resolver resolver(*m_data->io_service());
		tcp::resolver::query query(m_data->host(), boost::lexical_cast<std::string, uint16_t>(m_data->port()));
		tcp::resolver::iterator endpoint_iter = resolver.resolve(query);

		m_data->connecting(true);

		// Start the connect actor.
		start_connect(endpoint_iter);

		// Start the deadline actor. You will note that we're not setting any
		// particular deadline here. Instead, the connect and input actors will
		// update the deadline prior to each asynchronous operation.
		m_data->deadline()->async_wait(boost::bind(&tcp_session::check_deadline, shared_from_this(), boost::asio::placeholders::error));

		boost::thread t(boost::bind(&boost::asio::io_service::run, m_data->io_service()));
		t.detach();

		return *this;
	}

	tcp_session& tcp_session::send(boost::shared_ptr<buffer_type> snd_buf)
	{
		if (io_service_stopped()){
			caught_error("connection already closed.");
			return *this;
		}
		m_data->strand()->post(boost::bind(&tcp_session::start_write, shared_from_this(), snd_buf));

		return *this;
	}

	tcp_session& tcp_session::close()
	{
		if (io_service_stopped()){
			caught_error("repeatedly stop.");
			return *this;
		}

		m_data->strand()->post(boost::bind(&tcp_session::start_close, shared_from_this()));
		return *this;
	}

	bool tcp_session::io_service_stopped()
	{
		return m_data->io_service() == nullptr || m_data->io_service()->stopped();
	}

	tcp_session_data& tcp_session::data(){
		return *m_data;
	}

	// protected
	void tcp_session::start_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iter)
	{
		if (endpoint_iter != tcp::resolver::iterator())
		{
			//std::cout << "trying to connect to:" << endpoint_iter->endpoint() << std::endl;

			// Set a deadline for the connect operation.
			m_data->deadline()->expires_from_now(boost::posix_time::seconds(m_data->connect_timeout()));

			// Start the asynchronous connect operation.
			m_data->socket()->async_connect(endpoint_iter->endpoint(),
				boost::bind(&tcp_session::handle_connect, shared_from_this(), boost::asio::placeholders::error, endpoint_iter));
		}
		else
		{
			caught_error("There are no more endpoints to try. Shut down the client.");
			start_close();
		}
	}

	void tcp_session::handle_connect(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpoint_iter)
	{
		if (!m_data->connecting() || ec == boost::asio::error::operation_aborted)
		{
			this->caught_error("operation_aborted:" + ec.message());
			return;
		}

		// The async_connect() function automatically opens the socket at the start
		// of the asynchronous operation. If the socket is closed at this time then
		// the timeout handler must have run first.
		if (!m_data->socket()->is_open())
		{
			//std::cout << "Connect to:" << endpoint_iter->endpoint() << " timed out." << std::endl;

			// Try the next available endpoint.
			start_connect(++endpoint_iter);
		}

		// Check if the connect operation failed before the deadline expired.
		else if (ec)
		{
			//std::cout << "Connect to:" << endpoint_iter->endpoint() << " error:" << ec.message() << std::endl;

			// We need to close the socket used in the previous connection attempt
			// before starting a new one.
			m_data->socket()->close();

			// Try the next available endpoint.
			start_connect(++endpoint_iter);
		}

		// Otherwise we have successfully established a connection.
		else
		{
			//std::cout << "Connect to:" << endpoint_iter->endpoint() << " succeed." << std::endl;

			m_data->connected(true);
			m_data->connecting(false);

			// Start the input actor.
			start_read();

			// Wait before sending the next heartbeat or customer message.
			m_data->heartbeat_timer()->expires_from_now(boost::posix_time::seconds(m_data->heartbeat_interval()));
			m_data->heartbeat_timer()->async_wait(boost::bind(&tcp_session::send_heartbeat, shared_from_this(), boost::asio::placeholders::error));

			on_connected(endpoint_iter->endpoint());
		}
	}

	void tcp_session::on_connected(boost::asio::ip::tcp::endpoint endpoint) {
		//std::cout << "Connect to:" << endpoint << " succeed." << std::endl;

		if(m_data->on_connected_handler() != nullptr) {
			std::ostringstream os ;
				os << endpoint;
			m_data->on_connected_handler()(os.str());
		}
	}

	void tcp_session::start_read()
	{
		// Set a deadline for the read operation.
		m_data->deadline()->expires_from_now(boost::posix_time::seconds(m_data->read_timeout()));

		// Waiting to read.
		m_data->socket()->async_read_some(
			boost::asio::buffer(
				(m_data->cache_buffer() + m_data->cache_write_position()),
				(m_data->cache_size() - m_data->cache_write_position())),
			boost::bind(&tcp_session::handle_read, shared_from_this(), 
				boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred));
	}

	void tcp_session::handle_read(const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if(!ec)  
		{
			m_data->cache_write_position() += bytes_transferred;

			//new handle begin
			while(true){
				std::string error;
				uint32_t read_size = this->read_msg(
					(m_data->cache_buffer() + m_data->cache_read_position()),
					(m_data->cache_write_position() - m_data->cache_read_position()),
					error);
				//std::cout<<"read size:"<<read_size<<endl;
				if(!error.empty()) {
					this->caught_error(error);
					this->start_close();
					return;
				}
				else if(read_size != 0){
					m_data->cache_read_position() += read_size;
					//std::cout<<"decoded not empty, continue!"<<std::endl;
				}
				else{
					// read_size == 0;
					//std::cout<<"decoded empty, break!"<<std::endl;
					break;
				}
			}

			if(m_data->cache_write_position() == m_data->cache_read_position()){
				//do not need memory move, just reset r/w position.
				m_data->cache_write_position(0);
				m_data->cache_read_position(0);
				//std::cout<<"reset R/W position"<<std::endl;
			}else{
				//std::cout<<"before memory move"<<std::endl;
				::memmove(m_data->cache_buffer(), 
					m_data->cache_buffer() + m_data->cache_read_position(), 
					(m_data->cache_write_position() - m_data->cache_read_position()));
				m_data->cache_write_position(m_data->cache_write_position() - m_data->cache_read_position());
				m_data->cache_read_position(0);
			}
			
			//end of new handle
			start_read();
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			this->caught_error(ec.message());
			this->start_close();
			return;
		}
		/*else {
			this->caught_error("operation_aborted:" + ec.message());
		}*/
		return;  
	}

	uint32_t tcp_session::read_msg(uint8_t* buf, uint32_t buf_size, std::string& error)
	{
		if(buf_size <= m_data->header_length() ){
			//do not read.
			return 0;
		}

		uint32_t read_len = 0;
		uint32_t msg_len = check_msg_len(buf, buf_size);//big endian (network byte order).

		if(msg_len <= m_data->header_length() || msg_len > m_data->cache_size()){
			error = "invalid msg length:" + boost::lexical_cast<std::string, uint32_t>(msg_len);
			return 0;
		}

		if(m_data->magic_key() != 0){
			uint32_t flag = check_magic_key(buf, buf_size);
			if(flag != m_data->magic_key()) {
				error = "invalid magic key, client:" + boost::lexical_cast<std::string, uint32_t>(m_data->magic_key()) \
					+ ", server:" + boost::lexical_cast<std::string, uint32_t>(flag);
				return 0;
			}
		}

		//std::cout<<"Len:"<<msg_len<<std::endl;
		if(msg_len <= buf_size){
			//we can read at least one message.
			read_len = msg_len;
			//Utils::hex_dump(data,m_datasize);
			buffer_ptr bufp(new buffer_type(buf + m_data->read_skip_length(), msg_len - m_data->read_skip_length()));

			on_message(bufp);
		}else{
			//wait for more data.
			read_len = 0;
		}

		return read_len;
	}

	uint32_t tcp_session::check_msg_len(uint8_t* buf, uint32_t buf_size)
	{
		uint32_t int_bit = sizeof(uint32_t);
		uint32_t ret = 0;

		for(uint32_t i =0 ; i<int_bit; i++){
			ret = *(buf + i) + (ret << 8);
		}

		return ret;
	}

	uint32_t tcp_session::check_magic_key(uint8_t* buf, uint32_t buf_size)
	{
		uint32_t short_bit = sizeof(uint16_t);
		uint32_t ret = 0;

		for(uint32_t i =0 ; i<short_bit; i++){
			ret = *(buf + 4 + i) + (ret << 8);
		}

		return ret;
	}

	void tcp_session::on_message(boost::shared_ptr<buffer_type> msg_buffer)
	{
		//std::cout << "response:" << std::string(msg_buffer->getRawBuf().begin(), msg_buffer->getRawBuf().end()) << std::endl;

		if(m_data->on_message_handler() != nullptr) {
			m_data->on_message_handler()(msg_buffer);
		}
	}

	void tcp_session::start_write(boost::shared_ptr<buffer_type> snd_buffer)
	{
		bool write_in_progress = !m_data->outbox().empty();
		m_data->outbox().push_back(snd_buffer);
		if (!write_in_progress)
		{
			// not in write progress
			//std::cout << "request:" << std::string(snd_buffer->begin() + 4, snd_buffer->end()) << ", outbox:" << m_data->outbox().size() << std::endl;

			// cancel heartbeat sending.
			m_data->heartbeat_timer()->cancel();

			boost::asio::async_write(*m_data->socket(),
				boost::asio::buffer(snd_buffer->getRawBuf()),
				boost::bind(&tcp_session::handle_write, shared_from_this(), boost::asio::placeholders::error));
		}
	}

	void tcp_session::handle_write(const boost::system::error_code& ec)
	{
		if (!ec)
		{
			//std::cout << "send msg complete." << std::endl;
			m_data->outbox().pop_front();
			if (!m_data->outbox().empty())
			{
				boost::asio::async_write(*m_data->socket(),
					boost::asio::buffer(m_data->outbox().front()->getRawBuf()),
					boost::bind(&tcp_session::handle_write, shared_from_this(), boost::asio::placeholders::error)); 
			}
			else {
				// Wait before sending the next heartbeat or customer message.
				m_data->heartbeat_timer()->expires_from_now(boost::posix_time::seconds(m_data->heartbeat_interval()));
				m_data->heartbeat_timer()->async_wait(boost::bind(&tcp_session::send_heartbeat, shared_from_this(), boost::asio::placeholders::error));
			}
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			caught_error(ec.message());
			start_close();
		}
	}

	void tcp_session::check_deadline(const boost::system::error_code& ec)
	{
		if (!m_data->connected() && !m_data->connecting()) {
			return;
		}

		if (!ec || ec == boost::asio::error::operation_aborted)
		{
			// Check whether the deadline has passed. We compare the deadline against
			// the current time since a new asynchronous operation may have moved the
			// deadline before this actor had a chance to run.
			if (m_data->deadline()->expires_at() <= boost::asio::deadline_timer::traits_type::now())
			{
				caught_error("connection timeout.");

				// The deadline has passed. The socket is closed so that any outstanding
				// asynchronous operations are cancelled.
				start_close();

				// There is no longer an active deadline. The expiry is set to positive
				// infinity so that the actor takes no action until a new deadline is set.
				//deadline_->expires_at(boost::posix_time::pos_infin);
			}
			else
			{
				// Put the actor back to sleep.
				m_data->deadline()->async_wait(boost::bind(&tcp_session::check_deadline, shared_from_this(), boost::asio::placeholders::error));
			}
		}
		else {
			caught_error(ec.message());
			start_close();
		}
	}

	void tcp_session::send_heartbeat(const boost::system::error_code& ec)
	{
		if (!ec)
		{
			bool write_in_progress = !m_data->outbox().empty();
			if (!write_in_progress)
			{
				// std::cout << "send heartbeart" << std::endl;
				start_write(m_data->heartbeat_buffer());
			}
		}
		else if (ec != boost::asio::error::operation_aborted) {
			this->caught_error(ec.message());
			this->start_close();
		}
	}

	void tcp_session::start_close()
	{
		if(!m_data->connected() && !m_data->connecting()) {
			//caught_error("repeatedly close.");
			return;
		}

		boost::system::error_code ec;

		if(m_data->connected())
		{
			m_data->socket()->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			if(ec) {
				caught_error(ec.message());
			}
		}

		m_data->connected(false);
		m_data->connecting(false);

		m_data->socket()->close(ec);
		if(ec) {
			caught_error(ec.message());
		}

		m_data->io_service()->stop();

		m_data->deadline()->cancel();
		m_data->heartbeat_timer()->cancel();

		m_data->io_service().reset();
		m_data->socket().reset();
		m_data->strand().reset();
		m_data->deadline().reset();
		m_data->heartbeat_timer().reset();
		m_data->heartbeat_buffer().reset();

		on_closed();
	}

	void tcp_session::on_closed()
	{
		// std::cout << "session on closed." << std::endl;

		if(m_data->on_closed_handler() != nullptr) {
			m_data->on_closed_handler()();
		}
	}

	void tcp_session::caught_error(const std::string& error)
	{
		//std::cerr << "Error: " << error << std::endl;

		if(m_data->on_error_handler() != nullptr) {
			m_data->on_error_handler()(error);
		}
	}
} // namespace net
