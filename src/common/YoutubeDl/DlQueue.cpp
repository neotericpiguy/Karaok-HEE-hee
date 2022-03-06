#include "DlQueue.hpp"

#include <iostream>

#include "StringThings.hpp"

namespace YoutubeDl {

DlQueue::DlQueue() :
    mDlInfoMap(),
    mLinkQueue(),
    mMaxThreads(2),
    mFileLock(),
    mOutputDir("/mnt/sdb/karaoke"),
    mMaxOutputDirSize(100000000),
    mIsRunning(false),
    mDownloadMonitorThread(),
    mLinkQueueLock(),
    mDownloadCompleteFunc(nullptr),
    mQueueFile("current")
{
}

DlQueue::~DlQueue()
{
}

void DlQueue::setMaxThreads(unsigned int threads)
{
  mMaxThreads = threads;
}

void DlQueue::setMaxOutputDirSize(unsigned int size)
{
  mMaxOutputDirSize = size;
}

unsigned int DlQueue::getMaxOutputDirSize() const
{
  return mMaxOutputDirSize;
}

void DlQueue::setOutputDir(const std::string& dir)
{
  mOutputDir = dir;
}

unsigned int DlQueue::getMaxThreads() const
{
  return mMaxThreads;
}

std::string DlQueue::getOutputDir() const
{
  return mOutputDir;
}

const std::string& DlQueue::getQueueFile() const
{
  return mQueueFile;
}
void DlQueue::setQueueFile(const std::string& val)
{
  mQueueFile = val;
}

const std::function<void(const std::string& filename)>& DlQueue::getDownloadCompleteFunc() const
{
  return mDownloadCompleteFunc;
}
void DlQueue::setDownloadCompleteFunc(const std::function<void(const std::string& filename)>& val)
{
  mDownloadCompleteFunc = val;
}

unsigned int DlQueue::getOutputDirSize() const
{
  std::string cmd = "du " + mOutputDir;

  std::array<char, 4096> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result = buffer.data();
    auto results = StringThings::strToVec(result, '\t');
    if (results.size() == 2)
    {
      if (results.at(1).find(mOutputDir) != std::string::npos)
      {
        unsigned int val;
        if (StringThings::strTo(val, results.at(0)))
          return val;
      }
    }
  }
  return 0;
}

std::string DlQueue::getOldestDownload() const
{
  std::string cmd = "ls --color=no -t " + mOutputDir + "/*.mp4";

  std::array<char, 4096> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result = buffer.data();

    if (!result.empty())  // must let ls output finish or else broken pipe
      continue;
  }
  return result;
}

unsigned int DlQueue::getLinkQuequeSize() const
{
  return mLinkQueue.size();
}

void DlQueue::addToQueue(const std::string& link, bool addToFile)
{
  if (link.empty())
    return;

  // std::lock_guard<std::mutex> queueLock(mLinkQueueLock);
  if (mDlInfoMap.find(link) != mDlInfoMap.end())
    return;

  mLinkQueue.push_back(link);
  if (addToFile)
  {
    std::lock_guard<std::mutex> lock(mFileLock);
    std::ofstream stream(mQueueFile, std::ios_base::app);
    stream << link << std::endl;
  }

  return;
}

