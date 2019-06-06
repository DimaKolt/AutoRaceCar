#ifndef TCPSOCKET_TCPCLIENT_H
#define TCPSOCKET_TCPCLIENT_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <inaddr.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <sys/types.h>
#endif

#include <vector>
#include "ITcpClient.h"

class TcpClient : public ITcpClient
{
public:

    TcpClient() noexcept;

    virtual void connect(const string& ip, const unsigned short& port) const override;

    virtual std::vector<char> receive(const unsigned int &len) const override;

    virtual void send(const std::vector<char>& data) const noexcept override;

    virtual void send(const string& data) const noexcept override;

    virtual void send(const char* data, const uint &len) const noexcept override;

    virtual ~TcpClient() override;

private: //functions
    TcpClient &createSocket();
    sockaddr_in buildAddress(const string &ip, const unsigned short &port) const noexcept;

private:
    Socket _socket;
    sockaddr_in _address;
};


#endif //TCPSOCKET_TCPCLIENT_H
