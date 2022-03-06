#ifndef AUTHENTICATOR_HPP
#define AUTHENTICATOR_HPP

#include "Table.hpp"
#include "User.hpp"

class Authenticator : public CsvDb::Table
{
public:
  Authenticator();
  ~Authenticator();

  bool authenticateUser(const User& user);
};

#endif
