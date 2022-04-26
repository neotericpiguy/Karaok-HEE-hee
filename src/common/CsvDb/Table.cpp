#include "Table.hpp"

#include <fstream>
#include <iostream>

#include "StringThings.hpp"

namespace CsvDb {
Table::Table(const std::string& tablePath, const std::string& key) :
    mTablePath(tablePath),
    mHeaders(),
    mTableRecords(),
    mKey(key),
    mNewRecordFunc(nullptr),
    mFileMutex(),
    mScpiFunctions(),
    mNewFileHeader()
{
  mScpiFunctions = {
      {"PRINT", [this](const Scpi&) -> std::string { return std::to_string(printTable()); }},
      {"LOAD", [this](const Scpi&) -> std::string { return std::to_string(loadTable()); }},
      {"CLEAR", [this](const Scpi&) -> std::string { return std::to_string(clearTable()); }},
      {"SAVE", [this](const Scpi&) -> std::string { return std::to_string(saveTable()); }},
      {"RM", [this](const Scpi& scpi) -> std::string {
         auto params = scpi.getParams();
         if (params.size() < 1)
           return "no params";

         std::vector<bool> resultVec;

         for (const auto& param : params)
           resultVec.push_back(removeRecord(param));
         saveTable();

         return StringThings::vecToStr(resultVec, ",");
       }},
  };
}

Table::~Table()
{
}

const std::string& Table::getTablePath() const
{
  return mTablePath;
}
void Table::setTablePath(const std::string& val)
{
  mTablePath = val;
}
const std::vector<std::string>& Table::getHeaders() const
{
  return mHeaders;
}
void Table::setHeaders(const std::vector<std::string>& val)
{
  mHeaders = val;
}
const std::map<std::string, Record*>& Table::getTableRecords() const
{
  return mTableRecords;
}
void Table::setTableRecords(const std::map<std::string, Record*>& val)
{
  mTableRecords = val;
}
const std::string& Table::getKey() const
{
  return mKey;
}
void Table::setKey(const std::string& val)
{
  mKey = val;
}

const std::function<Record*()>& Table::getNewRecordFunc() const
{
  return mNewRecordFunc;
}
void Table::setNewRecordFunc(const std::function<Record*()>& val)
{
  mNewRecordFunc = val;
}

size_t Table::loadTable()
{
  std::lock_guard<std::mutex> fileLock(mFileMutex);
  std::ifstream file(mTablePath);
  if (!file.is_open())
  {
    std::cerr << "Failed to open file: " << mTablePath << std::endl;
    std::ofstream newFile(mTablePath);
    newFile << StringThings::vecToStr(mNewFileHeader, ",");
    newFile.close();

    file.open(mTablePath);
  }

  clearTable();
  std::cout << "Loading mTablePath: " << mTablePath << std::endl;

  // Assume first row is always the mHeaders that describe  the data columns
  std::string temp;
  std::getline(file, temp);
  mHeaders = StringThings::strToVec(temp, ',', false);

  auto iter = std::find(mHeaders.begin(), mHeaders.end(), getKey());

  auto idIndex = std::distance(mHeaders.begin(), iter);

  while (std::getline(file, temp))
  {
    auto values = StringThings::strToVec(temp, ',', false);
    if (values.size() != mHeaders.size() || temp.empty())
    {
      std::cerr << "malformed data: " << temp << std::endl;
      continue;
    }

    if (getNewRecordFunc())
      mTableRecords[values[idIndex]] = getNewRecordFunc()();
    else
      mTableRecords[values[idIndex]] = new Record;

    for (unsigned int i = 0; i < values.size(); i++)
    {
      mTableRecords[values[idIndex]]->setField(mHeaders[i], values[i]);
    }
  }

  return mTableRecords.size();
}

size_t Table::printTable() const
{
  for (const auto& [id, song] : mTableRecords)
    std::cout << getTablePath() << ":" << song->toString() << std::endl;
  return mTableRecords.size();
}

size_t Table::saveTable() const
{
  std::lock_guard<std::mutex> fileLock(mFileMutex);
  std::ofstream file(mTablePath + ".new");
  if (!file.is_open())
  {
    std::cerr << "Failed to open file: " << mTablePath << std::endl;
    return 0;
  }

  file << mHeaders[0];
  for (unsigned int i = 1; i < mHeaders.size(); i++)
    file << "," << mHeaders.at(i);
  file << std::endl;

  for (const auto& [id, record] : mTableRecords)
  {
    file << id;
    for (unsigned int i = 1; i < mHeaders.size(); i++)
    {
      file << ",";
      if (record->hasField(mHeaders.at(i)))
        file << record->getField(mHeaders.at(i));
    }
    file << std::endl;
  }

  std::rename(std::string(mTablePath + ".new").c_str(), mTablePath.c_str());

  return mTableRecords.size();
}

std::string Table::getSha(const std::string& filename)
{
  std::string cmd = "sha256sum " + filename;
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    if (!result.empty())
      continue;
    result = buffer.data();
  }

