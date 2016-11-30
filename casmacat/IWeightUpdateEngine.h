/*
 *   Copyright 2012, gsanchis
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
 * WeightUpdateEngine.h
 *
 *  Created on: 31/10/2012
 *      Author: gsanchis
 */

#ifndef CASMACAT_IWEIGHTUPDATEENGINE_H_
#define CASMACAT_IWEIGHTUPDATEENGINE_H_

#include <string>
#include <vector>
#include <casmacat/IPluginFactory.h>

namespace casmacat {

/**
 * Interface for Weight update plug-ins
 */

  class IWeightUpdateEngine {
  public:
    virtual ~IWeightUpdateEngine() {};

    /**
     * obtain updated weight vector from the nbest list and nbest features
     */
    virtual void updatelogWeights(const std::vector<double>& currentWeights,
                                  const std::string& reference,
                                  const std::vector<std::string>& nblist,
                                  const std::vector<std::vector<double> >& scoreCompsVec,
                                  std::vector<double>& newWeights) = 0;
  };

  typedef IPluginFactory<IWeightUpdateEngine> IWeightUpdateFactory;
}

#endif /* CASMACAT_IWEIGHTUPDATEENGINE_H_ */
