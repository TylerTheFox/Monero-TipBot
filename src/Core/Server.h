/*
Copyright(C) 2018 Brandan Tyler Lasley

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
*/
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Logger.h"
#include <functional>

class Server;
typedef std::function<void(Server*,std::string, std::string, Poco::Net::StreamSocket*)> OnConnectFunc;

class Server
{
public:
    Server(unsigned short port);
    ~Server();

    void                                start();
    void                                stop();
    void                                write(Poco::Net::StreamSocket* out, const std::string & data);
    void                                setOnConnectFunc(const OnConnectFunc & f);
private:
    Poco::Net::ServerSocket             serv;
    OnConnectFunc                       onConnect;
    Poco::Logger*                       PLog;
    bool                                exit_server;
    bool                                shutdown_complete;
    void                                server_loop();
};