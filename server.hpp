#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <ctime>
#include <functional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <typeinfo>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

#define INF 0
#define DBG 1
#define ERR 2
#define LOG_LEVEL DBG

#define LOG(level, format, ...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (level < LOG_LEVEL)                                                                                         \
            break;                                                                                                     \
        time_t t = time(NULL);                                                                                         \
        struct tm *ltm = localtime(&t);                                                                                \
        char tmp[32] = {0};                                                                                            \
        strftime(tmp, 31, "%H:%M:%S", ltm);                                                                            \
        fprintf(stdout, "[%p %s %s:%d] " format "\n", (void *)pthread_self(), tmp, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define INF_LOG(format, ...) LOG(INF, format, ##__VA_ARGS__)
#define DBG_LOG(format, ...) LOG(DBG, format, ##__VA_ARGS__)
#define ERR_LOG(format, ...) LOG(ERR, format, ##__VA_ARGS__)
#define BUFFER_DEFAULT_SIZE 1024
class Buffer
{
private:
    std::vector<char> _buffer;
    uint64_t _reader_idx;
    uint64_t _writer_idx;

public:
    Buffer() : _buffer(BUFFER_DEFAULT_SIZE), _reader_idx(0), _writer_idx(0) {}
    char *Begin() // 缓冲区的开始,这里应该返回什么呢?
    {
        // return &*_buffer.begin();
        // 这里可以用data(),直接获取指向数组的指针
        return _buffer.data();
    }
    char *WritePosition() // 返回写的位置
    {
        return Begin() + _writer_idx;
    }
    char *ReadPosition()
    {
        return Begin() + _reader_idx;
    }
    uint64_t TailIdleSize() // 返回末尾的空闲区域大小
    {
        return &*_buffer.end() - WritePosition();
    }
    uint64_t HeadIdleSize() // 头部空闲区域大小
    {
        return _reader_idx;
    }
    uint64_t ReadAbleSize() // 可读区域大小
    {
        return _writer_idx - _reader_idx;
    }
    void MoveReadOffset(uint64_t len) // 读后移
    {
        if (!len)
        {
            return;
        }
        assert(len <= TailIdleSize());
        _reader_idx += len;
    }
    void MoveWriteOffset(uint64_t len) // 写后移
    {
        assert(len >= ReadAbleSize());
        _writer_idx += len;
    }
    void EnsureWriteSpace(uint64_t len) // 确保写入空间足够
    {
        if (len < TailIdleSize() + HeadIdleSize())
        {
            if (len > TailIdleSize())
            {
                uint64_t rsz = ReadAbleSize();
                std::copy(ReadPosition(), ReadPosition() + rsz, Begin());
                _reader_idx = 0;
                _writer_idx = rsz;
            }
            else
            {
                return;
            }
        }
        else
        {
            _buffer.resize(_writer_idx + len);
        }
    }
    void Write(const void *data, uint64_t len) // 写入
    {
        if (len == 0)
        {
            return;
        }
        EnsureWriteSpace(len);
        const char *d = (const char *)data;
        std::copy(d, d + len, WritePosition());
    }
    void WriteAndPush(const void *data, uint64_t len)
    {
        Write(data, len);
        MoveWriteOffset(len);
    }
    void WriteString(const std::string &data)
    {
        Write(data.c_str(), data.size());
        return;
    }
    void WriteStringAndPush(const std::string &data)
    {
        WriteString(data);
        MoveWriteOffset(data.size());
        return;
    }
    void WriteBuffer(Buffer &data)
    {
        Write(data.ReadPosition(), data.ReadAbleSize());
    }
    void WriteBufferAndPush(Buffer &data)
    {
        WriteBuffer(data);
        MoveWriteOffset(data.ReadAbleSize());
    }
    void Read(void *buf, uint64_t len)
    {
        assert(len <= ReadAbleSize());
        std::copy(ReadPosition(), ReadPosition() + len, (char *)buf);
    }
    void ReadAndPop(void *buf, uint64_t len)
    {
        Read(buf, len);
        MoveReadOffset(len);
    }
    std::string ReadAsString(uint64_t len)
    {
        assert(len <= ReadAbleSize());
        std::string ans;
        ans.resize(len);
        Read(&ans[0], len); // mark
        return ans;
    }
    std::string ReadAsStringAndPop(uint64_t len)
    {
        std::string ans = ReadAsString(len);
        MoveReadOffset(len);
        return ans;
    }
    char *FindCRLF()
    {
        char *res = (char *)memchr(ReadPosition(), '\n', ReadAbleSize());
        return res;
    }
    std::string GetLine()
    {
        char *pos = FindCRLF();
        if (pos == NULL)
        {
            return "";
        }
        // +1是为了把换行字符也取出来。
        return ReadAsString(pos - ReadPosition() + 1);
    }
    std::string GetLineAndPop()
    {
        std::string ans = GetLine();
        MoveReadOffset(ans.size());
        return ans;
    }
    void Clear()
    {
        _reader_idx = 0;
        _writer_idx = 0;
    }
};

#define MAX_LISTEN 1024
class Socket
{
private:
    int _sockfd;

public:
    Socket() : _sockfd(-1) {}
    Socket(int fd) : _sockfd(fd) {}
    ~Socket()
    {
        std::cout << "socket close~" << std::endl;
        Close();
    }
    int Fd() { return _sockfd; }
    // 创建套接字
    bool Create()
    {
        // int socket(int domain, int type, int protocol)
        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sockfd < 0)
        {
            ERR_LOG("创建失败了f!!");
            return false;
        }
        return true;
    }
    // 绑定地址信息
    bool Bind(const std::string &ip, uint16_t port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;                    // 设置IPV4
        addr.sin_port = htons(port);                  // 设置端口号
        addr.sin_addr.s_addr = inet_addr(ip.c_str()); // 设置IP地址(点分十进制->网络字节序)
        socklen_t len = sizeof(struct sockaddr_in);
        // int bind(int sockfd, struct sockaddr*addr, socklen_t len);
        int ret = bind(_sockfd, (struct sockaddr *)&addr, len);
        if (ret < 0)
        {
            ERR_LOG("BIND ADDRESS FAILED!");
            return false;
        }
        return true;
    }
    // 开始监听
    bool Listen(int backlog = MAX_LISTEN)
    {
        // int listen(int backlog)
        int ret = listen(_sockfd, backlog);
        if (ret < 0)
        {
            ERR_LOG("SOCKET LISTEN FAILED!");
            return false;
        }
        return true;
    }
    // 向服务器发起连接
    bool Connect(const std::string &ip, uint16_t port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        socklen_t len = sizeof(struct sockaddr_in);
        // int connect(int sockfd, struct sockaddr*addr, socklen_t len);
        int ret = connect(_sockfd, (struct sockaddr *)&addr, len);
        if (ret < 0)
        {
            ERR_LOG("CONNECT SERVER FAILED!");
            return false;
        }
        return true;
    }
    // 获取新连接
    int Accept()
    {
        // int accept(int sockfd, struct sockaddr *addr, socklen_t *len);
        int newfd = accept(_sockfd, NULL, NULL);
        if (newfd < 0)
        {
            ERR_LOG("SOCKET ACCEPT FAILED!");
            return -1;
        }
        return newfd;
    }
    // 接收数据
    ssize_t Recv(void *buf, size_t len, int flag = 0)
    {
        // ssize_t recv(int sockfd, void *buf, size_t len, int flag);
        ssize_t ret = recv(_sockfd, buf, len, flag);
        if (ret <= 0)
        {
            // EAGAIN 当前socket的接收缓冲区中没有数据了，在非阻塞的情况下才会有这个错误
            // EINTR  表示当前socket的阻塞等待，被信号打断了，
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0; // 表示这次接收没有接收到数据
            }
            ERR_LOG("SOCKET RECV FAILED!!");
            return -1;
        }
        return ret; // 实际接收的数据长度
    }
    ssize_t NonBlockRecv(void *buf, size_t len)
    {
        return Recv(buf, len, MSG_DONTWAIT); // MSG_DONTWAIT 表示当前接收为非阻塞。
    }
    // 发送数据
    ssize_t Send(const void *buf, size_t len, int flag = 0)
    {
        // ssize_t send(int sockfd, void *data, size_t len, int flag);
        ssize_t ret = send(_sockfd, buf, len, flag);
        if (ret < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            ERR_LOG("SOCKET SEND FAILED!!");
            return -1;
        }
        return ret; // 实际发送的数据长度
    }
    ssize_t NonBlockSend(void *buf, size_t len)
    {
        if (len == 0)
            return 0;
        return Send(buf, len, MSG_DONTWAIT); // MSG_DONTWAIT 表示当前发送为非阻塞。
    }
    // 关闭套接字
    void Close()
    {
        if (_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }
    // 创建一个服务端连接
    bool CreateServer(uint16_t port, const std::string &ip = "0.0.0.0", bool block_flag = false)
    {
        // 1. 创建套接字，2. 绑定地址，3. 开始监听，4. 设置非阻塞， 5. 启动地址重用
        if (Create() == false)
            return false;
        if (block_flag)
            NonBlock();
        if (Bind(ip, port) == false)
            return false;
        if (Listen() == false)
            return false;
        ReuseAddress();
        return true;
    }
    // 创建一个客户端连接
    bool CreateClient(uint16_t port, const std::string &ip)
    {
        // 1. 创建套接字，2.指向连接服务器
        if (Create() == false)
            return false;
        if (Connect(ip, port) == false)
            return false;
        return true;
    }
    // 设置套接字选项---开启地址端口重用
    void ReuseAddress()
    {
        // int setsockopt(int fd, int leve, int optname, void *val, int vallen)
        int val = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
        val = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, (void *)&val, sizeof(int));
    }
    // 设置套接字阻塞属性-- 设置为非阻塞
    void NonBlock()
    {
        // int fcntl(int fd, int cmd, ... /* arg */ );
        int flag = fcntl(_sockfd, F_GETFL, 0);
        fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
    }
};
class Poller;
class EventLoop;
class Channel
{
private:
    int _fd;
    EventLoop *_loop;
    uint32_t _events;  // 当前需要监控的事件
    uint32_t _revents; // 当前连接触发的事件
    using EventCallback = std::function<void()>;
    EventCallback _read_callback;  // 可读事件被触发的回调函数
    EventCallback _write_callback; // 可写事件被触发的回调函数
    EventCallback _error_callback; // 错误事件被触发的回调函数
    EventCallback _close_callback; // 连接断开事件被触发的回调函数
    EventCallback _event_callback; // 任意事件被触发的回调函数
public:
    Channel(EventLoop *loop, int fd) : _fd(fd), _events(0), _revents(0), _loop(loop) {}
    int Fd() { return _fd; }
    uint32_t Events() { return _events; }
    void SetRevents(uint32_t events) { _revents = events; }
    void SetReadCallback(const EventCallback &cb) { _read_callback = cb; }
    void SetWriteCallback(const EventCallback &cb) { _write_callback = cb; }
    void SetErrorCallback(const EventCallback &cb) { _error_callback = cb; }
    void SetCloseCallback(const EventCallback &cb) { _close_callback = cb; }
    void SetEventCallback(const EventCallback &cb) { _event_callback = cb; }
    bool ReadAble() { return (_events & EPOLLIN); }
    bool WriteAble() { return (_events & EPOLLOUT); }
    void Remove();
    void Update();
    void EnableRead() // 启动读事件监控
    {
        _events |= EPOLLIN;
        Update();
    }
    void EnableWrite() // 启动写事件监控
    {
        _events |= EPOLLOUT;
        Update();
    }
    void DisableRead() // 关闭读事件监控
    {
        _events &= ~EPOLLIN;
        Update();
    }
    void DisableWrite() // 关闭写事件监控
    {
        _events &= ~EPOLLOUT;
        Update();
    }
    void CloseAll() // 关闭所有事件监控
    {
        _events = 0;
        Update();
    }
    void HandleEvent()
    {
        if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
        {
            // 可读事件的回调是一定触发的
            if (_read_callback)
                _read_callback();
        }
        if (_revents & EPOLLOUT)
        {
            if (_write_callback)
                _write_callback();
        }
        else if (_revents & EPOLLERR)
        {
            // 一旦出错，就会释放连接，因此要放到前边调用任意回调
            if (_error_callback)
                _error_callback();
        }
        else if (_revents & EPOLLHUP)
        {
            if (_close_callback)
                _close_callback();
        }
        if (_event_callback)
            _event_callback();
    }
};
#define MAX_EPOLLEVENTS 1024
class Poller
{
private:
    int _epfd;
    struct epoll_event _evs[MAX_EPOLLEVENTS];
    std::unordered_map<int, Channel *> _channels;
    void Update(Channel *channel, int op)
    {
        int fd = channel->Fd();
        struct epoll_event ev; // 直接访问用户空间数据可能导致安全问题,故这里使用临时变量来传入内核
        ev.data.fd = fd;
        ev.events = channel->Events();
        int ret = epoll_ctl(_epfd, op, fd, &ev);
        if (ret < 0)
        {
            ERR_LOG("EPOLLCTL FAILED!");
        }
        return;
    }
    bool HashChannel(Channel *channel)
    {
        auto it = _channels.find(channel->Fd());
        if (it == _channels.end())
        {
            return false;
        }
        return true;
    }

public:
    Poller()
    {
        _epfd = epoll_create(MAX_EPOLLEVENTS);
        if (_epfd < 0)
        {
            ERR_LOG("EPOLL CREATE FAILED!!");
            abort();
        }
    }
    // 添加或修改监控事件
    void UpdateEvent(Channel *channel)
    {
        bool ret = HashChannel(channel);
        if (!ret)
        {
            _channels.insert(std::make_pair(channel->Fd(), channel));
            Update(channel, EPOLL_CTL_ADD);
            return;
        }

        Update(channel, EPOLL_CTL_MOD);
    }
    // 移除监控
    void RemoveEvent(Channel *channel)
    {
        bool ret = HashChannel(channel);
        if (!ret)
            return;
        else
            _channels.erase(channel->Fd());
        Update(channel, EPOLL_CTL_DEL);
    }
    // 开始监控，返回活跃连接
    void Poll(std::vector<Channel *> *active)
    {
        int ew = epoll_wait(_epfd, _evs, MAX_EPOLLEVENTS, -1);
        if (ew < 0)
        {
            if (errno == EINTR)
            {
                return;
            }
            ERR_LOG("EPOLL WAIT ERROR:%s\n", strerror(errno));
            abort();
        }
        for (int i = 0; i < ew; ++i)
        {
            auto it = _channels.find(_evs[i].data.fd);
            assert(it != _channels.end());
            it->second->SetRevents(_evs[i].events);
            active->push_back(it->second);
        }
        return;
    }
};
using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;
class TimerTask
{
private:
    uint64_t _id;         // 定时器任务对象ID
    uint32_t _timeout;    // 定时任务的超时时间
    bool _canceled;       // false-表示没有被取消， true-表示被取消
    TaskFunc _task_cb;    // 定时器对象要执行的定时任务
    ReleaseFunc _release; // 用于删除TimerWheel中保存的定时器对象信息
public:
    TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb) : _id(id), _timeout(delay), _task_cb(cb), _canceled(false) {}
    ~TimerTask()
    {
        if (_canceled == false)
            _task_cb();
        _release();
    }
    void Cancel() { _canceled = true; }
    void SetRelease(const ReleaseFunc &cb) { _release = cb; }
    uint32_t DelayTime() { return _timeout; }
};

