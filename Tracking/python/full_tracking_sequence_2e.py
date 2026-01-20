# Load the tracking module
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector('ldmx-det-v15-8gev')

# RIGHT NOW OVERLAYPRODUCER CAN'T SUPPORT TRUTH TRACKING OR DQM

# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to reduce combinatorics
digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
digi_tagger.hit_collection = "TaggerSimHitsOverlay"
digi_tagger.out_collection = "DigiTaggerSimHitsOverlay"

# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digi_recoil.hit_collection = "RecoilSimHitsOverlay"
digi_recoil.out_collection = "DigiRecoilSimHitsOverlay"

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at the track
# parameters and the impact parameters at the target or generation point. For the tagger one should look
# for compatibility with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor("SeedTagger")
seeder_tagger.input_hits_collection =  digi_tagger.out_collection
seeder_tagger.out_seed_collection = "TaggerRecoSeedsOverlay"
seeder_tagger.pmin  = 0.03
seeder_tagger.pmax  =  63.0
seeder_tagger.d0min =  -36.9
seeder_tagger.d0max = 31.5
seeder_tagger.z0max = 54.6
seeder_tagger.thetacut = 0.26
seeder_tagger.phicut =  0.84

#Seed finder processor - Recoil
seeder_recoil = tracking.SeedFinderProcessor("SeedRecoil")
seeder_recoil.perigee_location = [0.,0.,0.]
seeder_recoil.input_hits_collection =  digi_recoil.out_collection
seeder_recoil.out_seed_collection = "RecoilRecoSeedsOverlay"
seeder_recoil.bfield = 1.5
seeder_recoil.pmin  =   0.04
seeder_recoil.pmax  =  819.0
seeder_recoil.d0min =  -40.2
seeder_recoil.d0max = 36.5
seeder_recoil.z0max = 40.5
seeder_recoil.thetacut =  1.5
seeder_recoil.phicut =  1.6

# CKF track finding for tagger tracker using seeds.
tracking_tagger  = tracking.CKFProcessor("Tagger_TrackFinder")
tracking_tagger.taggerTracking = True
# for truth seed based case use 
# tracking_tagger.seed_coll_name = "TaggerTruthSeeds"
tracking_tagger.seed_coll_name = seeder_tagger.out_seed_collection
tracking_tagger.out_trk_collection = "TaggerTracksOverlay"
tracking_tagger.measurement_collection = digi_tagger.out_collection
tracking_tagger.min_hits = 5
tracking_tagger.outlier_pval_ = 16.5

# CKF track finding for recoil tracker using seeds.
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.taggerTracking = False
tracking_recoil.seed_coll_name = seeder_recoil.out_seed_collection
tracking_recoil.out_trk_collection = "RecoilTracksOverlay"
# for truth seed based case use 
# tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.min_hits = 4
tracking_recoil.outlier_pval_ =  22.1

# Greedy ambiguity solver for the tagger
greedy_solver_tagger = tracking.GreedyAmbiguitySolver("GreedySolverTagger")
greedy_solver_tagger.out_trk_collection = "TaggerTracksCleanOverlay"
greedy_solver_tagger.trackCollection = tracking_tagger.out_trk_collection
greedy_solver_tagger.measCollection = digi_tagger.out_collection

# Greedy ambiguity solver for the recoil
greedy_solver_recoil = tracking.GreedyAmbiguitySolver("GreedySolverRecoil")
greedy_solver_recoil.out_trk_collection = "RecoilTracksCleanOverlay"
greedy_solver_recoil.trackCollection = tracking_recoil.out_trk_collection
greedy_solver_recoil.measCollection = digi_recoil.out_collection

# Gaussian sum filter for the tagger
GSF_tagger = tracking.GSFProcessor("Tagger_GSF")
GSF_tagger.taggerTracking = True
GSF_tagger.trackCollection = greedy_solver_tagger.out_trk_collection
GSF_tagger.measCollection  = digi_tagger.out_collection 
GSF_tagger.out_trk_collection = "GSFTaggerTracksOverlay"

# Gaussian sum filter for the recoil
GSF_recoil = tracking.GSFProcessor("Recoil_GSF")
GSF_recoil.taggerTracking = False
GSF_recoil.trackCollection = greedy_solver_recoil.out_trk_collection
GSF_recoil.measCollection  = digi_recoil.out_collection 
GSF_recoil.out_trk_collection = "GSFRecoilTracksOverlay"

# Tracker veto
tracker_veto = tracking.TrackerVetoProcessor()
tracker_veto.tagger_track_collection = "TaggerTracksOverlay"
tracker_veto.recoil_track_collection = "RecoilTracksOverlay"
tracker_veto.output_collection = "TrackerVetoOverlay"

# Put it all together into a single sequance
trk_sequence = [
    digi_tagger,
    digi_recoil,
    seeder_tagger,
    seeder_recoil,
    tracking_tagger,
    tracking_recoil,
    greedy_solver_tagger,
    greedy_solver_recoil,
    GSF_tagger,
    GSF_recoil,
    tracker_veto
]