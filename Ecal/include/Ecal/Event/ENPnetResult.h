/**
 * @file ENPnetResult.h
 * @brief Class used to encapsulate the results obtained from
 *        ENPnetClassifier.
 * @author Oscar Lewis (UCSB)
 */

#ifndef EVENT_ENPNETRESULT_H_
#define EVENT_ENPNETRESULT_H_

// C++ standard library
#include <cassert>
#include <vector>
#include <numeric>

// ROOT
#include <TObject.h>  //For ClassDef

namespace ldmx {

class ENPnetResult {
  public:
    // Constructor
    ENPnetResult() = default;

    // Destructor
    virtual ~ENPnetResult();

    // Clear all values
    void clear();

    void setLeadingPID(std::vector<unsigned>& class_vec);
    void setLeadingPID(unsigned class_indx);

    void setProbs(std::vector<float>& class_probs);

    void setLeadingConfidence(float conf) { leading_confidence_ = conf; }
  
  private:
    // final state classes
    bool leading_is_charged_pion_ = false;
    bool leading_is_neutral_pion_ = false;
    bool leading_is_charged_kaon_ = false;
    bool leading_is_neutral_kaon_ = false;
    bool leading_is_proton_       = false;
    bool leading_is_neutron_      = false;
    bool leading_is_other_        = false;
    bool no_en_daughters_         = false;
    float leading_confidence_           = -1.0;
    float prob_leading_is_charged_pion_ = -1.0;
    float prob_leading_is_neutral_pion_ = -1.0;
    float prob_leading_is_charged_kaon_ = -1.0;
    float prob_leading_is_neutral_kaon_ = -1.0;
    float prob_leading_is_proton_       = -1.0;
    float prob_leading_is_neutron_      = -1.0;
    float prob_leading_is_other_        = -1.0;
    float prob_no_en_daughters_         = -1.0;

  ClassDef(ENPnetResult, 1);
}; // end class definition

} // end namespace ldmx

#endif
