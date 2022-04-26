#ifndef TABLE_HPP
#define TABLE_HPP

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Record.hpp"
#include "Scpi.hpp"

namespace CsvDb {
class Table
{
public:
  Table(const std::string& tablePath, const std::string& key);
  virtual ~Table();

  const std::string& getTablePath() const;
  void setTablePath(const std::string& val);

  const std::vector<std::string>& getHeaders() const;
  void setHeaders(const std::vector<std::string>& val);

  const std::map<std::string, Record*>& getTableRecords() const;
  void setTableRecords(const std::map<std::string, Record*>& val);

  const std::string& getKey() const;
  void setKey(const std::string& val);

  const std::function<Record*()>& getNewRecordFunc() const;
  void setNewRecordFunc(const std::function<Record*()>& val);

  const std::vector<std::string>& getNewFileHeader() const;
  void setNewFileHeader(const std::vector<std::string>& val);

  virtual size_t loadTable();
  size_t printTable() const;
  size_t saveTable() const;
  bool clearTable();

  bool addRecord(Record* record);
  bool removeRecord(const std::string& id);
  bool removeRecord(const Record& record);

  Record* getRecord(const std::string& id);

  std::vector<const Record*> findRecord(const std::string& criteria, const std::vector<std::string>& searchKeys) const;

  std::string parseScpi(const Scpi& scpi);
  void addScpiFunction(const std::string&, std::function<std::string(const Scpi&)>);
  void addScpiFunctions(const std::map<std::string, std::function<std::string(const Scpi&)>>& funcs);

  static std::string getSha(const std::string& filename);
  static std::string sha256(const std::string& plain);

private:
  std::string mTablePath;
  std::vector<std::string> mHeaders;
  std::map<std::string, Record*> mTableRecords;

  // unique column of table that should be used for the key for mTableRecords
  std::string mKey;
  std::function<Record*()> mNewRecordFunc;
  mutable std::mutex mFileMutex;
  std::map<std::string, std::function<std::string(const Scpi&)>> mScpiFunctions;

  std::vector<std::string> mNewFileHeader;
};
}  // namespace CsvDb

#endif