class TimerWheel
{
private:
    using WeakTask = std::weak_ptr<TimerTask>;
    using PtrTask = std::shared_ptr<TimerTask>;
    int _tick;     // 当前的秒针，走到哪里释放哪里，释放哪里，就相当于执行哪里的任务
    int _capacity; // 表盘最大数量---其实就是最大延迟时间
    std::vector<std::vector<PtrTask>> _wheel;
    std::unordered_map<uint64_t, WeakTask> _timers;

    EventLoop *_loop;
    int _timerfd; // 定时器描述符--可读事件回调就是读取计数器，执行定时任务
    std::unique_ptr<Channel> _timer_channel;

private:
    void RemoveTimer(uint64_t id)
    {
        auto it = _timers.find(id);
        if (it != _timers.end())
        {
            _timers.erase(it);
        }
    }
    static int CreateTimerfd()
    {
        int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timerfd < 0)
        {
            ERR_LOG("TIMERFD CREATE FAILED!");
            abort();
        }
        // int timerfd_settime(int fd, int flags, struct itimerspec *new, struct itimerspec *old);
        struct itimerspec itime;
        itime.it_value.tv_sec = 1;
        itime.it_value.tv_nsec = 0; // 第一次超时时间为1s后
        itime.it_interval.tv_sec = 1;
        itime.it_interval.tv_nsec = 0; // 第一次超时后，每次超时的间隔时
        timerfd_settime(timerfd, 0, &itime, NULL);
        return timerfd;
    }
    int ReadTimefd()
    {
        uint64_t times;
        // 有可能因为其他描述符的事件处理花费事件比较长，然后在处理定时器描述符事件的时候，有可能就已经超时了很多次
        // read读取到的数据times就是从上一次read之后超时的次数
        int ret = read(_timerfd, &times, 8);
        if (ret < 0)
        {
            ERR_LOG("READ TIMEFD FAILED!");
            abort();
        }
        return times;
    }
    // 这个函数应该每秒钟被执行一次，相当于秒针向后走了一步
    void RunTimerTask()
    {
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear(); // 清空指定位置的数组，就会把数组中保存的所有管理定时器对象的shared_ptr释放掉
    }
    void OnTime()
    {
        // 根据实际超时的次数，执行对应的超时任务
        int times = ReadTimefd();
        for (int i = 0; i < times; i++)
        {
            RunTimerTask();
        }
    }
    void TimerAddInLoop(uint64_t id, uint32_t delay, const TaskFunc &cb)
    {
        PtrTask pt(new TimerTask(id, delay, cb));
        pt->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));
        int pos = (_tick + delay) % _capacity;
        _wheel[pos].push_back(pt);
        _timers[id] = WeakTask(pt);
    }
    void TimerRefreshInLoop(uint64_t id)
    {
        // 通过保存的定时器对象的weak_ptr构造一个shared_ptr出来，添加到轮子中
        auto it = _timers.find(id);
        if (it == _timers.end())
        {
            return; // 没找着定时任务，没法刷新，没法延迟
        }
        PtrTask pt = it->second.lock(); // lock获取weak_ptr管理的对象对应的shared_ptr
        int delay = pt->DelayTime();
        int pos = (_tick + delay) % _capacity;
        _wheel[pos].push_back(pt);
    }
    void TimerCancelInLoop(uint64_t id)
    {
        auto it = _timers.find(id);
        if (it == _timers.end())
        {
            return; // 没找着定时任务，没法刷新，没法延迟
        }
        PtrTask pt = it->second.lock();
        if (pt)
            pt->Cancel();
    }

