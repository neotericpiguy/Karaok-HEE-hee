#ifndef DLINFO_HPP
#define DLINFO_HPP

#include <future>

namespace YoutubeDl {

class DlInfo
{
public:
  DlInfo();
  void start();

  int getThreadFuture();
  const std::future<int>& getDlThread();
  bool dlThreadValid() const;
  // void setDlThread(std::future<int>& val);

  std::string getLink() const;
  void setLink(const std::string& val);

  std::string getFilename() const;
  void setFilename(const std::string& val);

  std::string getSpeed() const;
  void setSpeed(const std::string& val);

  double getSize() const;
  void setSize(double val);

  double getStatus() const;
  void setStatus(double val);

  std::string getState() const;
  void setState(const std::string& val);

  std::string getEta() const;
  void setEta(const std::string& val);

  std::string getOutputDir() const;
  void setOutputDir(const std::string& val);

private:
  int downloadLink();
  std::future<int> dlThread;
  std::string link;
  std::string filename;
  std::string speed;
  double size;
  double status;
  std::string state;
  std::string eta;
  std::string outputDir;
};

}  // namespace YoutubeDl
#endif
