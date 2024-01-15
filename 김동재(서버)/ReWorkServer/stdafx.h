#pragma once
#include <iostream>
#include <thread>
#include <atomic>
#include <concurrent_unordered_map.h>

#include <boost/asio.hpp>

#include "protocol.h"

using namespace std;
using boost::asio::ip::tcp;
