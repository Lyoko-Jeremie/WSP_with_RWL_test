#include "WinSocketPack.h"

#include "CharArray.h"

#include "RWLock.h"

// C++11 Pthread 库
//#include <thread>
//#include <mutex>


#include <iostream>

#include <stdexcept> // 抛出错误 runtime_error
#include <vector>
#include <string>
#include <sstream>
using namespace std;


WinSocketPack::WinSocketPack() throw (std::runtime_error)
        : RWLPoolSocket(true)   // 初始化锁参数
{

    // 单例 保护
    {
        int static a;
        if (a==1)
        {
            throw std::runtime_error("Cannot ctor two WinSocketPack.");
        }
        else
        {
            a = 1;
        }
    }

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    // 要启动的版本号
//    wVersionRequested = MAKEWORD( 1, 1 );
    wVersionRequested = MAKEWORD( 2, 2 );   // 2.2

    // 启动库
    // WSAStartup ???
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) // 检查启动错误
    {
//        cout << "WSAStartup error" << endl;
        throw std::runtime_error("WSAStartup error");
    }

}

WinSocketPack::~WinSocketPack()
{
    {   /// ATMWGuard
        ATMWGuard atm(this->RWLPoolSocket);
        // 从后往前关闭
        while ( !this->PoolSocket.empty() )
        {
    //        if ( *this->PoolSocket.end() - 1 != 0 )       // 之前没有关闭
    //        {
    //            int info = 0;
    //            do{
    //                info = closesocket( *this->PoolSocket.end() - 1 );
    //            }while( info == WSAEWOULDBLOCK );       // 非阻塞套接字关闭时需要等待重试
    //        }
    //        this->PoolSocket.erase(this->PoolSocket.end() - 1 );
            {
                ATMWTRGuard atmtr(this->RWLPoolSocket);
                this->CloseSocket( this->GetPoolSize() -1);
            }
            this->PoolSocket.erase(this->PoolSocket.end() - 1 );
        }
    }   /// ATMWGuard
    WSACleanup();
}


SOCKET WinSocketPack::GetSocket(size_t i)
{
    ATMRGuard atm(this->RWLPoolSocket);
    return this->PoolSocket.at(i);
}



size_t WinSocketPack::GetPoolSize()
{
    ATMRGuard atm(this->RWLPoolSocket);
    return this->PoolSocket.size();
}



bool WinSocketPack::EmptyPool()
{
    ATMRGuard atm(this->RWLPoolSocket);
    return this->PoolSocket.empty();
}



void WinSocketPack::CloseSocket(size_t i)
{
    ATMWGuard atm(this->RWLPoolSocket);
    if ( this->PoolSocket.at(i) != 0 )       // 之前没有关闭
    {
        int info = 0;
        do{
            info = closesocket( this->PoolSocket.at(i) );
        }while( info == WSAEWOULDBLOCK );       // 非阻塞套接字关闭时需要等待重试
//        this->PoolSocket.erase( this->PoolSocket.begin() + i );       为多线程着想  不移除【erase】
//        移除会导致定位用下标失效
        this->PoolSocket[i] = 0;
    }
}



bool WinSocketPack::Shutdown(size_t i, WSP_SHUTDOWN how)
{
    ATMRGuard atm(this->RWLPoolSocket);
    return shutdown( this->PoolSocket.at(i), how) != SOCKET_ERROR ? true : false;
}



size_t WinSocketPack::NewSocket(WSP_AF af, WSP_TYPE type, WSP_PROTOCOL protocol)
{
    SOCKET socks=socket( AF_INET, SOCK_STREAM, 0);
    if ( INVALID_SOCKET == socks)
    {
        stringstream ss("socket NewSocket INVALID_SOCKET ");
        ss << WSAGetLastError();
        throw std::runtime_error( ss.str() );
    }
    {   /// ATMWGuard
        ATMWGuard atm(this->RWLPoolSocket);
        this->PoolSocket.push_back(socks);
        return this->PoolSocket.size() - 1;
    }   /// ATMWGuard
}


size_t WinSocketPack::NewSocketTCP()
{
    return this->NewSocket( WSP_AF_INET, WSP_SOCK_STREAM, WSP_IPPROTO_IP );
}


