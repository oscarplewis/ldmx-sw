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
#include <string>

// ldmx-sw framework
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Configure/Parameters.h"

// ldmx-sw other
#include "Tools/ONNXRuntime.h"
#include "Ecal/Event/EcalHit.h"
#include "Hcal/Event/HcalHit.h"
#include "Tracking/Event/Measurement.h"

namespace recon {

class ENPnetClassifier : public framework::Producer {
 public:
  ENPnetClassifier(const std::string& name,
                             framework::Process& process)
      : Producer(name, process) {};
  virtual ~ENPnetClassifier() = default;
  void configure(framework::config::Parameters& parameters) override;
  void produce(framework::Event& event) override;

 private:
  void makeInputs(const std::vector<ldmx::Measurement>& digi_tracker_hits,
                  const std::vector<ldmx::EcalHit>& ecal_rec_hits,
                  const std::vector<ldmx::HcalHit>& hcal_rec_hits);

  std::unique_ptr<ldmx::ort::ONNXRuntime> rt_;

  std::string model_path_;
  std::string digi_tracker_coll_name_;
  std::string digi_tracker_pass_name_;
  std::string ecal_rec_hits_coll_name_;
  std::string ecal_rec_hits_pass_name_;
  std::string hcal_rec_hits_coll_name_;
  std::string hcal_rec_hits_pass_name_;

  const std::vector<std::string> input_names_{
      "recoil_points", "recoil_features", "ecal_points",
      "ecal_features", "hcal_points",     "hcal_features"};
  const std::vector<unsigned> input_sizes_;

  // the maximum number of hits imposed on the input to the model
  const unsigned recoil_feats_len = 64;
  const unsigned ecal_feats_len = 256;
  const unsigned hcal_feats_len = 256;

  // initialize input matrix based on the input for this particular model
  std::vector<std::vector<std::vector<float>>> input_data_{std::vector<std::vector<float>>(recoil_feats_len, std::vector<float>(3, 0.0)), std::vector<std::vector<float>>(recoil_feats_len, std::vector<float>(4, 0.0)), std::vector<std::vector<float>>(ecal_feats_len, std::vector<float>(3, 0.0)), std::vector<std::vector<float>>(ecal_feats_len, std::vector<float>(4, 0.0)), std::vector<std::vector<float>>(hcal_feats_len, std::vector<float>(3, 0.0)), std::vector<std::vector<float>>(hcal_feats_len, std::vector<float>(4, 0.0))};
  // init variable to store predictions
  std::vector<std::vector<float>> pred_;
};

}  // namespace recon
#endif /* ENPNETCLASSIFIER_H */
