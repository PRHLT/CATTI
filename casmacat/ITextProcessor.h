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
 * ITextProcessor.h
 *
 *  Created on: 14/09/2012
 *      Author: valabau
 */

#ifndef CASMACAT_ITEXTPROCESSOR_H_
#define CASMACAT_ITEXTPROCESSOR_H_

#include <casmacat/IPluginFactory.h>

namespace casmacat {

/**
 * Interface for Confidence plug-ins
 */

  class ITextProcessor {
  public:
    virtual ~ITextProcessor() {};

    virtual void preprocess(const std::string &detokenized,
                                      std::vector<std::string> &tokenized_out,
                                      std::vector< std::pair<size_t, size_t> > &segmentation_out) = 0;
    virtual void postprocess(const std::vector<std::string> &tokenized,
                                       std::string &detokenized_out,
                                       std::vector< std::pair<size_t, size_t> > &segmentation_out) = 0;
  };

  typedef IPluginFactory<ITextProcessor> ITextProcessorFactory;

}

#endif /* CASMACAT_ITEXTPROCESSOR_H_ */