void DlQueue::resumeDownloads()
{
  std::vector<std::string> vec;
  {
    std::lock_guard<std::mutex> lock(mFileLock);
    std::ifstream istream(mQueueFile);
    std::string link;
    while (std::getline(istream, link))
      vec.push_back(link);
  }

  for (const auto& link : vec)
    addToQueue(link, false);

  if (mIsRunning)
    return;

  mIsRunning = true;

  mDownloadMonitorThread = std::thread([this]() {
    while (mIsRunning)
    {
      serviceQueue();
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
  });
}

void DlQueue::serviceQueue()
{
  // Add new downloads if Max Threads allows
  while (getRunningThreadCount() < mMaxThreads && !mLinkQueue.empty())
  {
    auto link = mLinkQueue.at(0);
    {
      //      std::lock_guard<std::mutex> queueLock(mLinkQueueLock);
      mLinkQueue.pop_front();
    }

    if (mDlInfoMap.find(link) != mDlInfoMap.end())
      continue;

    auto& info = mDlInfoMap[link];
    info.setLink(link);
    info.setOutputDir(mOutputDir);
    info.start();
  }

  if (mLinkQueue.empty())
    resumeDownloads();

  // Invalidate threads that are done
  for (auto dlInfoMapIter = mDlInfoMap.begin(); dlInfoMapIter != mDlInfoMap.end();)
  {
    auto& link = dlInfoMapIter->first;
    auto& info = dlInfoMapIter->second;

    if (!info.getDlThread().valid())
    {
      dlInfoMapIter++;
      continue;
    }

    auto status = info.getDlThread().wait_for(std::chrono::milliseconds(0));
    // If thread is done
    if (status != std::future_status::ready)
    {
      dlInfoMapIter++;
      continue;
    }

    if (info.getState() == "Complete" || info.getStatus() == 100)
    {
      {
        std::lock_guard<std::mutex> lock(mFileLock);
        std::ifstream istream(mQueueFile);
        std::ofstream ofstream(mQueueFile + ".temp");

        std::string temp;
        while (std::getline(istream, temp))
        {
          if (temp.find(link) == std::string::npos)
            ofstream << temp << std::endl;
        }
        std::rename(std::string(mQueueFile + ".temp").c_str(), mQueueFile.c_str());
      }
      info.setStatus(100);
    }
    else
    {
      info.setState("Incomplete :" + std::to_string(info.getStatus()));
    }

    info.getThreadFuture();
    if (mDownloadCompleteFunc)
    {
      auto filename = info.getFilename();

      std::string filenameNoExt = filename;
      StringThings::replaceRegex(filenameNoExt, "\\..*$", "");

      std::string cmd = "ls --color=no " + getOutputDir() + "/" + filenameNoExt + ".*";
      std::array<char, 4096> buffer;
      std::string result;
      std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
      if (!pipe)
        throw std::runtime_error("popen() failed!");

      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      {
        result = buffer.data();
      }
      result = result.substr(0, result.length());
      StringThings::replace(result, "\r", "");
      StringThings::replace(result, "\n", "");

      std::cout << "Ls: result : " << result << std::endl;

      mDownloadCompleteFunc(result);
    }

    dlInfoMapIter = mDlInfoMap.erase(dlInfoMapIter);
  }

  // Clean out old files that make to big
  if (getOutputDirSize() > getMaxOutputDirSize())
  {
    std::string cmd = "rm " + getOldestDownload();
    system(cmd.c_str());
  }

  return;
}

unsigned int DlQueue::getRunningThreadCount() const
{
  int result = mDlInfoMap.size();
  // Check if any threads are done
  for (const auto& [link, info] : mDlInfoMap)
  {
    if (!info.dlThreadValid())
    {
      result--;
      continue;
    }
  }
  return result;
}

const std::map<std::string, DlInfo>& DlQueue::getDlInfoMap() const
{
  return mDlInfoMap;
}

void DlQueue::stopDl(const std::string& link)
{
  std::string cmd = "ps aux";
  std::array<char, 4096> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result = buffer.data();
    auto results = StringThings::strToVec(result, ' ');
    auto lastStr = results[results.size() - 1];
    StringThings::replace(lastStr, "\n", "");
    if (lastStr != link)
      continue;

    std::string killCmd = "kill -9 " + results[1];
    system(killCmd.c_str());
  }
}

std::string DlQueue::parseScpi(const Scpi& scpi)
{
  const std::map<std::string, std::function<std::string()>> functionMap = {
      {"MAXTHREADS?", [this]() { return std::to_string(getMaxThreads()); }},
      {"THREADS?", [this]() { return std::to_string(getRunningThreadCount()); }},
      {"OUTPUTDIR?", [this]() { return getOutputDir(); }},
      {"OUTPUTDIRSIZE?", [this]() { return std::to_string(getOutputDirSize()); }},
      {"OLDESTDOWNLOAD?", [this]() { return getOldestDownload(); }},
      {"MAXOUTPUTDIRSIZE?", [this]() { return std::to_string(getMaxOutputDirSize()); }},
      {"QUEUE?", [this]() { return std::to_string(getLinkQuequeSize()); }},
      {"LIST?", [this]() {
         std::vector<std::string> dlLinks;
         for (const auto& [link, info] : getDlInfoMap())
           dlLinks.push_back(link);

         return StringThings::vecToStr(dlLinks, "\n");
       }},
      {"MAXTHREADS", [this, &scpi]() -> std::string {
         auto params = scpi.getParams();
         unsigned int val;
         if (!scpi.getParam(val, 0))
           return "poor param";
         setMaxThreads(val);
         return std::to_string(getMaxThreads());
       }},
      {"OUTPUTDIR", [this, &scpi]() -> std::string {
         auto params = scpi.getParams();
         std::string val;
         if (!scpi.getParam(val, 0))
           return "poor param";
         setOutputDir(val);
         return getOutputDir();
       }},
      {"MAXOUTPUTDIRSIZE", [this, &scpi]() -> std::string {
         auto params = scpi.getParams();
         unsigned int val;
         if (!scpi.getParam(val, 0))
           return "poor param";
         setMaxOutputDirSize(val);
         return std::to_string(getMaxOutputDirSize());
       }},
      {"ADD", [this, &scpi]() -> std::string {
         std::string val;
         if (!scpi.getParam(val, 0))
           return "poor param";
         addToQueue(val);
         return val;
       }},
      {"STOP", [this, &scpi]() -> std::string {
         std::string val;
         if (!scpi.getParam(val, 0))
           return "poor param";
         stopDl(val);
         return val;
       }},
  };

  const auto& cmd = scpi.getCommand();
  if (functionMap.find(cmd) == functionMap.end())
    return "Unknown command: \"" + scpi.getScpiStr() + "\" -> \"" + cmd + "\"\n";

  return functionMap.at(cmd)();
}

}  // namespace YoutubeDl
