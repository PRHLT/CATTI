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
 * plugin-utils.h
 *
 *  Created on: 02/07/2012
 *      Author: valabau
 */

#ifndef CASMACAT_PLUGIN_UTILS_H_
#define CASMACAT_PLUGIN_UTILS_H_

#include <typeinfo>
#include <iostream>
#include <iterator>
#include <cerrno>
#include <casmacat/Context.h>

#define EXPORT_CASMACAT_PLUGIN_NAME(I, C, name) \
  extern "C" casmacat::IPluginFactory<casmacat::I> * new_##name(int argc, char *argv[], casmacat::Context *context) { return casmacat::new_object<casmacat::IPluginFactory<casmacat::I>, C>(argc, argv, context); } \
  extern "C" void delete_##name(casmacat::IPluginFactory<casmacat::I> * c) { casmacat::delete_object<casmacat::IPluginFactory<casmacat::I>, C>(c); } \
  extern "C" const std::type_info& name##_type() { return casmacat::plugin_type<casmacat::IPluginFactory<casmacat::I> >(); }

#define EXPORT_CASMACAT_PLUGIN(I, C) EXPORT_CASMACAT_PLUGIN_NAME(I, C, plugin)

namespace casmacat {

template <typename I, typename C> I *new_object(int argc, char *argv[], Context *context = 0) {
  I* c = 0;
  try {
    c = new C();
    int status = c->init(argc, argv, context);
    if (status != 0) {
      std::cerr << "Plugin initialization failed with status " << status << std::endl;
      errno = status;
      delete c;
      c = 0;
    }
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    delete c;
    c = 0;
  }
  return c;
}

template <typename I, typename C> bool delete_object(I * i) {
  try {
    const C* c = dynamic_cast<C *>(i);
    delete c;
    return true;
  }
  catch (const std::bad_cast& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "This object of type " << typeid(I).name() << " is not of type " << typeid(C).name() << std::endl;
    delete i;
    return false;
  }
}

template <typename I> const std::type_info& plugin_type() { return typeid(I); }


}

#endif /* CASMACAT_PLUGIN_UTILS_H_ */
