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
#include "Server.h"
#include "Tipbot.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Process.h"
#include "Poco/Thread.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/StreamCopier.h"
#include <sstream>
#include <thread>

Server::Server(unsigned short port) : serv(port), exit_server(false), shutdown_complete(true)
{
    PLog = &Poco::Logger::get("Script Server");
}

Server::~Server()
{
    stop();
}

void Server::server_loop()
{
    GlobalConfig.General.Threads++;
    PLog->information("Thread Started! Threads: %?i", GlobalConfig.General.Threads);

    shutdown_complete = false;
    while (!exit_server)
    {
        Poco::Net::StreamSocket socket = serv.acceptConnection();
        std::stringstream ss;
        Poco::Net::SocketStream istr(socket);
        //Poco::StreamCopier::copyStream(istr, ss);
        onConnect(this, socket.address().toString(), "I dont work", &socket);
        socket.close();
    }
    shutdown_complete = true;

    GlobalConfig.General.Threads--;
    PLog->information("Thread Stopped! Threads: %?i", GlobalConfig.General.Threads);
}

void Server::start()
{
    if (shutdown_complete)
    {
        // Create server thread
        std::thread t1(&Server::server_loop, this);
        t1.detach();
    }
}

void Server::stop()
{
    if (!shutdown_complete)
    {
        exit_server = true;
        while (!shutdown_complete) Poco::Thread::sleep(1);
    }
}

void Server::write(Poco::Net::StreamSocket* out, const std::string & data)
{
    if (!exit_server)
    {
        out->sendBytes(data.data(), (int)data.size());
    }
}

void Server::setOnConnectFunc(const OnConnectFunc & f)
{
    onConnect = f;
}