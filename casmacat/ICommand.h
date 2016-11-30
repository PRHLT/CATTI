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
 * ICommand.h
 *
 *  Created on: 11/09/2012
 *      Author: valabau
 */

#ifndef CASMACAT_ICOMMAND_H_
#define CASMACAT_ICOMMAND_H_



#include <string>
#include <vector>
#include <casmacat/plugin-utils.h>


namespace casmacat {

/**
 * Interface for Confidence plug-ins
 */

  class ICommand {
  public:
    ICommand() {};
    virtual ~ICommand() {};
    /**
     * executes the command and return an error code (0 if it went OK)
     */
    virtual int execute() = 0;
  private:
    // Following the rule of three copy and the assignment operator are disabled
    ICommand(const ICommand&); // Disallow copy
    ICommand& operator=(const ICommand&); // Disallow assignment operator
  };

}


#endif /* CASMACAT_ICOMMAND_H_ */
