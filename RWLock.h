#ifndef RWLOCK_H
#define RWLOCK_H

/**
  *|多读者多写者锁【互斥锁】
  *|
  *|允许读取方多次加锁
  *|可选择性允许写入方多次加锁【设置启动写入互斥锁(AreLockWriter = true)则对写入者之间进行排斥】
  *|
  *|本锁适合对STL容器进行多线程保护
  *|
  */

// C++11 Pthread 库
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds milliseconds microseconds
#include <mutex>
#include <atomic>

class ATMRGuard;     // 读者护盾
class ATMWGuard;     // 写者护盾
class ATMRTRGuard;   // 读者临时释放守卫
class ATMWTRGuard;   // 写者临时释放守卫

class RWLock    /// 多读者多写者锁
{
    public:
        RWLock( bool AreLockWriter):LockWriter(AreLockWriter){}
        ~RWLock() = default;
        RWLock( RWLock const & other) = delete;
        RWLock &operator=( RWLock const & other) = delete;
        RWLock( RWLock && other) = delete;
        RWLock &operator=( RWLock && other) = delete;

        // 让操作类能够访问私有成员
        friend ATMRGuard;
        friend ATMWGuard;
        friend ATMRTRGuard;
        friend ATMWTRGuard;
    protected:
    private:

        // 是否加写入锁
        bool LockWriter;

        /// 所需的原子锁

        // 只在准备写入时阻止读取  读取者等待其为false
        std::atomic_bool ATMStopReader;
        // 读者数量记录器
        std::atomic_long ATMNumReader;
        // 只在准备读取时阻止写入 写入者等待其为false
        std::atomic_bool ATMStopWriter;
        // 写者数量记录器
        std::atomic_long ATMNumWriter;
        // 写者 互斥锁
        std::mutex MTXWriter;
        // 保护信号量    去除隐藏的判断时被打断问题
        std::mutex MTXATM;
        // Peterson 死锁谦让保护
        std::atomic_bool TurnR;  // true 读者先入       false 写者先入
};

// Peterson 算法

class ATMRGuard     // 读者守卫
{
    public:

        inline
        ATMRGuard( RWLock &rwLock) : RWL(rwLock)
        {
            {
                std::lock_guard<std::mutex> lg(RWL.MTXATM);
                // guard
                RWL.ATMStopWriter = true;   // 禁止写者
                RWL.TurnR = false;   // 送给写者
            }
            while ( true )
            {
                {
                    std::lock_guard<std::mutex> lg(RWL.MTXATM);
                    // guard
                    if ( !(          ( RWL.ATMStopReader == true && RWL.TurnR == false )        ||       ( 0 != RWL.ATMNumWriter )               ) )
                        // 用了"!"非 所以中间是循环条件
                    {
                        ++RWL.ATMNumReader;
                        break;      // 跳出循环
                    }
                }
                // Wait
//                std::this_thread::sleep_for (std::chrono::microseconds(1));
                std::this_thread::sleep_for (std::chrono::milliseconds(1));
            }
            return;
        }

        inline
        ~ATMRGuard()
        {
            std::lock_guard<std::mutex> lg(RWL.MTXATM);
            // guard
            --RWL.ATMNumReader;
            RWL.ATMStopWriter = false;  // 允许写者
        }

        ATMRGuard( ATMRGuard const & other) = delete;
        ATMRGuard &operator=( ATMRGuard const & other) = delete;
        ATMRGuard( ATMRGuard && other) = delete;
        ATMRGuard &operator=( ATMRGuard && other) = delete;
    protected:
    private:
        RWLock &RWL;
};


class ATMWGuard     // 写者守卫
{
    public:

        inline
        ATMWGuard( RWLock &rwLock) : RWL(rwLock)
        {
            {
                std::lock_guard<std::mutex> lg(RWL.MTXATM);
                // guard
                RWL.ATMStopReader = true;   // 禁止读者
                RWL.TurnR = true;   // 送给读者
            }
            while ( true  )
            {
                {
                    std::lock_guard<std::mutex> lg(RWL.MTXATM);
                    // guard
                    if ( !(          ( RWL.ATMStopWriter == true && RWL.TurnR == true )        ||       ( 0 != RWL.ATMNumReader )               ) )
                        // 用了"!"非 所以中间是循环条件
                    {
                        ++RWL.ATMNumWriter;
                        break;      // 跳出循环
                    }
                }
                // Wait
                std::this_thread::sleep_for (std::chrono::milliseconds(1));
            }
//            ++RWL.ATMNumWriter;

            if (RWL.LockWriter)
            {
                RWL.MTXWriter.lock();
            }
            return;
        }

