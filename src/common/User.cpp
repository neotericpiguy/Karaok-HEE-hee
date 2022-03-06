#include "User.hpp"

#include "StringThings.hpp"

const std::string User::kUSERNAME = "USERNAME";
const std::string User::kPASSWORD_HASH = "PASSWORD_HASH";
const std::string User::kROLES = "ROLES";

User::User() :
    mAuthenticated(false)
{
}

const std::string& User::getUsername() const
{
  return getField(User::kUSERNAME);
}
void User::setUsername(const std::string& val)
{
  setField(User::kUSERNAME, val);
}

const std::string& User::getPasswordHash() const
{
  return getField(User::kPASSWORD_HASH);
}
void User::setPasswordHash(const std::string& val)
{
  setField(User::kPASSWORD_HASH, val);
}

std::vector<std::string> User::getRoles() const
{
  return StringThings::strToVec(getField(User::kROLES), '|');
}
void User::setRoles(const std::vector<std::string>& val)
{
  setField(User::kROLES, StringThings::vecToStr(val, "|"));
}

bool User::getAuthenticated() const
{
  return mAuthenticated;
}
void User::setAuthenticated(bool val)
{
  mAuthenticated = val;
}

bool User::hasRole(const std::string& role)
{
  auto roles = getField(User::kROLES);
  return roles.find(role) != std::string::npos;
}
