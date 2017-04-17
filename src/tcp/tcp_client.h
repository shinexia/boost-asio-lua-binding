#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <boost/enable_shared_from_this.hpp>

namespace net {

	class tcp_session;
	class tcp_client_data;
	class tcp_session_data;

	class tcp_client
		: public boost::enable_shared_from_this<tcp_client>
	{
	public:
		typedef boost::shared_ptr<tcp_client>  ptr;

	public:
		virtual tcp_client& connect(std::string host, uint32_t port);
		virtual tcp_client& connect();

		virtual tcp_client& send(std::string json);
		virtual tcp_client& send(const char* jsonp, size_t len);

		virtual tcp_client& close();

		tcp_session& session();
		tcp_session_data& session_data();

		tcp_client_data&  data();

	public:
		tcp_client();
		virtual ~tcp_client();

	protected:
		boost::shared_ptr<tcp_session>  m_session;
		boost::shared_ptr<tcp_client_data> m_data;
	};
};

#endif //__TCP_CLIENT_H__