public:
    TimerWheel(EventLoop *loop) : _capacity(60), _tick(0), _wheel(_capacity), _loop(loop),
                                  _timerfd(CreateTimerfd()), _timer_channel(new Channel(_loop, _timerfd))
    {
        _timer_channel->SetReadCallback(std::bind(&TimerWheel::OnTime, this));
        _timer_channel->EnableRead(); // 启动读事件监控
    }
    /*定时器中有个_timers成员，定时器信息的操作有可能在多线程中进行，因此需要考虑线程安全问题*/
    /*如果不想加锁，那就把对定期的所有操作，都放到一个线程中进行*/
    void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb);
    // 刷新/延迟定时任务
    void TimerRefresh(uint64_t id);
    void TimerCancel(uint64_t id);
    /*这个接口存在线程安全问题--这个接口实际上不能被外界使用者调用，只能在模块内，在对应的EventLoop线程内执行*/
    bool HasTimer(uint64_t id)
    {
        auto it = _timers.find(id);
        if (it == _timers.end())
        {
            return false;
        }
        return true;
    }
};

class EventLoop
{
private:
    using function = std::function<void()>;
    std::thread::id _thread_id; // 线程ID
    int _event_fd;              // eventfd唤醒IO事件监控有可能导致的阻塞
    std::unique_ptr<Channel> _event_channel;
    Poller _poller;               // 进行所有描述符的事件监控
    std::vector<function> _tasks; // 任务池
    std::mutex _mutex;            // 实现任务池操作的线程安全
    TimerWheel _timer_wheel;      // 定时器模块

public:
    EventLoop() : _thread_id(std::this_thread::get_id()),
                  _event_fd(CreateEventFd()),
                  _event_channel(new Channel(this, _event_fd)),
                  _timer_wheel(this)
    {
        // 这里为什么要设置读回调呢?让我查查代码......
        // 这里是把std::bind(&EventLoop::ReadEventfd, this)作为参数传进去...
        // 也就是让_read_callback设为std::bind(&EventLoop::ReadEventfd, this)
        // std::bind(&EventLoop::ReadEventfd, this)这个函数等效于ReadEventfd()?
        // 也就是说这是把_read_callback设为ReadEventfd()了
        // 那...别的不需要这样设置吗?
        // mark一下
        _event_channel->SetReadCallback(std::bind(&EventLoop::ReadEventfd, this));
        _event_channel->EnableRead();
    }
    ~EventLoop() { close(_event_fd); }
    // void quit() { _quit = true; WeakUpEventfd(); }
    void RunAllTasks()
    {
        // if (_tasks.empty()) return; // 错误写法,不加锁的情况下直接访问_task线程不安全
        std::vector<function> tmp;
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            // if (_tasks.empty()) return; // 这才是正确写法,但这里仅仅是先让他能跑就行了,暂不优化过多
            _tasks.swap(tmp);
        }
        for (auto &f : tmp)
        {
            f();
        }
        return;
    }
    void Start()
    {
        while (1)
        {
            std::vector<Channel *> actives;
            _poller.Poll(&actives);
            // 处理事件
            for (auto &f : actives)
            {
                f->HandleEvent();
            }
            RunAllTasks();
        }
    }
    static int CreateEventFd()
    {
        int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (efd < 0)
        {
            ERR_LOG("CREATE EVENTFD FAILED!!");
            abort(); // 让程序异常退出
        }
        return efd;
    }
    // 读eventfd
    void ReadEventfd()
    {
        uint64_t res = 0;
        int ret = read(_event_fd, &res, sizeof(res));
        if (ret < 0)
        {
            if (errno == EINTR || errno == EAGAIN)
            {
                return;
            }
            ERR_LOG("READ EVENTFD FAILED!");
            abort();
        }
        return;
    }
    // 写入eventfd
    void WeakUpEventfd()
    {
        uint64_t val = 1;
        int ret = write(_event_fd, &val, sizeof(val));
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                return;
            }
            ERR_LOG("READ EVENTFD FAILED!");
            abort();
        }
        return;
    }
    bool IsInLoop()
    {
        return (_thread_id == std::this_thread::get_id());
    }
    void AssertInLoop()
    {
        assert(_thread_id == std::this_thread::get_id());
    }
    // 判断将要执行的任务是否处于当前线程中，如果是则执行，不是则压入队列。
    void RunInLoop(const function &cb)
    {
        if (IsInLoop())
        {
            return cb();
        }
        return QueueInLoop(cb);
    }
    // 压入队列
    void QueueInLoop(const function &cb)
    {
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            _tasks.push_back(cb);
        }
        WeakUpEventfd(); // 我不禁思考,为什么要在这里写入eventfd?
        return;
    }
    void UpdateEvent(Channel *channel) { return _poller.UpdateEvent(channel); }
    // 移除描述符的监控
    void RemoveEvent(Channel *channel) { return _poller.RemoveEvent(channel); }
    void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb) 
    { return _timer_wheel.TimerAdd(id, delay, cb); }
    void TimerRefresh(uint64_t id) { return _timer_wheel.TimerRefresh(id); }
    void TimerCancel(uint64_t id) { return _timer_wheel.TimerCancel(id); }
    bool HasTimer(uint64_t id) { return _timer_wheel.HasTimer(id); }
};
void Channel::Remove() { return _loop->RemoveEvent(this); }
void Channel::Update() { return _loop->UpdateEvent(this); }

