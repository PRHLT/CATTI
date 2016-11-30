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
 * Updateable.h
 *
 *  Created on: 20/09/2012
 *      Author: valabau
 */

#ifndef CASMACAT_UPDATEABLE_H_
#define CASMACAT_UPDATEABLE_H_


namespace casmacat {


class IUpdateable {
public:
  virtual ~IUpdateable() {}

  /**
   * updates translation models with source/target pair
   *
   * This is a simplified version of the MT engine in `D5.1: Specification of casmacat workbench'
   * since this version does not take into account the optional parameters, as they are specific for
   * Moses. The original description is the following:

   * updates a suffix array phrase table. If `bounded' switch is on,
   * seems to do nothing at the moment. If `updateORLM' is on, a suffix array language
   * model is also updated.
   *
   * @param[in] source a sentence in the source language
   * @param[in] target a sentence in the target language that is a valid translation of source
   */
  virtual void update(const std::vector<std::string> &source,
                      const std::vector<std::string> &target) = 0;
};

}

#endif /* CASMACAT_UPDATEABLE_H_ */
