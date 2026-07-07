from LDMX.Framework import Processor, processor
from .make_path import make_bdt_path

@processor("ecal::ENPnetClassifier", "Ecal")
class ENPnetClassifier(Processor):
    model_path:str = make_bdt_path("en_pnet_classifier")
    digi_tracker_coll_name:str  = "DigiRecoilSimHits"
    digi_tracker_pass_name:str  = ""
    ecal_rec_hits_coll_name:str = "EcalRecHits"
    ecal_rec_hits_pass_name:str = ""
    hcal_rec_hits_coll_name:str = "HcalRecHits"
    hcal_rec_hits_pass_name:str = ""