class Any
{
private:
    class holder
    {
    public:
        virtual ~holder() {}
        virtual const std::type_info &type() = 0;
        virtual holder *clone() = 0;
    };
    template <class T>
    class placeholder : public holder
    {
    public:
        placeholder(const T &val) : _val(val) {}
        // 获取子类对象保存的数据类型
        virtual const std::type_info &type() { return typeid(T); }
        // 针对当前的对象自身，克隆出一个新的子类对象
        virtual holder *clone() { return new placeholder(_val); }

    public:
        T _val;
    };
    holder *_content;

public:
    Any() : _content(NULL) {}
    template <class T>
    Any(const T &val) : _content(new placeholder<T>(val)) {}
    Any(const Any &other) : _content(other._content ? other._content->clone() : NULL) {}
    ~Any() { delete _content; }

    Any &swap(Any &other)
    {
        std::swap(_content, other._content);
        return *this;
    }

    // 返回子类对象保存的数据的指针
    template <class T>
    T *get()
    {
        // 想要获取的数据类型，必须和保存的数据类型一致
        assert(typeid(T) == _content->type());
        return &((placeholder<T> *)_content)->_val;
    }
    // 赋值运算符的重载函数
    template <class T>
    Any &operator=(const T &val)
    {
        // 为val构造一个临时的通用容器，然后与当前容器自身进行指针交换，临时对象释放的时候，原先保存的数据也就被释放
        Any(val).swap(*this);
        return *this;
    }
    Any &operator=(const Any &other)
    {
        Any(other).swap(*this);
        return *this;
    }
};

