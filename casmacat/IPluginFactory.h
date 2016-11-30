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
 * PluginFactory.h
 *
 *  Created on: 20/09/2012
 *      Author: valabau
 */

#ifndef CASMACAT_IPLUGIN_FACTORY_H_
#define CASMACAT_IPLUGIN_FACTORY_H_

#include <casmacat/plugin-utils.h>
#include <casmacat/Logger.h>

namespace casmacat {

  template <class C>
  class IPluginFactory: public Loggable {
  public:
    virtual ~IPluginFactory() {};
    /**
     * initialize the Confidence engine with main-like parameters
     */
    virtual int init(int argc, char *argv[], Context *context = 0) = 0;
    virtual std::string getVersion() = 0;

    /**
     * create an instance of a confidence engine
     * @param[in] specialization_id returns a specialized version of the confidence engine,
     *            for instance, for user specific models
     */
    virtual C *createInstance(const std::string &specialization_id = "") = 0;
    virtual void deleteInstance(C *instance) = 0;
  };
}


#endif /* CASMACAT_IPLUGIN_FACTORY_H_ */
