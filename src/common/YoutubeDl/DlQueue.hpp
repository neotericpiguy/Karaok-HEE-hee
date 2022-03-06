#ifndef DLQUEUE_HPP
#define DLQUEUE_HPP

#include <deque>
#include <future>
#include <map>

#include "DlInfo.hpp"
#include "Scpi.hpp"

namespace YoutubeDl {
class DlQueue
{
public:
  DlQueue();
  DlQueue(const DlQueue&) = delete;
  DlQueue& operator=(const DlQueue& env) = delete;
  ~DlQueue();

  void addToQueue(const std::string& link, bool addToFile = true);
  void serviceQueue();
  void resumeDownloads();

  unsigned int getRunningThreadCount() const;
  unsigned int getLinkQuequeSize() const;

  unsigned int getMaxThreads() const;
  std::string getOutputDir() const;
  unsigned int getOutputDirSize() const;
  std::string getOldestDownload() const;

  unsigned int getMaxOutputDirSize() const;

  void setMaxThreads(unsigned int threads);
  void setOutputDir(const std::string& dir);
  void setMaxOutputDirSize(unsigned int size);

  const std::string& getQueueFile() const;
  void setQueueFile(const std::string& val);

  const std::map<std::string, DlInfo>& getDlInfoMap() const;

  void stopDl(const std::string& link);

  std::string parseScpi(const Scpi& scpi);

  const std::function<void(const std::string& filename)>& getDownloadCompleteFunc() const;
  void setDownloadCompleteFunc(const std::function<void(const std::string& filename)>& val);

private:
  std::map<std::string, DlInfo> mDlInfoMap;
  std::deque<std::string> mLinkQueue;
  unsigned int mMaxThreads;
  mutable std::mutex mFileLock;
  std::string mOutputDir;
  unsigned int mMaxOutputDirSize;
  std::atomic<bool> mIsRunning;
  std::thread mDownloadMonitorThread;
  mutable std::mutex mLinkQueueLock;

  std::function<void(const std::string& filename)> mDownloadCompleteFunc;
  std::string mQueueFile;
};

}  // namespace YoutubeDl
#endif
