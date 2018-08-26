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