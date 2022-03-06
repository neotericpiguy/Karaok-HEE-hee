#ifndef SCPI_HPP
#define SCPI_HPP

#include <string>
#include <vector>

// subsystem:command paramters
// DLQUEUE:MAXTHREADS 123
// DLQUEUE:MAXTHREADS?
class Scpi
{
public:
  Scpi(const std::string& scpi);
  Scpi(const Scpi&) = delete;
  Scpi& operator=(const Scpi& env) = delete;
  ~Scpi();

  const std::string& getScpiStr() const;
  void setScpiStr(const std::string& val);

  const std::vector<std::string>& getParams() const;
  void setParams(const std::vector<std::string>& val);

  const std::string& getCommand() const;
  void setCommand(const std::string& val);

  const std::vector<std::string>& getSubsystems() const;
  void setSubSystem(const std::vector<std::string>& val);

  template <typename T>
  bool getParam(T& retval, unsigned int index) const;

private:
  std::string mScpiStr;
  std::vector<std::string> mParams;
  std::string mCommand;
  std::vector<std::string> mSubsystem;
};

#endif
