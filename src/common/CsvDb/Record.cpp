#include "Record.hpp"

#include "StringThings.hpp"

namespace CsvDb {
Record::Record() :
    mFields()
{
}

Record::~Record()
{
}

const std::map<std::string, std::string>& Record::getFields() const
{
  return mFields;
}
void Record::setFields(const std::map<std::string, std::string>& val)
{
  mFields = val;
}

std::string Record::toString() const
{
  std::string result;
  for (const auto& [field, value] : mFields)
    result += "(" + field + ":" + value + ")";
  return result;
}

void Record::setField(const std::string& field, const std::string& val)
{
  mFields[field] = val;
}
const std::string& Record::getField(const std::string& field) const
{
  return mFields.at(field);
}
bool Record::hasField(const std::string& field) const
{
  return mFields.find(field) != mFields.end();
}
}  // namespace CsvDb
