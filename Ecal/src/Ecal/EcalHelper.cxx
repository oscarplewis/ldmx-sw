#include "Ecal/EcalHelper.h"

namespace ecal {

// old trackProp function
std::vector<float> trackProp(const ldmx::Tracks &tracks,
                             ldmx::TrackStateType ts_type,
                             const std::string &ts_title) {
  // Vector to hold the new track state variables
  std::vector<float> new_track_states;

  // Return if no tracks
  if (tracks.empty()) return new_track_states;

  // Otherwise loop on the tracks
  for (auto &track : tracks) {
    // Get track state for ts_type
    auto trk_ts = track.getTrackState(ts_type);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState &ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.params_.size() < 5) continue;

    float track_state_loc0 = static_cast<float>(ecal_track_state.params_[0]);
    float track_state_loc1 = static_cast<float>(ecal_track_state.params_[1]);
    // param 2 = phi (azimuthal), param 3 = theta (polar)
    // param 4 = QoP
    // ACTS (local)  to  LDMX (global) coordinates: (y_,z_,x_)->  (x_,y_,z_)
    // convert qop [1/GeV] to p [MeV]
    float p_track_state = (-1 / ecal_track_state.params_[4]) * 1000;
    // p * sin(theta) * sin(phi)
    float recoil_mom_x = p_track_state * sin(ecal_track_state.params_[3]) *
                         sin(ecal_track_state.params_[2]);
    // p * cos(theta)
    float recoil_mom_y = p_track_state * cos(ecal_track_state.params_[3]);
    // p * sin(theta) * cos(phi)
    float recoil_mom_z = p_track_state * sin(ecal_track_state.params_[3]) *
                         cos(ecal_track_state.params_[2]);

    // Store the new track state variables
    new_track_states.push_back(track_state_loc0);
    new_track_states.push_back(track_state_loc1);
    // z_-position at the ECAL (4) or Target (1)
    if (ts_type == 4) {
      // this should match `ECAL_SCORING_PLANE` in CKFProcessor
      new_track_states.push_back(240.5);
    } else if (ts_type == 1) {
      // This should match `target_surface` in CKFProcessor
      new_track_states.push_back(0.0);
    }

    new_track_states.push_back(recoil_mom_x);
    new_track_states.push_back(recoil_mom_y);
    new_track_states.push_back(recoil_mom_z);

    // Break after getting the first valid track state
    // TODO: interface this with CLUE to make sure the propageted track
    //       has an associated cluster in the ECAL
    break;
  }

  return new_track_states;
}

std::vector<float> recoilTrackProp(const ldmx::Tracks &tracks,
                                   ldmx::TrackStateType ts_type,
                                   const std::string &ts_title) {
  // Vector to hold the new track state variables
  std::vector<float> new_track_states;
  float pt_max{0.0};

  // Return if no tracks
  if (tracks.empty()) return new_track_states;

  // Otherwise loop on the tracks
  for (auto &track : tracks) {
    // Get track state for ts_type
    auto trk_ts = track.getTrackState(ts_type);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState &ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.params_.size() < 5) continue;

    float track_state_loc0 = static_cast<float>(ecal_track_state.params_[0]);
    float track_state_loc1 = static_cast<float>(ecal_track_state.params_[1]);
    // param 2 = phi (azimuthal), param 3 = theta (polar)
    // param 4 = QoP
    // discard positively charged and neutral tracks
    if (ecal_track_state.params_[4] >= 0.0) continue;
    // ACTS (local)  to  LDMX (global) coordinates: (y_,z_,x_)->  (x_,y_,z_)
    // convert qop [1/GeV] to p [MeV]
    float p_track_state = (-1 / ecal_track_state.params_[4]) * 1000;
    // p * sin(theta) * sin(phi)
    float recoil_mom_x = p_track_state * sin(ecal_track_state.params_[3]) *
                         sin(ecal_track_state.params_[2]);
    // p * cos(theta)
    float recoil_mom_y = p_track_state * cos(ecal_track_state.params_[3]);
    // p * sin(theta) * cos(phi)
    float recoil_mom_z = p_track_state * sin(ecal_track_state.params_[3]) *
                         cos(ecal_track_state.params_[2]);

    // Check that the transverse momentum is greater than the previous
    // apprehended track
    float recoil_mom_trns =
        sqrt((recoil_mom_x * recoil_mom_x) + (recoil_mom_y * recoil_mom_y));
    if (recoil_mom_trns < pt_max) continue;

    // by this point, we want to keep this track
    pt_max = recoil_mom_trns;
    // Store the new track state variables
    if (new_track_states.empty()) {
      new_track_states.assign(6, 0.0);
    }
    new_track_states[0] = track_state_loc0;
    new_track_states[1] = track_state_loc1;
    // z_-position at the ECAL (4) or Target (1)
    if (ts_type == 4) {
      // this should match `ECAL_SCORING_PLANE` in CKFProcessor
      new_track_states[2] = 240.5;
    } else if (ts_type == 1) {
      // This should match `target_surface` in CKFProcessor
      new_track_states[2] = 0.0;
    }

    new_track_states[3] = recoil_mom_x;
    new_track_states[4] = recoil_mom_y;
    new_track_states[5] = recoil_mom_z;

    // No break after first track to allow for the possibility of multiple
    // electrons.
  }

  return new_track_states;
}

