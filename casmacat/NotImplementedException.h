#ifndef NOT_IMPLEMENTED_EXCEPTION_HPP
#define NOT_IMPLEMENTED_EXCEPTION_HPP

#include <string>
#include <stdexcept>

namespace casmacat {

  #define STRINGIFY(s) XSTRINGIFY(s)
  #define XSTRINGIFY(s) #s
  #define METHOD_DEFINITION (std::string(__FILE__)+std::string(":")+std::string(__func__)+std::string(":")+std::string(STRINGIFY(__LINE__)))

  class NotImplementedException: public std::runtime_error {
  public:
    NotImplementedException(const std::string& msg): runtime_error(std::string("Method not implemented: ")+msg) {}
  };
}

#endif

