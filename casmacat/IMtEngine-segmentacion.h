/**
 * @file IMtEngine.h
 * Interface for Machine Translation plug-ins
 *
 * @author Vicent Alabau
 */

#ifndef CASMACAT_IMTENGINE_HPP
#define CASMACAT_IMTENGINE_HPP

#include <string>
#include <vector>
#include <casmacat/IPluginFactory.h>
#include <casmacat/IUpdateable.h>


namespace casmacat {

/**
 * @class IMtEngine
 *
 * @brief Interface for Machine Translation plug-in engines
 *
 * This class provides a simple interface for translation
 *
 * @author Vicent Alabau
 */

  class IMtEngine: public IUpdateable {
  public:
    virtual ~IMtEngine() {};

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
    virtual void translate(const std::vector<std::string> &source,
                                 std::vector<std::string> &target_out,
                                 std::vector< std::pair<size_t, size_t> > &segmentation_out) = 0;

  };

  typedef IPluginFactory<IMtEngine> IMtFactory;

}

#endif // CASMACAT_IMTENGINE_HPP
