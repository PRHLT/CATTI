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
 * IImtEngine.h
 *
 *  Created on: 16/07/2012
 *      Author: valabau
 */

#ifndef CASMACAT_IIMTENGINE_H_
#define CASMACAT_IIMTENGINE_H_

#include <string>
#include <vector>
#include <casmacat/IPluginFactory.h>
#include <casmacat/IMtEngine.h>


namespace casmacat {

/**
 * @class IImtEngine
 *
 * @brief Interface for Interactive Machine Translation plug-in engines
 *
 * This class provides an interface for interactive-predictive machine translation.
 * This class inherits from @link IMtEngine.
 *
 * @author Vicent Alabau
 */

  class IInteractiveMtSession {
  public:
    virtual ~IInteractiveMtSession() {};

    /* Set partial validation of a translation */
    virtual void setPartialValidation(const std::vector<std::string> &partial_translation,
                                      const std::vector<bool> &validated,
                                            std::vector<std::string> &corrected_translation_out,
                                            std::vector<bool> &corrected_validated_out
                                     ) = 0;

    /**
     * translates a sentence in a source language into a sentence in a target language
     *
     * This is a simplified version of the MT engine in `D5.1: Specification of casmacat workbench'
     * since this version does not take into account the optional parameters, as they are specific for
     * Moses. The original description is the following:
     *
     * translates sentence specified as `text'. If `align' switch is on, phrase alignment is returned.
     * If ’sg’ is on, search graph is returned. If ’topt’ is on, phrase options used are returned.
     * If ’report-all-factors’ is on, all factors are included in output. ’presence’ means that the
     * switch is on, if the category appears in the xml,value can be anything
     *
     * @param[in] source a sentence in the source language
     * @param[out] target a translation of source in the target language
     */

    virtual void setPrefix(const std::vector<std::string> &prefix,
                           const std::vector<std::string> &suffix,
                           const bool last_token_is_partial,
                           std::vector<std::string> &corrected_translation_out,
                           std::vector<std::pair<size_t, size_t> > &segmentation_out
                          ) = 0;

    virtual void rejectSuffix(const std::vector<std::string> &prefix,
                              const std::vector<std::string> &suffix,
                              const bool last_token_is_partial,
                              std::vector<std::string> &corrected_translation_out,
                              std::vector<std::pair<size_t, size_t> > &segmentation_out
                          ) = 0;
  };

  /**
   * @class IImtEngine
   *
   * @brief Interface for Interactive Machine Translation plug-in engines
   *
   * This class provides an interface for interactive-predictive machine translation.
   * This class inherits from @link IMtEngine.
   *
   * @author Vicent Alabau
  */

  class IInteractiveMtEngine: public IMtEngine {
  public:
    virtual ~IInteractiveMtEngine() {};

    /**
     * create new IMT session
     */
    virtual IInteractiveMtSession *newSession(const std::vector<std::string> &source) = 0;

    /**
     * delete IMT session
     */
    virtual void deleteSession(IInteractiveMtSession *session) = 0;

    /* Update translation models with source/target pair (total or partial translation) */
    virtual void validate(const std::vector<std::string> &source,
                          const std::vector<std::string> &target,
                          const std::vector<bool> &validated
                         ) = 0;
  };

  typedef IPluginFactory<IInteractiveMtEngine> IInteractiveMtFactory;
}

#endif /* CASMACAT_IIMTENGINE_H_ */
