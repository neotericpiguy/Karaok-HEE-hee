#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

/** \brief Custom Tcp Server written in c for C++17
 *
 * c++ wrapper over the c library
 */
class TcpServer
{
public:
  /** Default Constructor */
  TcpServer();
  /** Construct a TcpServer */
  TcpServer(std::function<std::vector<char>(const std::vector<char>&)> callback, uint32_t port);
  ~TcpServer();

  /** Start the TcpServer
   *
   * Should be non blocking
   * */
  bool start();

  /** Stop a TcpServer */
  bool stop();

  /** Check if TcpServer is running*/
  bool isRunning() const;

  /** Get rx stats */
  long getRxCount() const;
  /** Get tx stats */
  long getTxCount() const;

  /** Set the port for tcp listening */
  bool setPort(uint32_t port);
  /** Set how to handle each request */
  bool setCallback(std::function<std::vector<char>(const std::vector<char>&)> callback);

private:
  int run();
  int cleanWorkerThreads();

  std::thread mServerThread;
  std::vector<std::thread> mWorkerThreads;

  std::atomic<bool> mIsRunning;
  std::function<std::vector<char>(const std::vector<char>&)> mCallback;
  std::atomic<long> mRxCount;
  std::atomic<long> mTxCount;
  uint32_t mPort;
  int mSockFd;
  mutable std::mutex mCoreMutex;
};

#endif

