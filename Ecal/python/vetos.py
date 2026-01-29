"""Configuration for Ecal veto

Examples
--------
    from LDMX.Ecal.ecal_veto import ecal_veto
    p.sequence.append( ecal_veto )
"""

from LDMX.Framework import ldmxcfg


class EcalVetoProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal veto"""

    def __init__(self,name = 'ecal_veto') :
        super().__init__(name,"ecal::EcalVetoProcessor",'Ecal')

        from LDMX.Ecal.make_path import makeBDTPath, makeRoCPath
        self.num_ecal_layers = 32
        self.verbose = False
        self.feature_list_name = "input"
        self.bdt_file = makeBDTPath( "segmip" )
        self.roc_file = makeRoCPath( "RoC_v14_8gev" )
        self.beam_energy = 8000.0  # in MeV
        self.disc_cut = 0.99741

        self.sp_pass_name = ""
        self.collection_name = "EcalVeto"
        self.rec_pass_name = ""
        self.rec_coll_name = "EcalRecHits"
        self.recoil_from_tracking = True
        self.track_collection = "RecoilTracksClean"
        self.inverse_skim = False

        self.sim_particles_passname = ""
        self.sim_particles_coll_name = "SimParticles"
        self.track_pass_name = ""

        self.ecal_simhits_passname = ""
        self.ecal_digis_passname = ""
        self.ecal_rechits_passname = ""
        self.ecal_trig_digis_passname = ""

class EcalMipProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal MIP processor"""

    def __init__(self,name = 'ecalMipTracking') :
        super().__init__(name,"ecal::EcalMipTrackingProcessor",'Ecal')

        self.num_ecal_layers = 32
        self.linreg_radius = 35.0 # in mm
        self.ecal_collection_name = "EcalVeto"
        self.ecal_pass_name = ""
        self.mip_collection_name = "EcalTrajectoryInfo"
        self.mip_pass_name = ""
        self.mip_result_name = "EcalMipInfo"

class EcalPnetVetoProcessor(ldmxcfg.Producer) :
    """Configuration for ParticleNet Ecal Veto
        ParticleNet trained on v14 geometry ecalPN + signal
    """

    def __init__(self,name = 'EcalPnetVeto') :
        super().__init__(name,"ecal::EcalPnetVetoProcessor",'Ecal')

        from LDMX.Ecal.make_path import makeBDTPath
        self.model_path = makeBDTPath("particle_net_ecal_v10")
        self.disc_cut = 0.65
        self.collection_name = "EcalPnetVeto"
        self.rec_coll_name = "EcalRecHits"
        self.ecal_rec_hits_passname = ""
        self.ecal_sp_hits_passname = ""
        self.track_collection = "RecoilTracksClean"
        self.track_pass_name = ""
        self.recoil_from_tracking = True
        
class EcalWABRecRemProcessor(ldmxcfg.Producer):
    """Configuration for the recoil electron removal processor
    for WAB and WAB-like events."""

    def __init__(self, name = 'EcalWABRecRem') :
        super().__init__(name, "ecal::EcalWABRecRemProcessor", 'Ecal')

        from LDMX.Ecal.makePath import makeRoCPath

        self.beam_energy = 8000.0  # in MeV
        self.num_ecal_layers = 32

        self.rem_dist_file = makeRoCPath ( 'RoC_v14_8gev_0.95' )
        self.collection_name_included = 'EcalRecHitsInc'
        self.collection_name_excluded = 'EcalRecHitsExc'
        self.rec_coll_name = 'EcalRecHits'
        self.rec_pass_name = ''
        self.ecal_sim_pass_name = ''
        self.ecal_sp_hits_pass_name = ''

        self.recoil_from_tracking = True
        self.track_coll_name = 'RecoilTracksClean'
        self.track_pass_name = ''

from LDMX.Ecal.makePath import makeBDTPath
ecalveto_wab_reprocessor = EcalVetoProcessor()
vars(ecalveto_wab_reprocessor).update(
    bdt_file = makeBDTPath( 'wab_bdt_2e_1RoC_0.95' ),
    disc_cut = 0.9913983,
    rec_coll_name = 'EcalRecHitsInc',
    collection_name = 'EcalVetoInc',
)
recoil_removal_processing = [EcalWABRecRemProcessor(), ecalveto_wab_reprocessor]