class Connection;
typedef enum
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
} ConnStatu;
using PtrConnection = std::shared_ptr<Connection>;
class Connection : public std::enable_shared_from_this<Connection>
{
    uint64_t _conn_id;             // Connection的文件描述符
    int _sockfd;                   // 连接关联的文件描述符
    bool _enable_inactive_release; // 是否启动非活跃销毁
    EventLoop *_loop;
    ConnStatu _statu; // Q:这个statu和之前的_enable_inactive_release的区别是什么呢?
    Socket _socket;
    Channel _channel;
    Buffer _in_buffer;
    Buffer _out_buffer;
    Any _context; // 请求的接受处理上下文

    using ConnectCallback = std::function<void(const PtrConnection &)>;
    using MessageCallback = std::function<void(const PtrConnection &, Buffer *)>;
    using ClosedCallback = std::function<void(const PtrConnection &)>;
    using AnyEventCallback = std::function<void(const PtrConnection &)>;

    ConnectCallback _connected_callback;
    MessageCallback _message_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _event_callback;
    ClosedCallback _server_closed_callback;

private:
    // 回调函数设置
    void HandleRead()
    {
        // 接受socket的数据放在缓冲区
        char buff[65536];
        ssize_t ret = _socket.NonBlockRecv(buff, 65535); // 非阻塞接受
        if (ret < 0)
        {
            return ShutdownInLoop();
        }
        // 写入,然后把写偏移后移
        _in_buffer.WriteAndPush(buff, ret);
        // 调用message_callback处理
        if (_in_buffer.ReadAbleSize() > 0)
        {
            return _message_callback(shared_from_this(), &_in_buffer);
        }
    }

