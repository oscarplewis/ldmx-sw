#include "Ecal/Event/ENPnetResult.h"

ClassImp(ldmx::ENPnetResult);

namespace ldmx {

// Destructor
ENPnetResult::~ENPnetResult() { clear(); }

// Clear all values
void ENPnetResult::clear() {
  leading_is_charged_pion_ = false;
  leading_is_neutral_pion_ = false;
  leading_is_charged_kaon_ = false;
  leading_is_neutral_kaon_ = false;
  leading_is_proton_ = false;
  leading_is_neutron_ = false;
  leading_is_other_ = false;
  leading_confidence_ = -1.0;
  prob_leading_is_charged_pion_ = -1.0;
  prob_leading_is_neutral_pion_ = -1.0;
  prob_leading_is_charged_kaon_ = -1.0;
  prob_leading_is_neutral_kaon_ = -1.0;
  prob_leading_is_proton_ = -1.0;
  prob_leading_is_neutron_ = -1.0;
  prob_leading_is_other_ = -1.0;
}

void ENPnetResult::setLeadingPID(std::vector<unsigned>& class_vec) {
  assert(class_vec.size() == n_classes &&
         std::accumulate(class_vec.begin(), class_vec.end(), 0) == 1 &&
         "There must be a specified value for each class, and only one class "
         "may be specified!");
  leading_is_charged_pion_ = static_cast<bool>(class_vec.at(0));
  leading_is_neutral_pion_ = static_cast<bool>(class_vec.at(1));
  leading_is_charged_kaon_ = static_cast<bool>(class_vec.at(2));
  leading_is_neutral_kaon_ = static_cast<bool>(class_vec.at(3));
  leading_is_proton_ = static_cast<bool>(class_vec.at(4));
  leading_is_neutron_ = static_cast<bool>(class_vec.at(5));
  leading_is_other_ = static_cast<bool>(class_vec.at(6));
}

void ENPnetResult::setLeadingPID(unsigned class_indx) {
  std::vector<unsigned> class_vec(8, 0);
  class_vec.at(class_indx) = 1;
  setLeadingPID(class_vec);
}

void ENPnetResult::setLeadingProbs(std::vector<float>& class_probs) {
  assert(class_probs.size() == n_classes &&
         "There must be a specified value for each class!");
  prob_leading_is_charged_pion_ = class_probs.at(0);
  prob_leading_is_neutral_pion_ = class_probs.at(1);
  prob_leading_is_charged_kaon_ = class_probs.at(2);
  prob_leading_is_neutral_kaon_ = class_probs.at(3);
  prob_leading_is_proton_ = class_probs.at(4);
  prob_leading_is_neutron_ = class_probs.at(5);
  prob_leading_is_other_ = class_probs.at(6);
  leading_confidence_ =
      *std::max_element(class_probs.begin(), class_probs.end());
}

}  // end namespace ldmx