bool WinSocketPack::Bind(size_t i, std::string addr, int port, WSP_AF family)
{
    sockaddr_in addrin;
    addrin.sin_addr.S_un.S_addr = inet_addr(addr.c_str());
    addrin.sin_family = family;
    addrin.sin_port = htons(port);
    ATMRGuard atm(this->RWLPoolSocket);
    int iResult = bind(this->PoolSocket.at(i), (SOCKADDR *) &addrin, sizeof (addrin));
    if (iResult == SOCKET_ERROR) {
//        stringstream ss("socket Bind SOCKET_ERROR ");
//        ss << WSAGetLastError();
//        throw std::runtime_error( ss );
        return false;
    }
    return true;
}


bool WinSocketPack::Bind(size_t i, WSP_INADDR addr, int port, WSP_AF family)
{
    sockaddr_in addrin;
    addrin.sin_addr.S_un.S_addr = htonl(addr);
    addrin.sin_family = family;
    addrin.sin_port = htons(port);
    ATMRGuard atm(this->RWLPoolSocket);
    int iResult = bind(this->PoolSocket.at(i), (SOCKADDR *) &addrin, sizeof (addrin));
    if (iResult == SOCKET_ERROR) {
//        stringstream ss("socket Bind SOCKET_ERROR ");
//        ss << WSAGetLastError();
//        throw std::runtime_error( ss );
        return false;
    }
    return true;
}


bool WinSocketPack::Listen(size_t i, int MaximumQueue)
{
    ATMRGuard atm(this->RWLPoolSocket);
    if ( listen(this->PoolSocket.at(i), MaximumQueue) == SOCKET_ERROR )
    {
        return false;
    }
    return true;
}


bool WinSocketPack::SetBlockingMode( size_t i, bool blocking)
{
    // int ioctlsocket(SOCKET s,__LONG32 cmd,u_long *argp);
    // 设置 socket I/O 模式

    // 第一个参数是需要设置的 socket 句柄
    // 当第二个参数是 FIONBIO 时    第三个参数决定是否开启阻塞模式     0 阻塞开启      非0 不阻塞
    // Note: 创建的新 socket 默认是阻塞的
    u_long mode = blocking ? 0 : 1;
    ATMRGuard atm(this->RWLPoolSocket);
    if ( ioctlsocket( this->PoolSocket.at(i), FIONBIO, &mode) == SOCKET_ERROR )
    {
        return false;
    }
    return true;
}


size_t WinSocketPack::Accept(size_t i)
{
    sockaddr_in addrin;
    int len=sizeof(sockaddr_in);
    ATMRGuard atm(this->RWLPoolSocket);
    SOCKET socketin = accept( this->PoolSocket.at(i), (SOCKADDR*)&addrin, &len);
    if ( socketin == INVALID_SOCKET )
    {
        return WSP_ACCEPT_ERROR;
    }
    {   /// ATMWGuard
        ATMWGuard atm(this->RWLPoolSocket);
        this->PoolSocket.push_back(socketin);
        return this->PoolSocket.size() - 1;
    }   /// ATMWGuard
}



bool WinSocketPack::Connect(size_t i, std::string addr, int port, WSP_AF family)
{
    sockaddr_in addrin;
    addrin.sin_addr.S_un.S_addr = inet_addr( addr.c_str() );
    addrin.sin_family = family;
    addrin.sin_port = htons(port);
    ATMRGuard atm(this->RWLPoolSocket);
    int iResult = connect(this->PoolSocket.at(i), (SOCKADDR *) &addrin, sizeof (addrin));
    if (iResult == SOCKET_ERROR) {
//        stringstream ss("socket Connect SOCKET_ERROR ");
//        ss << WSAGetLastError();
//        throw std::runtime_error( ss );
        return false;
    }
    return true;
}



bool WinSocketPack::Send(size_t i, std::string data)
{
    ATMRGuard atm(this->RWLPoolSocket);
    if ( send( this->PoolSocket.at(i), data.c_str(), data.size() + 1, 0) == SOCKET_ERROR )      // data.size() + 1 是为了送出 \0 字符
    {
        return false;
    }
    return true;
}


int WinSocketPack::Recv(size_t i, std::string& data, size_t maxlength)
{
    CharArray ca(maxlength);
    this->Recv( i, ca);
    data = ca.GetString();
    return 1;
}



int WinSocketPack::Recv(size_t i, CharArray& data)
{
    ATMRGuard atm(this->RWLPoolSocket);
    int r = recv( this->PoolSocket.at(i), data.GetPtr(), data.GetSize(), 0);
    if ( r == SOCKET_ERROR )      // 失败或正常中断
    {
        return -1;
    }
    if ( r == 0)
    {
        return 0;
    }
    return 1;
}


