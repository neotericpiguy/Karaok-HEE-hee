#include "TcpServer.hpp"

TcpServer::TcpServer() :
    TcpServer(nullptr, 0)
{
}

TcpServer::TcpServer(std::function<std::vector<char>(const std::vector<char>&)> callback, uint32_t port) :
    mServerThread(),
    mWorkerThreads(),
    mIsRunning(false),
    mCallback(callback),
    mRxCount(0),
    mTxCount(0),
    mPort(port),
    mSockFd(-1),
    mCoreMutex()
{
}

TcpServer::~TcpServer()
{
}

bool TcpServer::start()
{
  std::lock_guard<std::mutex> locker(mCoreMutex);
  if (mIsRunning)
    return false;

  setbuf(stdout, NULL);
  mIsRunning = true;

  struct sockaddr_in servAddr = sockaddr_in();

  mSockFd = socket(AF_INET, SOCK_STREAM, 0);

  if (mSockFd < 0)
  {
    printf("ERROR opening socket\n");
    return false;
  }

  // Keep Tcp port alive
  int yes = 1;
  setsockopt(mSockFd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));

  int idle = 1;
  setsockopt(mSockFd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));

  int interval = 1;
  setsockopt(mSockFd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

  int maxpkt = 10;
  setsockopt(mSockFd, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));

  // Important for rebinding
  constexpr int optVal = 1;
  constexpr int backlog = 5;
  setsockopt(mSockFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(mPort);

  if (bind(mSockFd, reinterpret_cast<sockaddr*>(&servAddr), sizeof(servAddr)) < 0)
  {
    printf(__FILE__ ": ERROR on binding\n");
    mIsRunning = false;
    return false;
  }

  fcntl(mSockFd, F_SETFL, O_NONBLOCK);

  if (listen(mSockFd, backlog) == -1)
  {
    printf(__FILE__ ": ERROR on listen\n");
    mIsRunning = false;
    return false;
  }

  mServerThread = std::thread{[&]() { run(); }};

  return true;
}

int TcpServer::run()
{
  // http://www.linuxhowtos.org/data/6/server.c
  // http://www.bogotobogo.com/cplusplus/sockets_server_client.php

  // There is a max scpi size. If you need to send something raw do it over
  // rsync or scp
  constexpr int bufferSize = 4096 * 1024;
  constexpr int maxThreads = 50;
  constexpr uint32_t sleep = 50;

  struct timeval timeout;
  fd_set readFds;
  fd_set masterFds;

  FD_ZERO(&masterFds);
  FD_SET(mSockFd, &masterFds);

  struct sockaddr_storage remoteaddr;  // client address
  socklen_t addrlen = sizeof remoteaddr;

  while (mIsRunning)
  {
    // http://beej.us/guide/bgnet/html/multi/advanced.html
    readFds = masterFds;
    timeout.tv_sec = 0;
    timeout.tv_usec = sleep;

    if (select(mSockFd + 1, &readFds, NULL, NULL, &timeout) < 0)
    {
      printf(__FILE__ ":select failed");
      continue;
    }

    if (FD_ISSET(mSockFd, &readFds) == 0)
      continue;

    int clientFd = accept(mSockFd, reinterpret_cast<struct sockaddr*>(&remoteaddr), &addrlen);
    if (clientFd == -1)
    {
      printf(__FILE__ ": accept failed");
      continue;
    }

    auto listenFunc = [this, clientFd]() -> void {
      mRxCount++;

      char buffer[bufferSize];
      int nBytes;

      if ((nBytes = recv(clientFd, buffer, sizeof buffer, 0)) <= 0)
      {
        printf("selectserver: socket %d hung up\n", clientFd);
        close(clientFd);
        return;
      }

      std::vector<char> data(buffer, buffer + nBytes);

      std::vector<char> result = mCallback(data);

      if (send(clientFd, result.data(), result.size(), 0) == -1)
        perror("Failed to send,");
      close(clientFd);

      mTxCount++;
      return;
    };

    mWorkerThreads.push_back(std::thread(listenFunc));

    while (mWorkerThreads.size() >= maxThreads)
    {
      cleanWorkerThreads();
      // You have to put a little sleep or else it will max out cpu usage
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
  }

  ::close(mSockFd);

  return true;
}

bool TcpServer::isRunning() const
{
  return mIsRunning;
}

long TcpServer::getRxCount() const
{
  return mRxCount;
}

long TcpServer::getTxCount() const
{
  return mTxCount;
}

int TcpServer::cleanWorkerThreads()
{
  auto predicate = [&](std::thread& t) {
    // just because a thread is joinable doesn't mean it has completed running it's code
    if (!t.joinable())
      return false;
    t.join();
    return true;
  };

  mWorkerThreads.erase(std::remove_if(mWorkerThreads.begin(), mWorkerThreads.end(), predicate), mWorkerThreads.end());

  return 0;
}

bool TcpServer::stop()
{
  if (!mIsRunning)
    return false;

  printf("Stopping TcpServer\n");
  mIsRunning = false;

  if (mServerThread.joinable())
    mServerThread.join();

  while (mWorkerThreads.size())
  {
    printf("Cleaning worker threads %d\r\n", static_cast<int>(mWorkerThreads.size()));
    cleanWorkerThreads();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return true;
}

bool TcpServer::setPort(uint32_t port)
{
  mPort = port;
  return true;
}

bool TcpServer::setCallback(std::function<std::vector<char>(const std::vector<char>&)> callback)
{
  mCallback = callback;
  return true;
}

