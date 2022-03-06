#ifndef USER_HPP
#define USER_HPP

#include <map>
#include <string>
#include <vector>

#include "Record.hpp"

class User : public CsvDb::Record
{
public:
  User();
  const std::string& getUsername() const;
  void setUsername(const std::string& val);

  const std::string& getPasswordHash() const;
  void setPasswordHash(const std::string& val);

  std::vector<std::string> getRoles() const;
  void setRoles(const std::vector<std::string>& val);

  bool getAuthenticated() const;
  void setAuthenticated(bool val);

  bool hasRole(const std::string& role);

  static const std::string kUSERNAME;
  static const std::string kPASSWORD_HASH;
  static const std::string kROLES;

private:
  bool mAuthenticated;
};

#endif
