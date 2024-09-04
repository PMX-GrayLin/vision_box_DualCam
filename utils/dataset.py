import re
import numpy as np
from pathlib import Path as p
import pandas as pd




__all__ = ["Dataset"]


class Dataset:
    def __init__(self, data_dir, is_train=True, kfold=0):
        suffix = {".jpg", "jpeg", ".png", ".bmp"}
        self.pattern = re.compile("\d+")

        self.data_dir = data_dir
        self.classes = [path.name for path in p(data_dir).glob("*") if path.is_dir()]
        self.classes.sort(key=self._sort_class)
        self.label_to_class = dict(zip(range(len(self.classes)), self.classes))
        self.class_to_label = dict(zip(self.classes, range(len(self.classes))))
        
        self.path = []
        self.path_name = []
        self.path_class = []
        self.label = []
        for path in p(data_dir).glob("**/*"):
            if path.suffix.lower() in suffix:
                class_name = path.parts[-2]
                self.path.append(path)
                self.path_name.append(path.name)
                self.path_class.append(class_name)
                self.label.append(self.class_to_label[class_name])

        # ad_label: label for anormally detection                        
        self.has_multi_ng, self.ad_label = self.check_label()

        if is_train and kfold:
            excel = pd.read_excel(p(data_dir).parent/'group.xlsx')
            d = dict(zip(excel['path'], excel['group']))
            self.group_label = [d[path.name] for path in self.path]

    # -----------------------------------------------------------------
    
    def __getitem__(self, index):
        return self.path[index]

    # -----------------------------------------------------------------

    def _sort_class(self, class_name):
        lower_class = class_name.lower()
        number = self.pattern.findall(class_name)
        
        if number:
            if "ok" in lower_class or 'good' in lower_class:
                return 10000 + int(number[0])
            else:
                return int(number[0])
        else:
            if "ok" in lower_class or 'good' in lower_class:
                return 10000
            else:
                return 0

    # -----------------------------------------------------------------

    def check_label(self) -> bool:
        lower_class, num_ng, ad_label = [], 0, None
        for cls in self.classes:
            if 'ng' in cls.lower():
                num_ng += 1
            lower_class.append(cls.lower())
        
        has_multi_ng = True if num_ng > 1 else False
                
        if 'ok' in lower_class:
            ad_label = np.where(np.array(self.label) == lower_class.index('ok'), 1, -1)                
        elif 'good' in lower_class:
            ad_label = np.where(np.array(self.label) == lower_class.index('good'), 1, -1)                
        
        return has_multi_ng, ad_label


