#ifndef WINSOCKETPACK_H
#define WINSOCKETPACK_H

// Winsock2 TCP 客户端代码
#include <Winsock2.h>       // 记得包含库 ws2_32
//#pragma comment(lib,"ws2_32.lib")

#include <stdexcept> // 抛出错误 runtime_error

// C++11 Pthread 库
//#include <thread>
#include <mutex>
#include <atomic>

#include "CharArray.h"

#include "RWLock.h"

//#include <vector>
#include <deque>
#include <string>





/// WinSocket 包装类
class WinSocketPack
{
    public:

        enum WSP_AF{

            WSP_AF_UNSPEC = 0,
            WSP_AF_UNIX = 1,
            WSP_AF_INET = 2,
            WSP_AF_IMPLINK = 3,
            WSP_AF_PUP = 4,
            WSP_AF_CHAOS = 5,
            WSP_AF_NS = 6,
            WSP_AF_IPX = AF_NS,
            WSP_AF_ISO = 7,
            WSP_AF_OSI = AF_ISO,
            WSP_AF_ECMA = 8,
            WSP_AF_DATAKIT = 9,
            WSP_AF_CCITT = 10,
            WSP_AF_SNA = 11,
            WSP_AF_DECnet = 12,
            WSP_AF_DLI = 13,
            WSP_AF_LAT = 14,
            WSP_AF_HYLINK = 15,
            WSP_AF_APPLETALK = 16,
            WSP_AF_NETBIOS = 17,
            WSP_AF_VOICEVIEW = 18,
            WSP_AF_FIREFOX = 19,
            WSP_AF_UNKNOWN1 = 20,
            WSP_AF_BAN = 21,
            WSP_AF_ATM = 22,
            WSP_AF_INET6 = 23,
            WSP_AF_CLUSTER = 24,
            WSP_AF_12844 = 25,
            WSP_AF_IRDA = 26,
            WSP_AF_NETDES = 28,
            WSP_AF_TCNPROCESS = 29,
            WSP_AF_TCNMESSAGE = 30,
            WSP_AF_ICLFXBM = 31,
            WSP_AF_BTH = 32,
            WSP_AF_MAX = 33

        };

        enum WSP_TYPE {

            WSP_SOCK_STREAM = 1,
            WSP_SOCK_DGRAM = 2,
            WSP_SOCK_RAW = 3,
            WSP_SOCK_RDM = 4,
            WSP_SOCK_SEQPACKET = 5

        };

        enum WSP_PROTOCOL {

            WSP_IPPROTO_IP = 0,
            WSP_IPPROTO_HOPOPTS = 0,
            WSP_IPPROTO_ICMP = 1,
            WSP_IPPROTO_IGMP = 2,
            WSP_IPPROTO_GGP = 3,
            WSP_IPPROTO_IPV4 = 4,
            WSP_IPPROTO_TCP = 6,
            WSP_IPPROTO_PUP = 12,
            WSP_IPPROTO_UDP = 17,
            WSP_IPPROTO_IDP = 22,
            WSP_IPPROTO_IPV6 = 41,
            WSP_IPPROTO_ROUTING = 43,
            WSP_IPPROTO_FRAGMENT = 44,
            WSP_IPPROTO_ESP = 50,
            WSP_IPPROTO_AH = 51,
            WSP_IPPROTO_ICMPV6 = 58,
            WSP_IPPROTO_NONE = 59,
            WSP_IPPROTO_DSTOPTS = 60,
            WSP_IPPROTO_ND = 77,
            WSP_IPPROTO_ICLFXBM = 78,
            WSP_IPPROTO_RAW = 255,
            WSP_IPPROTO_MAX = 256

        };

        enum WSP_INADDR {
            WSP_INADDR_ANY = INADDR_ANY,
            WSP_INADDR_LOOPBACK = INADDR_LOOPBACK,
            WSP_INADDR_BROADCAST = INADDR_BROADCAST,
            WSP_INADDR_NONE = INADDR_NONE

        };

        enum WSP_SHUTDOWN {
            WSP_SD_RECEIVE = SD_RECEIVE,
            WSP_SD_SEND = SD_SEND,
            WSP_SD_BOTH = SD_BOTH
        };

        int static const WSP_SOMAXCONN = 0x7fffffff;
        size_t static const WSP_ACCEPT_ERROR = ~0;

        // 单例 禁止重复构造
        WinSocketPack()  throw (std::runtime_error);
        // 禁用拷贝与移动函数
        WinSocketPack( WinSocketPack const & ) = delete;
        WinSocketPack & operator= ( WinSocketPack const &) = delete;
        WinSocketPack( WinSocketPack &&) = delete;
        WinSocketPack & operator= ( WinSocketPack && ) = delete;

        virtual ~WinSocketPack();

        SOCKET GetSocket( size_t i);
        size_t GetPoolSize( );
        bool EmptyPool( );

        void CloseSocket( size_t i);
        bool Shutdown( size_t i, WSP_SHUTDOWN how);

        size_t NewSocket( WSP_AF af, WSP_TYPE type, WSP_PROTOCOL protocol );
        size_t NewSocketTCP();

        bool Bind( size_t i, std::string addr, int port, WSP_AF family);
        bool Bind( size_t i, WSP_INADDR addr, int port, WSP_AF family);

        bool Listen( size_t i, int MaximumQueue);

        bool SetBlockingMode( size_t i, bool blocking);

        // Note: Accept 失败返回 WSP_ACCEPT_ERROR
        size_t Accept( size_t i);

        bool Connect(  size_t i, std::string addr, int port, WSP_AF family);

        bool Send( size_t i, std::string data);
        int Recv( size_t i, std::string &data, size_t maxlength = 100);     // -1 SOCKET_ERROR    0 正常结束  1 成功
        int Recv( size_t i, CharArray &data);     // -1 SOCKET_ERROR    0 正常结束  1 成功
    protected:
    private:
        std::deque<SOCKET> PoolSocket;  // SOCKET 句柄池      TODOED 已经增加多线程支持
        // 尚未实现重用功能

//        // deque 添加元素不移动位置 vector有可能 效能相近
//        // 因为 SOCKET 实为句柄 【unsigned __int64 or unsigned __int】
//        // 并且没有删除操作

//        // 所有访问都加锁  反正除了send & recv & Accept 不会出现频繁访问

//        std::mutex MTXPoolSocket;   // 对 PoolSocket 的互斥锁
//        //std::lock_guard<std::mutex> mtxp(MTXPoolSocket);

        /// PoolSocket 自写“高性能”读写保护原子锁
        RWLock RWLPoolSocket;


};


#endif // WINSOCKETPACK_H
