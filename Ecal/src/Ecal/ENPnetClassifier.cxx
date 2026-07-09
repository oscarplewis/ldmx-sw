#include "Ecal/ENPnetClassifier.h"

namespace ecal {

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
  collection_name_ = parameters.get<std::string>("collection_name");

  // define ONNX model runtime
  rt_ = std::make_unique<ldmx::ort::ONNXRuntime>(model_path_.c_str());
}

void ENPnetClassifier::produce(framework::Event& event) {
  // first clear the output
  result_.clear();
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
  // fill input matrix from collections
  makeInputs(digi_tracker_hits, ecal_rec_hits, hcal_rec_hits);

  // ldmx_log(debug) << toString(rt_->getOutputNames());

  // ldmx_log(debug) << "ParticleNet output shape: "
  //                 << toString(rt_->getOutputShape("softmax"));

  // pass input matrix to model, and get 'softmax' 8-component output from 2d
  // output tensor
  pred_ = rt_->run(input_names_, input_data_,
                   {recoil_points_max_, recoil_points_max_, ecal_points_max_,
                    ecal_points_max_, hcal_points_max_, hcal_points_max_},
                   {}, 1)[0];
  ldmx_log(debug) << "ParticleNet classifier run with results: "
                  << toString(pred_);

  processOutputs(pred_);

  event.add(collection_name_, result_);
}

void ENPnetClassifier::makeInputs(
    const std::vector<ldmx::Measurement>& digi_tracker_hits,
    const std::vector<ldmx::EcalHit>& ecal_rec_hits,
    const std::vector<ldmx::HcalHit>& hcal_rec_hits) {
  if (!digi_tracker_hits.empty()) {
    unsigned n_hits_filled_ = 0;
    for (unsigned i = 0; i < digi_tracker_hits.size(); i++) {
      if (i >= recoil_points_max_) {
        break;
      }
      n_hits_filled_++;
      // load data from hit
      ldmx::Measurement hit = digi_tracker_hits.at(i);
      auto [x, y, z] = hit.getGlobalPosition();
      auto edep = std::log(hit.getEdep());
      // fill recoil_points input
      int indx = std::distance(
          input_names_.begin(),
          std::find(input_names_.begin(), input_names_.end(), "recoil_points"));
      input_data_.at(indx).at((coord_x_offset_ * recoil_points_max_) + i) = x;
      input_data_.at(indx).at((coord_y_offset_ * recoil_points_max_) + i) = y;
      input_data_.at(indx).at((coord_z_offset_ * recoil_points_max_) + i) = z;
      // fill recoil_features input
      indx = std::distance(input_names_.begin(),
                           std::find(input_names_.begin(), input_names_.end(),
                                     "recoil_features"));
      input_data_.at(indx).at((feat_e_offset_ * recoil_points_max_) + i) = edep;
      input_data_.at(indx).at((feat_x_offset_ * recoil_points_max_) + i) = x;
      input_data_.at(indx).at((feat_y_offset_ * recoil_points_max_) + i) = y;
      input_data_.at(indx).at((feat_z_offset_ * recoil_points_max_) + i) = z;
    }

    ldmx_log(debug) << "Filled NN input matrix with " << n_hits_filled_
                    << " hits from " << digi_tracker_coll_name_;
  }

  if (!ecal_rec_hits.empty()) {
    unsigned n_hits_filled_ = 0;
    for (unsigned i = 0; i < ecal_rec_hits.size(); i++) {
      if (i >= ecal_points_max_) {
        break;
      }
      n_hits_filled_++;
      // load data from hit
      ldmx::EcalHit hit = ecal_rec_hits.at(i);
      auto x = hit.getXPos();
      auto y = hit.getYPos();
      auto z = hit.getZPos();
      auto energy = std::log(hit.getEnergy());
      // fill recoil_points input
      int indx = std::distance(
          input_names_.begin(),
          std::find(input_names_.begin(), input_names_.end(), "ecal_points"));
      input_data_.at(indx).at((coord_x_offset_ * ecal_points_max_) + i) = x;
      input_data_.at(indx).at((coord_y_offset_ * ecal_points_max_) + i) = y;
      input_data_.at(indx).at((coord_z_offset_ * ecal_points_max_) + i) = z;
      // fill recoil_features input
      indx = std::distance(
          input_names_.begin(),
          std::find(input_names_.begin(), input_names_.end(), "ecal_features"));
      input_data_.at(indx).at((feat_e_offset_ * ecal_points_max_) + i) = energy;
      input_data_.at(indx).at((feat_x_offset_ * ecal_points_max_) + i) = x;
      input_data_.at(indx).at((feat_y_offset_ * ecal_points_max_) + i) = y;
      input_data_.at(indx).at((feat_z_offset_ * ecal_points_max_) + i) = z;
    }

    ldmx_log(debug) << "Filled NN input matrix with " << n_hits_filled_
                    << " hits from " << ecal_rec_hits_coll_name_;
  }

  if (!hcal_rec_hits.empty()) {
    unsigned n_hits_filled_ = 0;
    for (unsigned i = 0; i < hcal_rec_hits.size(); i++) {
      if (i >= hcal_points_max_) {
        break;
      }
      n_hits_filled_++;
      // load data from hit
      ldmx::HcalHit hit = hcal_rec_hits.at(i);
      auto x = hit.getXPos();
      auto y = hit.getYPos();
      auto z = hit.getZPos();
      auto energy = std::log(hit.getEnergy());
      // fill recoil_points input
      int indx = std::distance(
          input_names_.begin(),
          std::find(input_names_.begin(), input_names_.end(), "hcal_points"));
      input_data_.at(indx).at((coord_x_offset_ * hcal_points_max_) + i) = x;
      input_data_.at(indx).at((coord_y_offset_ * hcal_points_max_) + i) = y;
      input_data_.at(indx).at((coord_z_offset_ * hcal_points_max_) + i) = z;
      // fill recoil_features input
      indx = std::distance(
          input_names_.begin(),
          std::find(input_names_.begin(), input_names_.end(), "hcal_features"));
      input_data_.at(indx).at((feat_e_offset_ * hcal_points_max_) + i) = energy;
      input_data_.at(indx).at((feat_x_offset_ * hcal_points_max_) + i) = x;
      input_data_.at(indx).at((feat_y_offset_ * hcal_points_max_) + i) = y;
      input_data_.at(indx).at((feat_z_offset_ * hcal_points_max_) + i) = z;
    }

    ldmx_log(debug) << "Filled NN input matrix with " << n_hits_filled_
                    << " hits from " << hcal_rec_hits_coll_name_;
  }
}

void ENPnetClassifier::processOutputs(std::vector<float>& outputs) {
  auto max_val_iter = std::max_element(outputs.begin(), outputs.end());
  unsigned max_val_indx = max_val_iter - outputs.begin();
  result_.setProbs(outputs);
  result_.setLeadingPID(max_val_indx);
  result_.setLeadingConfidence(*max_val_iter);
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::ENPnetClassifier);