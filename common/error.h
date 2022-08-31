#pragma once

#include <string>

class Error {
  
public:
  
  Error() { }
  Error(std::string&& message)
  : m_message(std::move(message)) { }
  
  const std::string& message() const {
    return m_message;
  }
  
private:
  
  std::string m_message;
  
};
