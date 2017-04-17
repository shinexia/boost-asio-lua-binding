#include "tcp_client.h"
#include "tcp_client_data.h"
#include "tcp_session_data.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <iostream>


namespace net {
	tcp_client::tcp_client()
		:m_data(new tcp_client_data())
		,m_session(new tcp_session())
	{
		m_session->data()
			.read_timeout(60)
			.heartbeat_interval(3)
			.heartbeat_buffer(m_data->make_heartbeat_buf())
			.magic_key(0)
			.header_length(4)
			.read_skip_length(4)
			.on_message_handler(std::bind(&tcp_client_data::on_message, m_data->shared_from_this(), std::placeholders::_1))
			.on_connected_handler(std::bind(&tcp_client_data::on_connected, m_data->shared_from_this(), std::placeholders::_1))
			.on_closed_handler(std::bind(&tcp_client_data::on_closed, m_data->shared_from_this()))
			.on_error_handler(std::bind(&tcp_client_data::on_error, m_data->shared_from_this(), std::placeholders::_1))
			;
	}

	tcp_client::~tcp_client()
	{
		std::cout << "tcp client destructed." << std::endl;

		// m_session's life is holded by client and io_service, so
		// just call m_session.reset() can't destruct m_session, you also need close it.
		// and close m_session is asynchronous, that means m_session will be destructed a later. 

		if(m_session != nullptr && !m_session->io_service_stopped()){
			m_session->close();
		}

		// now m_session's life is holded by m_session's io_service.
		m_session.reset();

		// now m_data's life is holded by m_session.
		m_data.reset();
	}

	tcp_client& tcp_client::connect(std::string host, uint32_t port)
	{
		m_session->data()
				.host(host)
				.port(port);
		this->connect();
		return *this;
	}

	tcp_client& tcp_client::connect()
	{
		m_session->connect();
		return *this;
	}

	tcp_client& tcp_client::send(std::string json)
	{
		m_session->send(m_data->make_buf(json));
		return *this;
	}

	tcp_client& tcp_client::send(const char* jsonp, size_t len)
	{
		m_session->send(m_data->make_buf(jsonp, len));
		return *this;
	}

	tcp_client& tcp_client::close()
	{
		m_session->close();
		return *this;
	}

	tcp_session& tcp_client::session()
	{
		return *m_session;
	}

	tcp_session_data& tcp_client::session_data()
	{
		return m_session->data();
	}

	tcp_client_data& tcp_client::data()
	{
		return *m_data;
	}

	

	
};