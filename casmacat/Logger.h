/*
 *   Copyright 2012, valabau
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * 
 * Logger.h
 *
 *  Created on: 20/09/2012
 *      Author: valabau
 */

#ifndef CASMACAT_LOGGER_H_
#define CASMACAT_LOGGER_H_

#include <sstream>

namespace casmacat {

typedef enum { ERROR_LOG, WARN_LOG, INFO_LOG, DEBUG_LOG } log_t;

class Logger {
public:
  virtual ~Logger() {}
  virtual void log(log_t type, const std::string &msg) = 0;
};

#define LOG(TYPE) if (_logger) LoggerStream<TYPE##_LOG>(_logger).get() << __PRETTY_FUNCTION__ << ": "

template <log_t type>
class LoggerStream {
  Logger *_logger;
  std::ostringstream os;
public:
  LoggerStream(Logger *logger): _logger(logger) {}
  virtual ~LoggerStream() {
    os << std::endl;
    _logger->log(type, os.str());
  }
  std::ostringstream& get() { return os; }
private:
  LoggerStream(const LoggerStream&);
  LoggerStream& operator =(const LoggerStream&);
};

class Loggable {
public:
  virtual ~Loggable() {};
  virtual void setLogger(Logger *) {};
};

}


#endif /* CASMACAT_LOGGER_H_ */
