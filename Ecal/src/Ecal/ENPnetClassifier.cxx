#include "Recon/ENPnetClassifier.h"

namespace recon {

void ENPnetClassifier::configure(framework::config::Parameters& parameters) {
  // set parameters to the Python configured values
  model_path_ = parameters.get<std::string>("model_path");
  digi_tracker_coll_name_ =
      parameters.get<std::string>("digi_tracker_coll_name");
  digi_tracker_pass_name_ =
      parameters.get<std::string>("digi_tracker_pass_name");
  ecal_rec_hits_coll_name_ =
      parameters.get<std::string>("ecal_rec_hits_coll_name");
  ecal_rec_hits_pass_name_ =
      parameters.get<std::string>("ecal_rec_hits_pass_name");
  hcal_rec_hits_coll_name_ =
      parameters.get<std::string>("hcal_rec_hits_coll_name");
  hcal_rec_hits_pass_name_ =
      parameters.get<std::string>("hcal_rec_hits_pass_name");

  // define ONNX model runtime
  rt_ = std::make_unique<ldmx::ort::ONNXRuntime>(model_path_.c_str());
}

void ENPnetClassifier::produce(framework::Event& event) {
  // load collections
  const std::vector<ldmx::Measurement> digi_tracker_hits =
      event.getCollection<ldmx::Measurement>(digi_tracker_coll_name_,
                                             digi_tracker_pass_name_);
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(ecal_rec_hits_coll_name_,
                                         ecal_rec_hits_pass_name_);
  const std::vector<ldmx::HcalHit> hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(hcal_rec_hits_coll_name_,
                                         hcal_rec_hits_pass_name_);
  // create input matrix from collections
  makeInputs(digi_tracker_hits, ecal_rec_hits, hcal_rec_hits);
  // pass input matrix to model
  pred_ = rt_->run(input_names_, input_data_);
  // ldmx_log(debug) << "ParticleNet Classifier run with results: " << pred_; // doesn't work bc pred_ is a vector :(
  // do something with prediction, and output to some branch probably
}

void ENPnetClassifier::makeInputs(
    const std::vector<ldmx::Measurement>& digi_tracker_hits,
    const std::vector<ldmx::EcalHit>& ecal_rec_hits,
    const std::vector<ldmx::HcalHit>& hcal_rec_hits) {

  if (!digi_tracker_hits.empty()) {
    for (unsigned i = 0; i < digi_tracker_hits.size(); i++) {
      // load data from hit
      auto [x, y, z] = hit.getGlobalPosition();
      auto edep = TMath::Log(hit.getEdep());
      // fill recoil_points input
      int indx = std::distance(input_names_.begin(), std::find(input_names_.begin(), input_names_.end(), "recoil_points"));
      input_data_.at(indx).at(i).at(0) = x;
      input_data_.at(indx).at(i).at(1) = y;
      input_data_.at(indx).at(i).at(2) = z;
      // fill recoil_features input
      indx = std::distance(input_names_.begin(), std::find(input_names_.begin(), input_names_.end(), "recoil_features"));
      input_data_.at(indx).at(i).at(0) = edep;
      input_data_.at(indx).at(i).at(1) = x;
      input_data_.at(indx).at(i).at(2) = y;
      input_data_.at(indx).at(i).at(3) = z;
    }

    ldmx_log(trace) << "Filled NN input matrix with " << digi_tracker_hits.size() << " feature vectors from " << digi_tracker_coll_name_;
  }

  if (!ecal_rec_hits.empty()) {
    for (unsigned i = 0; i < ecal_rec_hits.size(); i++) {
      // load data from hit
      auto x = hit.getXPos();
      auto y = hit.getYPos();
      auto z = hit.getZPos();
      auto energy = hit.getEnergy();
      // fill recoil_points input
      int indx = std::distance(input_names_.begin(), std::find(input_names_.begin(), input_names_.end(), "ecal_points"));
      input_data_.at(indx).at(i).at(0) = x;
      input_data_.at(indx).at(i).at(1) = y;
      input_data_.at(indx).at(i).at(2) = z;
      // fill recoil_features input
      indx = std::distance(input_names_.begin(), std::find(input_names_.begin(), input_names_.end(), "ecal_features"));
      input_data_.at(indx).at(i).at(0) = energy;
      input_data_.at(indx).at(i).at(1) = x;
      input_data_.at(indx).at(i).at(2) = y;
      input_data_.at(indx).at(i).at(3) = z;
    }

    ldmx_log(trace) << "Filled NN input matrix with " << ecal_rec_hits.size() << " feature vectors from " << ecal_rec_hits_coll_name_;
  }

  if (!hcal_rec_hits.empty()) {
    for (unsigned i = 0; i < hcal_rec_hits.size(); i++) {
      // load data from hit
      auto x = hit.getXPos();
      auto y = hit.getYPos();
      auto z = hit.getZPos();
      auto energy = hit.getEnergy();
      // fill recoil_points input
      int indx = std::distance(input_names_.begin(), std::find(input_names_.begin(), input_names_.end(), "hcal_points"));
      input_data_.at(indx).at(i).at(0) = x;
      input_data_.at(indx).at(i).at(1) = y;
      input_data_.at(indx).at(i).at(2) = z;
      // fill recoil_features input
      indx = std::distance(input_names_.begin(), std::find(input_names_.begin(), input_names_.end(), "hcal_features"));
      input_data_.at(indx).at(i).at(0) = energy;
      input_data_.at(indx).at(i).at(1) = x;
      input_data_.at(indx).at(i).at(2) = y;
      input_data_.at(indx).at(i).at(3) = z;
    }

    ldmx_log(trace) << "Filled NN input matrix with " << hcal_rec_hits.size() << " feature vectors from " << hcal_rec_hits_coll_name_;
  }

}

}  // namespace recon

DECLARE_PRODUCER(recon::ENPnetClassifier)