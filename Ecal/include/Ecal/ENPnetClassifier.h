/**
 * @file ENPnetClassifier.h
 * @brief Class that determines the type of EN event using a ParticleNet-based
 * DGNN
 * @author Oscar Lewis (UCSB)
 */

#ifndef ENPNETCLASSIFIER_H
#define ENPNETCLASSIFIER_H

// C++ standard library
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

// ldmx-sw framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"

// ldmx-sw other
#include "Ecal/Event/ENPnetResult.h"
#include "Ecal/Event/EcalHit.h"
#include "Hcal/Event/HcalHit.h"
#include "Tools/ONNXRuntime.h"
#include "Tracking/Event/Measurement.h"

// type trait to detect if something is a std::vector
template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;

// Generic converter: works for scalars, strings, and nested vectors
template <typename T>
std::string toString(const T& value) {
  if constexpr (is_vector_v<T>) {
    std::ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < value.size(); ++i) {
      oss << toString(value[i]);
      if (i + 1 < value.size()) oss << ", ";
    }
    oss << ")";
    return oss.str();
  } else {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }
}

namespace ecal {

class ENPnetClassifier : public framework::Producer {
 public:
  ENPnetClassifier(const std::string& name, framework::Process& process)
      : Producer(name, process) {};
  virtual ~ENPnetClassifier() = default;
  void configure(framework::config::Parameters& parameters) override;
  void produce(framework::Event& event) override;

 private:
  // Loads detector hit collections into the format required by the model.
  void makeInputs(const std::vector<ldmx::Measurement>& digi_tracker_hits,
                  const std::vector<ldmx::EcalHit>& ecal_rec_hits,
                  const std::vector<ldmx::HcalHit>& hcal_rec_hits);
  // Calculates softmax function on a vector of logits to produce prediction
  // probabilities.
  std::vector<float> softmax(const std::vector<float>& logits);
  // Interprets prediction probabilities to write to `result_`.
  void generateResult(std::vector<float>& outputs);

  std::string model_path_;
  std::string digi_tracker_coll_name_;
  std::string digi_tracker_pass_name_;
  std::string ecal_rec_hits_coll_name_;
  std::string ecal_rec_hits_pass_name_;
  std::string hcal_rec_hits_coll_name_;
  std::string hcal_rec_hits_pass_name_;
  std::string collection_name_;

  const std::vector<std::string> input_names_{
      "recoil_points", "recoil_features", "ecal_points",
      "ecal_features", "hcal_points",     "hcal_features"};
  const std::vector<std::string> output_classes_{
      "leading_is_charged_pion", "leading_is_neutral_pion",
      "leading_is_charged_kaon", "leading_is_neutral_kaon",
      "leading_is_proton",       "leading_is_neutron",
      "leading_is_other"};

  // the maximum number of hits imposed on the input to the model
  const unsigned recoil_points_max_ = 256;
  const unsigned ecal_points_max_ = 256;
  const unsigned hcal_points_max_ = 256;
  // position of different features in vectors
  // the flattened data needs to be passed to ONNXRuntime::run in the form
  // of (N, C, P), N=batch size, C=no. of features, P=no. of points
  // so the data should be channel-major ordered,
  //       (x0, x1,..., y0, y1,...) and NOT (x0, y0, z0, ...)
  // these offsets here represent the order of the features, which will be
  // multiplied by the number of points to give the correct ordering in the
  // implementation of makeInputs()
  const unsigned coord_x_offset_ = 0;
  const unsigned coord_y_offset_ = 1;
  const unsigned coord_z_offset_ = 2;
  const unsigned feat_e_offset_ = 0;
  const unsigned feat_x_offset_ = 1;
  const unsigned feat_y_offset_ = 2;
  const unsigned feat_z_offset_ = 3;
  // size of features column
  const unsigned feats_len_ = 4;
  const unsigned coords_len_ = 3;

  // the classifier model itself
  std::unique_ptr<ldmx::ort::ONNXRuntime> rt_;
  // input matrix based on the input for this particular model
  std::vector<std::vector<float>> input_data_{
      std::vector<float>(recoil_points_max_ * coords_len_, 0.0),
      std::vector<float>(recoil_points_max_* feats_len_, 0.0),
      std::vector<float>(ecal_points_max_* coords_len_, 0.0),
      std::vector<float>(ecal_points_max_* feats_len_, 0.0),
      std::vector<float>(hcal_points_max_* coords_len_, 0.0),
      std::vector<float>(hcal_points_max_* feats_len_, 0.0)};
  // logits are directly outputted by the ParticleNet model
  std::vector<float> logits_;
  // pred_ stores the processed probabilities, calculated from the logits
  std::vector<float> pred_;
  // result_ stores the final result of the classifier processor
  ldmx::ENPnetResult result_;
};

}  // namespace ecal
#endif /* ENPNETCLASSIFIER_H */
