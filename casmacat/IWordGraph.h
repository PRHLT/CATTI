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
 * IWordGraph.h
 *
 *  Created on: 11/09/2012
 *      Author: valabau
 */

#ifndef CasMaCat_IWORDGRAPH_H_
#define CasMaCat_IWORDGRAPH_H_



#include <string>
#include <vector>
#include <casmacat/plugin-utils.h>


namespace casmacat {

  class IWordGraph {
  public:
    IWordGraph() {};
    virtual ~IWordGraph() {};
  private:
    // Following the rule of three copy and the assignment operator are disabled
    IWordGraph(const IWordGraph&); // Disallow copy
    IWordGraph& operator=(const IWordGraph&); // Disallow assignment operator
  };

  class IWordGraphProducer {
  public:
    IWordGraphProducer() {};
    virtual ~IWordGraphProducer() {};
    /**
     * return the word graph
     */
    IWordGraph *getWordGraph() { throw NotImplementedException(METHOD_DEFINITION); };
  private:
    // Following the rule of three copy and the assignment operator are disabled
    IWordGraphProducer(const IWordGraphProducer&); // Disallow copy
    IWordGraphProducer& operator=(const IWordGraphProducer&); // Disallow assignment operator
  };

  class IWordGraphConsumer {
  public:
    IWordGraphConsumer() {};
    virtual ~IWordGraphConsumer() {};
    /**
     * return the word graph
     */
    void setWordGraph(IWordGraph *wordgraph) { throw NotImplementedException(METHOD_DEFINITION); };
  private:
    // Following the rule of three copy and the assignment operator are disabled
    IWordGraphConsumer(const IWordGraphConsumer&); // Disallow copy
    IWordGraphConsumer& operator=(const IWordGraphConsumer&); // Disallow assignment operator
  };

}


#endif /* CasMaCat_IWORDGRAPH_H_ */