  auto results = StringThings::strToVec(result, ' ');
  if (results.size() > 1)
    return results.at(0);
  return "";
}

bool Table::clearTable()
{
  for (auto& [id, song] : mTableRecords)
    delete song;
  mTableRecords.clear();
  return true;
}

bool Table::addRecord(Record* record)
{
  if (!record->hasField(getKey()))
    return false;

  auto key = record->getField(getKey());
  mTableRecords[key] = record;

  std::cout << getTablePath() << " Added: " + mTableRecords.at(key)->toString() << std::endl;
  return true;
}

bool Table::removeRecord(const std::string& key)
{
  if (mTableRecords.find(key) == mTableRecords.end())
    return false;

  delete mTableRecords.at(key);
  mTableRecords.erase(mTableRecords.find(key));
  return true;
}

bool Table::removeRecord(const Record& record)
{
  return removeRecord(record.getField(getKey()));
}

std::vector<const Record*> Table::findRecord(const std::string& criteria, const std::vector<std::string>& searchKeys) const
{
  auto newCriteria = criteria;
  StringThings::toLower(newCriteria);

  std::vector<const Record*> songs;
  for (const auto& [id, value] : mTableRecords)
  {
    std::string data;
    for (const auto& searchKey : searchKeys)
      data += value->getField(searchKey);
    StringThings::toLower(data);

    if (data.find(newCriteria) == std::string::npos)
      continue;

    songs.push_back(mTableRecords.at(id));
  }
  return songs;
}

Record* Table::getRecord(const std::string& id)
{
  if (mTableRecords.find(id) == mTableRecords.end())
    return nullptr;
  return mTableRecords.at(id);
}

std::string Table::sha256(const std::string& plain)
{
  std::string cmd = "echo -n \"" + plain + "\" | sha256sum";
  std::array<char, 128> buffer;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  std::string result;
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    if (result.empty())
      result = buffer.data();
  }
  auto results = StringThings::strToVec(result, ' ');

  return results[0];
}

void Table::addScpiFunction(const std::string& scpiCmd, std::function<std::string(const Scpi&)> func)
{
  mScpiFunctions[scpiCmd] = func;
}

void Table::addScpiFunctions(const std::map<std::string, std::function<std::string(const Scpi&)>>& funcs)
{
  for (const auto& [key, func] : funcs)
    addScpiFunction(key, func);
}

std::string Table::parseScpi(const Scpi& scpi)
{
  const auto& cmd = scpi.getCommand();
  if (mScpiFunctions.find(cmd) == mScpiFunctions.end())
    return "Unknown command: \"" + scpi.getScpiStr() + "\" -> \"" + cmd + "\"\n";

  return mScpiFunctions.at(cmd)(scpi);
}

const std::vector<std::string>& Table::getNewFileHeader() const
{
  return mNewFileHeader;
}
void Table::setNewFileHeader(const std::vector<std::string>& val)
{
  mNewFileHeader = val;
}
}  // namespace CsvDb