// Finds a specified N number of electron tracks valid at the Ecal
std::vector<std::vector<float>> eleTrackProp(const ldmx::Tracks &tracks,
                                             int ele_count) {
  // Vector variable to hold track state parameters, indexed by total momentum
  std::vector<std::pair<float, std::vector<float>>> new_track_states;

  // Return empty vector if no tracks
  if (tracks.empty()) return {};

  // Otherwise loop on the tracks
  for (auto &track : tracks) {
    // Vector to hold track state parameters for a single track
    std::vector<float> track_state_vars;
    // Get track state for Ecal
    auto trk_ts = track.getTrackState(ldmx::TrackStateType::AtECAL);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState &ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.params_.size() < 5) continue;

    float track_state_loc0 = static_cast<float>(ecal_track_state.params_[0]);
    float track_state_loc1 = static_cast<float>(ecal_track_state.params_[1]);
    // param 2 = phi (azimuthal), param 3 = theta (polar)
    // param 4 = QoP
    // discard positively charged and neutral tracks
    if (ecal_track_state.params_[4] >= 0.0) continue;

    // ACTS (local)  to  LDMX (global) coordinates: (y_,z_,x_)->  (x_,y_,z_)
    // convert qop [1/GeV] to p [MeV]
    float p_track_state = (-1.0f / ecal_track_state.params_[4]) * 1000;
    // p * sin(theta) * sin(phi)
    float recoil_mom_x = p_track_state * sin(ecal_track_state.params_[3]) *
                         sin(ecal_track_state.params_[2]);
    // p * cos(theta)
    float recoil_mom_y = p_track_state * cos(ecal_track_state.params_[3]);
    // p * sin(theta) * cos(phi)
    float recoil_mom_z = p_track_state * sin(ecal_track_state.params_[3]) *
                         cos(ecal_track_state.params_[2]);
    // Calculate total momentum
    float recoil_momentum =
        sqrt((recoil_mom_x * recoil_mom_x) + (recoil_mom_y * recoil_mom_y) +
             (recoil_mom_z * recoil_mom_z));

    // store state variables
    track_state_vars.push_back(track_state_loc0);
    track_state_vars.push_back(track_state_loc1);
    // push z coord of ECAL_SCORING_PLANE in CKFProcessor
    track_state_vars.push_back(240.5);
    track_state_vars.push_back(recoil_mom_x);
    track_state_vars.push_back(recoil_mom_y);
    track_state_vars.push_back(recoil_mom_z);

    // index track by total momentum into output
    new_track_states.emplace_back(recoil_momentum, std::move(track_state_vars));
  }

  // filters to get only the [ele_count] number of highest momentum tracks
  std::sort(
      new_track_states.begin(), new_track_states.end(),
      [](auto &a, auto &b) { return a.first > b.first; });  // sort descending
  if (new_track_states.size() > ele_count) new_track_states.resize(ele_count);

  // Outputs the [ele_count] track states themselves without the momentum
  // indexing
  std::vector<std::vector<float>> max_e_track_states;
  max_e_track_states.reserve(new_track_states.size());
  std::transform(std::make_move_iterator(new_track_states.begin()),
                 std::make_move_iterator(new_track_states.end()),
                 std::back_inserter(max_e_track_states),
                 [](auto &&ts) { return std::move(ts.second); });

  return max_e_track_states;
}

// MIP tracking functions:

float distTwoLines(ROOT::Math::XYZVector v1, ROOT::Math::XYZVector v2,
                   ROOT::Math::XYZVector w1, ROOT::Math::XYZVector w2) {
  ROOT::Math::XYZVector e1 = v1 - v2;
  ROOT::Math::XYZVector e2 = w1 - w2;
  ROOT::Math::XYZVector crs = e1.Cross(e2);
  if (crs.R() == 0) {
    return 100.0;  // arbitrary large number; edge case that shouldn't cause
                   // problems.
  } else {
    return std::abs(crs.Dot(v1 - w1) / crs.R());
  }
}

float distPtToLine(ROOT::Math::XYZVector h1, ROOT::Math::XYZVector p1,
                   ROOT::Math::XYZVector p2) {
  return ((h1 - p1).Cross(h1 - p2)).R() / (p1 - p2).R();
}

}  // namespace ecal