    void HaneldWrite()
    {
        // out_buffer里面存了要发送的数据
        ssize_t ret = _socket.NonBlockSend(_out_buffer.ReadPosition(), _out_buffer.ReadAbleSize());
        if (ret < 0)
        {
            // 这里为什么要这么写呢
            if (_in_buffer.ReadAbleSize() > 0)
            {
                _message_callback(shared_from_this(), &_in_buffer);
            }
            return Release();
        }
        // 读偏移后移
        _out_buffer.MoveReadOffset(ret);
    }

    void HandleClose();
    void HandleError();
    void HandleEvent();
    void EstablishedInLoop();
    void ReleaseInLoop();
    void SendInLoop(Buffer &buf)
    {
        if (_statu == DISCONNECTED)
        {
            return;
        }
        _out_buffer.WriteBufferAndPush(buf);
        if (_channel.WriteAble() == false)
        {
            // 在这里启动写事件监控
            _channel.EnableWrite();
        }
    }
    void ShutdownInloop()
    {
        _statu = DISCONNECTING; // Q:DISCONNECTING和DISCONNECTED的区别是什么呢
        if (_in_buffer.ReadAbleSize() > 0)
        {
            if (_message_callback)
            {
                _message_callback(shared_from_this(), &_in_buffer);
                // Q:这个share_from_this又是什么......
            }
        }
    }
    // 释放(释放什么的?)
    void ReleaseInLoop()
    {
        _statu = DISCONNECTED;
        _channel.Remove();
        _socket.Close();
        if (_loop->HasTimer(_conn_id))
        {
            CancelInactiveReleaseInLoop();
            // 为什么是取消而不是执行呢?
        }
        if (_closed_callback)
        {
            _closed_callback(shared_from_this());
        }
        if (_server_closed_callback)
        {
            _server_closed_callback(shared_from_this());
        }
    }
    void EnableInactiveReleaseInLoop(int sec)
    {
        // 1. 将判断标志 _enable_inactive_release 置为true
        _enable_inactive_release = true;
        // 刷新延迟
        if (_loop->HasTimer(_conn_id))
        {
            return _loop->TimerRefresh(_conn_id);
        }
        _loop->TimerAdd(_conn_id, sec, std::bind(&Connection::Release, this)); // 这里面的realese是什么来着???
    }
    void CancelInactiveReleaseInLoop()
    {
        _enable_inactive_release = false;
        if (_loop->HasTimer(_conn_id))
        {
            return _loop->TimerCancel(_conn_id);
        }
    }
    void UpgradeInLoop(const Any &context,
                       const ConnectedCallback &conn,
                       const MessageCallback &msg,
                       const ClosedCallback &closed,
                       const AnyEventCallback &event)
    {
        _context = context;
        _connected_callback = conn;
        _message_callback = msg;
        _closed_callback = closed;
        _event_callback = event;
    }

public:
    Connection(EventLoop *loop, uint64_t conn_id, int sockfd) : 
    _conn_id(conn_id), _sockfd(sockfd), _enable_inactive_release(false),
    _loop(loop), _statu(CONNECTING), _socket(_sockfd), _channel(loop, _sockfd)
    {
        _channel.SetCloseCallback(std::bind(&Connection::HandleClose, this));
        _channel.SetEventCallback(std::bind(&Connection::HandleEvent, this));
        _channel.SetReadCallback(std::bind(&Connection::HandleRead, this));
        _channel.SetWriteCallback(std::bind(&Connection::HandleWrite, this));
        _channel.SetErrorCallback(std::bind(&Connection::HandleError, this));
    }
    ~Connection() {}
    int fd() { return _sockfd; }
    int id() { return _conn_id; }
    bool Connected() { return (_statu == CONNECTED); }
    void SetContext(const Any &context) { _context = context; }
    Any GetContext() { return _context; }
    void SetConnectedCallback(const ConnectedCallback &cb) { _connected_callback = cb; }
    void SetMessageCallback(const MessageCallback &cb) { _message_callback = cb; }
    void SetClosedCallback(const ClosedCallback &cb) { _closed_callback = cb; }
    void SetAnyEventCallback(const AnyEventCallback &cb) { _event_callback = cb; }
    void SetSrvClosedCallback(const ClosedCallback &cb) { _server_closed_callback = cb; }

