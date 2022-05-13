#ifndef STRINGTHINGS_HPP
#define STRINGTHINGS_HPP

#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace StringThings {

std::string fixed(double number, unsigned int decimals);
void replaceRegex(std::string& string, const std::string& search, const std::string& replace);
void replace(std::string& str, const std::string& from, const std::string& to);

std::string rangify(std::vector<int>& vec);
std::vector<int> unrangify(const std::string& range);

std::vector<std::string> strToVec(const std::string& vec, char seperator, bool skipEmpty = true);

template <typename T>
std::string vecToStr(const std::vector<T>& vec, const std::string& seperator, std::function<std::string(const T&)> func = nullptr)
{
  if (vec.empty())
    return "";

  std::stringstream ss;
  auto iter = vec.begin();

  if (func)
    ss << func(*iter);
  else
    ss << *iter;

  iter++;

  for (; iter != vec.end(); iter++)
  {
    if (func)
      ss << seperator << func(*iter);
    else
      ss << seperator << *iter;
  }

  return ss.str();
}

template <typename T>
bool strTo(T& retval, const std::string& tempStr)
{
  std::istringstream iss(tempStr);

  iss >> retval;
  if (iss.fail())
    return false;
  return true;
}

bool isNumber(const std::string& string);

void toLower(std::string& str);

}  // namespace StringThings
#endif
