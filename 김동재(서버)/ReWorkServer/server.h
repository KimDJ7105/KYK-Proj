#pragma once
#include "session.h"

class SERVER
{
private:
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	atomic_int g_user_ID;

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
			{
				if (!ec)
				{
					int p_id = GetNewClientID();
					std::cout << "Client " << p_id << " loged in\n";
					players[p_id] = std::make_shared<SESSION>(std::move(socket_), p_id);
					players[p_id]->start();
					do_accept();
				}
			});
	}
	int GetNewClientID()
	{
		if (g_user_ID >= MAX_USER) {
			cout << "MAX USER FULL\n";
			exit(-1);
		}
		return g_user_ID++;
	}

public:
	SERVER(boost::asio::io_context& io_service, int port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
		socket_(io_service)
	{
		do_accept();
	}

};