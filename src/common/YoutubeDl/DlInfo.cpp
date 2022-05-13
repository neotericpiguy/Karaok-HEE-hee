#include "DlInfo.hpp"

#include <array>

#include "StringThings.hpp"

namespace YoutubeDl {

void DlInfo::start()
{
  dlThread = std::async(std::launch::async, [this]() -> int {
    return downloadLink();
  });
}

DlInfo::DlInfo() :
    dlThread(),
    link(),
    filename(),
    speed(),
    size(),
    status(),
    state(),
    eta(),
    outputDir()
{
}

int DlInfo::downloadLink()
{
  std::string youtubeDlCmd = "cd " + outputDir + "; youtube-dl -w --no-mtime --restrict-filenames --newline -o '%(title)s.%(ext)s' ";
  std::string cmd = youtubeDlCmd + link;

  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }

  state = "Starting";
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    if (state == "Complete")
      continue;

    result = buffer.data();
    auto results = StringThings::strToVec(result, ' ');
    state = result;
    if (results.size() < 2)
    {
      continue;
    }

    if (results.at(1) == "Destination:")
    {
      filename = results[2].substr(0, results[2].length() - 1);
      continue;
    }

    // Last line printed by youtube-dl
    if (results.at(0) == "[ffmpeg]" || status == 100)
    {
      status = 100;
      state = "Complete";
    }

    if (results[0] != "[download]")
      continue;

    state = "Downloading";

    auto percentCompleteStr = results[1].substr(0, results[1].length() - 1);
    if (!StringThings::strTo(status, percentCompleteStr))
      continue;

    if (results.size() < 4)
      continue;

    auto sizeStr = results[3].substr(1, results[3].length() - 4);
    StringThings::strTo(size, sizeStr);

    if (results.size() < 6)
      continue;

    speed = results[5];

    if (results.size() < 8)
      continue;

    eta = results[7];
  }
  state = "Finished";

  return 0;
}

bool DlInfo::dlThreadValid() const
{
  return dlThread.valid();
}
int DlInfo::getThreadFuture()
{
  return dlThread.get();
}
const std::future<int>& DlInfo::getDlThread()
{
  return dlThread;
}

std::string DlInfo::getLink() const
{
  return link;
}

void DlInfo::setLink(const std::string& val)
{
  link = val;
}

std::string DlInfo::getFilename() const
{
  return filename;
}
void DlInfo::setFilename(const std::string& val)
{
  filename = val;
}

std::string DlInfo::getEta() const
{
  return eta;
}
std::string DlInfo::getOutputDir() const
{
  return outputDir;
}
double DlInfo::getSize() const
{
  return size;
}
std::string DlInfo::getSpeed() const
{
  return speed;
}
std::string DlInfo::getState() const
{
  return state;
}
double DlInfo::getStatus() const
{
  return status;
}
void DlInfo::setEta(const std::string& val)
{
  eta = val;
}
void DlInfo::setOutputDir(const std::string& val)
{
  outputDir = val;
}
void DlInfo::setSize(double val)
{
  size = val;
}
void DlInfo::setSpeed(const std::string& val)
{
  speed = val;
}
void DlInfo::setState(const std::string& val)
{
  state = val;
}
void DlInfo::setStatus(double val)
{
  status = val;
}
}  // namespace YoutubeDl