        inline
        ~ATMWGuard()
        {
            if (RWL.LockWriter)
            {
                RWL.MTXWriter.unlock();
            }

            std::lock_guard<std::mutex> lg(RWL.MTXATM);
            // guard
            --RWL.ATMNumWriter;
            RWL.ATMStopReader = false;  // 允许读者
        }

        ATMWGuard( ATMWGuard const & other) = delete;
        ATMWGuard &operator=( ATMWGuard const & other) = delete;
        ATMWGuard( ATMWGuard && other) = delete;
        ATMWGuard &operator=( ATMWGuard && other) = delete;
    protected:
    private:
        RWLock &RWL;
};


// 临时释放守卫   和加锁守卫操作正好相反
// 性质也相反    产生一个无守卫空窗
// Temporarily Released

class ATMRTRGuard   // 读者临时释放守卫
{
    public:

        inline
        ATMRTRGuard( RWLock &rwLock) : RWL(rwLock)
        {
            std::lock_guard<std::mutex> lg(RWL.MTXATM);
            --RWL.ATMNumReader;
            RWL.ATMStopWriter = false;  // 允许写者
            return;
        }

        inline
        ~ATMRTRGuard()
        {
            {
                std::lock_guard<std::mutex> lg(RWL.MTXATM);
                // guard
                RWL.ATMStopWriter = true;   // 禁止写者
                RWL.TurnR = false;   // 送给写者
            }
            while ( true )
            {
                {
                    std::lock_guard<std::mutex> lg(RWL.MTXATM);
                    // guard
                    if ( !(          ( RWL.ATMStopReader == true && RWL.TurnR == false )        ||       ( 0 != RWL.ATMNumWriter )               ) )
                        // 用了"!"非 所以中间是循环条件
                    {
                        ++RWL.ATMNumReader;
                        break;      // 跳出循环
                    }
                }
                // Wait
//                std::this_thread::sleep_for (std::chrono::microseconds(1));
                std::this_thread::sleep_for (std::chrono::milliseconds(1));
            }
            return;
        }

        ATMRTRGuard( ATMRTRGuard const & other) = delete;
        ATMRTRGuard &operator=( ATMRTRGuard const & other) = delete;
        ATMRTRGuard( ATMRTRGuard && other) = delete;
        ATMRTRGuard &operator=( ATMRTRGuard && other) = delete;
    protected:
    private:
        RWLock &RWL;
};


class ATMWTRGuard   // 写者临时释放守卫
{
    public:

        inline
        ATMWTRGuard( RWLock &rwLock) : RWL(rwLock)
        {
            if (RWL.LockWriter)
            {
                RWL.MTXWriter.unlock();
            }

            std::lock_guard<std::mutex> lg(RWL.MTXATM);
            --RWL.ATMNumWriter;
            RWL.ATMStopReader = false;  // 允许读者
            return;
        }

        inline
        ~ATMWTRGuard()
        {
            {
                std::lock_guard<std::mutex> lg(RWL.MTXATM);
                // guard
                RWL.ATMStopReader = true;   // 禁止读者
                RWL.TurnR = true;   // 送给读者
            }
            while ( true  )
            {
                {
                    std::lock_guard<std::mutex> lg(RWL.MTXATM);
                    // guard
                    if ( !(          ( RWL.ATMStopWriter == true && RWL.TurnR == true )        ||       ( 0 != RWL.ATMNumReader )               ) )
                        // 用了"!"非 所以中间是循环条件
                    {
                        ++RWL.ATMNumWriter;
                        break;      // 跳出循环
                    }
                }
                // Wait
                std::this_thread::sleep_for (std::chrono::milliseconds(1));
            }
//            ++RWL.ATMNumWriter;

            if (RWL.LockWriter)
            {
                RWL.MTXWriter.lock();
            }
            return;
        }

        ATMWTRGuard( ATMWTRGuard const & other) = delete;
        ATMWTRGuard &operator=( ATMWTRGuard const & other) = delete;
        ATMWTRGuard( ATMWTRGuard && other) = delete;
        ATMWTRGuard &operator=( ATMWTRGuard && other) = delete;
    protected:
    private:
        RWLock &RWL;
};



#endif // RWLOCK_H
