#include <iostream>

#include <thread>
#include <mutex>
#include <deque>
#include <vector>

#include <string>
#include <sstream>
using namespace std;

#include "WinSocketPack.h"

WinSocketPack WSP;

string host = "127.0.0.1";
int port = 9898;

void ToSend( int i)
{
    stringstream ss;
    ss << i;

    size_t soh = WSP.NewSocketTCP();
    WSP.Connect( soh, host, port, WinSocketPack::WSP_AF::WSP_AF_INET);      // 默认模式是同步阻塞

    // 发送
    WSP.Send( soh, ss.str());

    // 礼貌地关闭
    WSP.Shutdown(soh, WinSocketPack::WSP_SHUTDOWN::WSP_SD_BOTH);
    return;
}

int main()
{
    cout << "Hello world!" << endl;

    deque<thread> tpp;

    for (int i = 0; i != 1000; ++i)
    {
        tpp.emplace_back(ToSend, i);
    }

    for ( thread &a : tpp )
    {
        a.join();
    }

    return 0;
}
