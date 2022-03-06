#ifndef RECORD_HPP
#define RECORD_HPP

#include <map>
#include <string>

namespace CsvDb {

class Record
{
public:
  Record();
  virtual ~Record();

  const std::map<std::string, std::string>& getFields() const;
  void setFields(const std::map<std::string, std::string>& val);

  void setField(const std::string& field, const std::string& val);
  const std::string& getField(const std::string& field) const;
  bool hasField(const std::string& field) const;

  std::string toString() const;

private:
  std::map<std::string, std::string> mFields;
};
}  // namespace CsvDb

#endif
