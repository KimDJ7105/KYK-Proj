#pragma once
#include "stdafx.h"

#include "server.h"

const auto X_START_POS = 4;
const auto Y_START_POS = 4;

void Init_Server()
{
	_wsetlocale(LC_ALL, L"korean");
}


void worker_thread(boost::asio::io_context* service)
{
	service->run();
}

int main()
{
	boost::asio::io_context io_service;
	vector <thread > worker_threads;
	SERVER s(io_service, MY_PORT);

	Init_Server();

	for (auto i = 0; i < 4; i++) worker_threads.emplace_back(worker_thread, &io_service);
	for (auto& th : worker_threads) th.join();
}