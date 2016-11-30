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
 * Context.h
 *
 *  Created on: 16/10/2012
 *      Author: valabau
 */

#ifndef CASMACAT_CONTEXT_H_
#define CASMACAT_CONTEXT_H_

#include <string>
#include <cstring>
#include <map>
#include <typeinfo>
#include <utility>

namespace casmacat {


template<typename Interface, typename Class>
bool provides(Class *_object) {
  Interface *object = dynamic_cast<Interface *>(_object);
  return object != 0;
}

template<typename Interface>
Interface *as(void *_object) {
  return dynamic_cast<Interface *>(_object);
}


class Context {
  typedef struct _context_t {
    std::string type;
    void *pointer;
    _context_t(const std::type_info &_type, void *_pointer): type(_type.name()), pointer(_pointer) {};
  } context_t;

public:
  template <typename Interface>
  void set(const std::string &name, Interface* object) {
    _objects.insert(std::make_pair(name, context_t(typeid(Interface), object)));
  }

  template <typename Interface>
  Interface* get(const std::string &name) {
    std::map<std::string, _context_t>::iterator it = _objects.find(name);
    if (it != _objects.end() and it->second.type == std::string(typeid(Interface).name())) {
//    if (it == _objects.end() or it->second.type != typeid(Interface)) {
      return reinterpret_cast<Interface *>(it->second.pointer);
    }
    return 0;
  }

  bool has(const std::string &name) {
	std::map<std::string, _context_t>::iterator it = _objects.find(name);
	return it != _objects.end();
  }

private:
  std::map<std::string, _context_t> _objects;
};

}



#endif /* CASMACAT_CONTEXT_H_ */
