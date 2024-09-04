import yaml
import logging
import threading

from pathlib import Path as p
from time import time

from .dataset import Dataset
from .model import Model
from .report import Report






class Engine:    
    def __init__(self) -> None:
        logging.basicConfig(filename="info.log", 
                            filemode='w', 
                            level=logging.INFO, 
                            format="\n%(asctime)s %(message)s")
        
        self.config = self.__load_config("config.yaml")
        self.proj_root = p(self.config['Project Root'])
        self.proj_name = None
        self.proj_path = None
        if not self.proj_root.exists():
            raise ValueError(f'Project root "{self.proj_root}" not exist !!!\n'
                              'Please recheck the content of config.yaml')
                    
        self.gCNN_PATH = self.config['Model']['path']
        self.gIMGSZ = self.config['Model']['width']
        self.gCNN = Model(self.gCNN_PATH, self.gIMGSZ)        
        
##---------------------------------------------------------------------------##            
    def init_project(self, project_name):
        self.proj_name = project_name
        self.proj_path = self.proj_root/project_name
        if not self.proj_path.exists():
            raise ValueError(f'Project path "{self.proj_path}" not exist !!!\n')

        self.gCNN._load_svm(self.proj_path)

    def rename(self, name):
        self.init_project(name)

    def project(self):
        if self.proj_name is None:
            return 'You have not set the project' 
        else:
            return self.proj_name
           
    def has_learned(self):
        self._check()
        elic_path = self.proj_path/'elic'
        return elic_path.exists()
                      
##---------------------------------------------------------------------------##
    def learn(self):
        cfg = self.config
        
        self._check()
        
        target_dir = self.proj_path/'Register'        
        if not target_dir.exists():
            raise FileNotFoundError(f'No such path {target_dir}')
        
        t1 = time()
        ds = Dataset(target_dir, True, cfg['kfold'])
        t2 = time()
        ftv = self.gCNN.get_feature(ds)
        t3 = time()
        if cfg['kfold'] > 1:
            #mean, std = self.gCNN.k_cross_val(ftv, ds.label, cfg['kfold'])
            self.gCNN.k_group_cross_val(ftv, 
                                        ds.label, 
                                        ds.group_label, 
                                        cfg['kfold'])
        t4 = time()
        self.gCNN.train(ftv, ds.label)
        t5 = time()
        self.gCNN.save()
        t6 = time()
        report = Report(target_dir, 
                        self.gCNN.predict_label(ftv), 
                        self.gCNN.predict_score(ftv), 
                        ds, 
                        False)
        acc, bin_acc = report.accuracy()
        t7 = time()
        _t = threading.Thread(target=report.save())
        _t.start()

        logging.info(f"{'-'*60}  Train"
                     f"\nmake dataset time: {t2 - t1:.4f}"
                     f"\nget feature: {t3 - t2:.4f}"
                     f"\n{cfg['kfold']}-fold Cross validation: {t4 - t3:.4f}"
                     f"\ntrain classifier: {t5 - t4:.4f}"
                     f"\nsave classifier: {t6 - t5:.4f}"
                     f"\nstatistic score: {t7 - t6:.4f}"
                     f"\nTotal: {t7 - t1:.4f}")

        return acc, round(t6 - t1, 2)

#-----------------------------------------------------------------------------#    
    def eval(self):
        self._check()
        
        target_dir = self.proj_path/'Test'        
        if not target_dir.exists():
            raise FileNotFoundError(f'No such path {target_dir}')
        
        t1 = time()
        ds = Dataset(target_dir, False)
        t2 = time()
        ftv = self.gCNN.get_feature(ds)
        t3 = time()
        report = Report(target_dir, 
                        self.gCNN.predict_label(ftv), 
                        self.gCNN.predict_score(ftv), 
                        ds, 
                        True)
        acc, bin_acc = report.accuracy()
        t4 = time()
        _t = threading.Thread(target=report.save())
        _t.start() 

        logging.info(
            f"{'-'*60}  Evaluate"
            f"\nmake dataset time: {t2 - t1:.4f}"
            f"\nget feature: {t3 - t2:.4f}"
            f"\nstatistic score: {t4 - t3:.4f}"
            f"\nTotal: {t4 - t1:.4f}"
        )
        
        return acc, round(t4 - t1, 2)
    
#-----------------------------------------------------------------------------#    
    def infer_from_path(self, path):     
        t1 = time()
        ftv = self.gCNN.get_feature(str(path))
        t2 = time()
        pred_class = self.gCNN.predict_class(ftv)
        t3 = time()
        
        logging.info(f"{'-'*60}  Evaluate"
                     f"\nget feature: {t2 - t1:.4f}"
                     f"\nmodel predict: {t3 - t2:.4f}"
                     f"\nTotal: {t3 - t1:.4f}")
        
        return pred_class, round(t3 - t1, 3)
    
#-----------------------------------------------------------------------------#    
    def infer_from_array(self, array):
        t1 = time()
        ftv = self.gCNN.get_feature(array)
        t2 = time()
        pred_class = self.gCNN.predict_class(ftv)
        t3 = time()
        
        logging.info(f"{'-'*60}  Evaluate"
                     f"\nget feature: {t2 - t1:.4f}"
                     f"\nmodel predict: {t3 - t2:.4f}"
                     f"\nTotal: {t3 - t1:.4f}")
        
        return pred_class, round(t3 - t1, 3)
        
##---------------------------------------------------------------------------##    
    def __load_config(self, configFile):
        if p(configFile).is_file():
            with open(configFile) as f:
                return yaml.safe_load(f)
        else:
            raise Exception()

    def _check(self):
        if self.proj_path is None:
            raise ValueError('You should set project first')
