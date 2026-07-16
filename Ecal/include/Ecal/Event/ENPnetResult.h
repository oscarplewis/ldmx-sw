/**
 * @file ENPnetResult.h
 * @brief Class used to encapsulate the results obtained from
 *        ENPnetClassifier.
 * @author Oscar Lewis (UCSB)
 */

#ifndef EVENT_ENPNETRESULT_H_
#define EVENT_ENPNETRESULT_H_

// C++ standard library
#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

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

  // Setter methods

  void setLeadingPID(std::vector<unsigned>& class_vec);
  void setLeadingPID(unsigned class_indx);

  void setLeadingProbs(std::vector<float>& class_probs);

  void setLeadingConfidence(float conf) { leading_confidence_ = conf; }

  // Getter methods

  float getLeadingConfidence() { return leading_confidence_; }

  std::vector<float> getLeadingProbs() {
    return {prob_leading_is_charged_pion_, prob_leading_is_neutral_pion_,
            prob_leading_is_charged_kaon_, prob_leading_is_neutral_kaon_,
            prob_leading_is_proton_,       prob_leading_is_neutron_,
            prob_leading_is_other_};
  }

  std::vector<bool> getLeadingPID() {
    return {leading_is_charged_pion_, leading_is_neutral_pion_,
            leading_is_charged_kaon_, leading_is_neutral_kaon_,
            leading_is_proton_,       leading_is_neutron_,
            leading_is_other_};
  }

  // number of classes that the model can choose between
  inline static const unsigned n_classes = 7;

 private:
  // final state classes
  bool leading_is_charged_pion_ = false;
  bool leading_is_neutral_pion_ = false;
  bool leading_is_charged_kaon_ = false;
  bool leading_is_neutral_kaon_ = false;
  bool leading_is_proton_ = false;
  bool leading_is_neutron_ = false;
  bool leading_is_other_ = false;
  float leading_confidence_ = -1.0;
  float prob_leading_is_charged_pion_ = -1.0;
  float prob_leading_is_neutral_pion_ = -1.0;
  float prob_leading_is_charged_kaon_ = -1.0;
  float prob_leading_is_neutral_kaon_ = -1.0;
  float prob_leading_is_proton_ = -1.0;
  float prob_leading_is_neutron_ = -1.0;
  float prob_leading_is_other_ = -1.0;

  ClassDef(ENPnetResult, 1);
};  // end class definition

}  // end namespace ldmx

#endif
