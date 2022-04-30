#include "Authenticator.hpp"

#include "StringThings.hpp"

Authenticator::Authenticator() :
    CsvDb::Table("users.db", User::kUSERNAME)
{
  // Important so that Base class can add Records that can be later dynamic
  // cast to the derivide class Song
  setNewRecordFunc([]() -> CsvDb::Record* {
    return new User;
  });

  addScpiFunctions({
      {"ADD", [this](const Scpi& scpi) -> std::string {
         if (scpi.getParams().size() != 2)
           return "Not enough params";

         std::string username;
         std::string passwordClear;

         scpi.getParam(username, 0);
         scpi.getParam(passwordClear, 1);

         auto passwordHash = CsvDb::Table::sha256(passwordClear);

         // People add through SCPI get DJ rights
         User* user = new User;
         user->setUsername(username);
         user->setPasswordHash(passwordHash);
         user->setRoles({"DJ", "Singer"});

         return std::to_string(addRecord(user));
       }},

      {"AUTH", [this](const Scpi& scpi) -> std::string {
         if (scpi.getParams().size() != 2)
           return "Not enough params";

         std::string username;
         std::string passwordClear;

         scpi.getParam(username, 0);
         scpi.getParam(passwordClear, 1);

         auto passwordHash = CsvDb::Table::sha256(passwordClear);

         // People add through SCPI get DJ rights
         User user;
         user.setUsername(username);
         user.setPasswordHash(passwordHash);

         return std::to_string(authenticateUser(user));
       }},
  });

  setNewFileHeader({
      "USERNAME",
      "PASSWORD_HASH",
      "ROLES",
  });
}

Authenticator::~Authenticator()
{
}

bool Authenticator::authenticateUser(const User& user)
{
  auto userFromTable = dynamic_cast<User*>(getRecord(user.getField(getKey())));

  if (!userFromTable)
    return false;

  if (user.getPasswordHash() != userFromTable->getPasswordHash())
    return false;

  userFromTable->setAuthenticated(true);

  return true;
}
