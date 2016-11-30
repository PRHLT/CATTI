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
 * utils.h
 *
 *  Created on: 21/06/2012
 *      Author: valabau
 */

#ifndef CasMaCat_UTILS_H_
#define CasMaCat_UTILS_H_

#include <sstream>
#include <limits>
#include <vector>
#include <iterator>

namespace casmacat {

  template <typename T>
  void tokenize(const T& str,
                std::vector<T>& tokens,
                const T& delimiters = T(" "))
  {
    if (delimiters == T("")) {
      tokens.reserve(str.size());
      for (typename T::const_iterator it = str.begin(); it != str.end(); ++it) {
        T tok;
        tok.push_back(*it);
        tokens.push_back(tok);
      }
    }
    else {
      tokens.clear();

      // Skip delimiters at beginning.
      typename T::size_type lastPos = str.find_first_not_of(delimiters, 0);
      // Find first "non-delimiter".
      typename T::size_type pos     = str.find_first_of(delimiters, lastPos);

      while (T::npos != pos || T::npos != lastPos) {
          // Found a token, add it to the vector.
          tokens.push_back(str.substr(lastPos, pos - lastPos));
          // Skip delimiters.  Note the "not_of"
          lastPos = str.find_first_not_of(delimiters, pos);
          // Find next "non-delimiter"
          pos = str.find_first_of(delimiters, lastPos);
      }
    }
  }

  template <typename T>
  void join(const std::vector<T>& tokens, T& str, const T& delimiter = T(" ")) {
    std::stringstream ss;
    std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<T>(ss, delimiter.c_str()));
    str = ss.str();
  }

  template <typename T>
  T convert_string(const std::string& text) {
    T ret;
    std::istringstream ss(text, std::istringstream::in);
    ss >> ret;
    if (ss.fail()) {
      if (text == "inf") return std::numeric_limits<T>::infinity();
      else if (text == "-inf") return -std::numeric_limits<T>::infinity();
      else if (text == "nan") return std::numeric_limits<T>::quiet_NaN();
      return std::numeric_limits<T>::quiet_NaN();
      //assert_bt(false, "Unknown error");

    }
    return ret;
  }

  template <typename T>
  std::string to_string(const T& value) {
    std::ostringstream ss(std::ostringstream::out);
    ss << value;
    return ss.str();
  }

}


#endif /* CasMaCat_UTILS_H_ */