    // 发送数据,把数据放到发送缓冲区
    void Send(const char *data, sizze_t len)
    {
        // Q:这里是怎么启动写事件监控的呢......
        Buffer buf;
        buf.WriteAndPush(data, len);
        _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, std::move(buf)));
    }
    // 关闭?
    void Shutdown()
    {
        _loop->RunInLoop(std::bind(&Connection::ShutdownInLoop, this));
    }
    void Release()
    {
        _loop->QueueInLoop(std::bind(&Connection::ReleaseInLoop, this));
    }
    void EnableInactiveRelease(int sec)
    {
        _loop->RunInLoop(std::bind(&Connection::EnableInactiveReleaseInLoop, this, sec));
    }
    // 取消非活跃销毁
    void CancelInactiveRelease()
    {
        _loop->RunInLoop(std::bind(&Connection::CancelInactiveReleaseInLoop, this));
    }
    // 切换协议---重置上下文以及阶段性回调处理函数 -- 而是这个接口必须在EventLoop线程中立即执行
    // 防备新的事件触发后，处理的时候，切换任务还没有被执行--会导致数据使用原协议处理了。
    void Upgrade(const Any &context, const ConnectedCallback &conn, const MessageCallback &msg,
                 const ClosedCallback &closed, const AnyEventCallback &event)
    {
        _loop->AssertInLoop();
        _loop->RunInLoop(std::bind(&Connection::UpgradeInLoop, this, context, conn, msg, closed, event));
    }
};
