import numpy as np
import pandas as pd
from pathlib import Path as p
from datetime import datetime






class Report:
    def __init__(self, save_dir, predict, score, dataset, is_test) -> None:
        strDt = datetime.now().strftime("%Y%m%d_%H%M%S")
        prefix = 'Test' if is_test else 'Register'
        
        self.ds = dataset
        self.pred = predict
        self.score = score
        self.save_dir = save_dir
        self.writer = pd.ExcelWriter(f'{save_dir}/{prefix}_Report_{strDt}.xlsx', 
                                     'xlsxwriter')
        (self.pr_cfm,
         self.acc,
         self.bin_acc,
         self.wrong) = self._statistic()
        
    # ----------------------------------------------------------------------------
     
    def _sheet_accuracy(self):
        df = pd.DataFrame(
            [['/'.join(p(self.save_dir).parts[-2:]), self.acc, self.bin_acc]],
            columns=["Data", "Acc (%)", "Binary Acc (%)"]    
        )
        df.to_excel(self.writer, "Accuracy", index=False)

    # ----------------------------------------------------------------------------

    def _sheet_wrong(self):
        df = pd.DataFrame(self.wrong, 
                          columns=["Path", "True Class", "Predict Class"])
        df.to_excel(self.writer, "Miss", index=False)

    # ----------------------------------------------------------------------------

    def _sheet_confussion_matrix(self):
        ds = self.ds
        cls_num = len(ds.classes)
        col = [["True"] * (cls_num + 2), ds.classes + ["Total", "Precision(%)"]]
        row = [["Predict"] * (cls_num + 2), ds.classes + ["Total", "Recall(%)"]]
        df = pd.DataFrame(self.pr_cfm, columns=col, index=row)
        df.to_excel(self.writer, "C-Matrix")

    # ----------------------------------------------------------------------------

    def save(self):
        self._sheet_accuracy()
        self._sheet_wrong()
        self._sheet_confussion_matrix()
        self.writer.close()

    # ----------------------------------------------------------------------------

    def accuracy(self):
        return self.acc, self.bin_acc

    # ----------------------------------------------------------------------------

    def _statistic(self):
        pred = self.pred
        ds = self.ds 
        
        cls_num = len(ds.classes)        
        cfm = np.zeros((cls_num, cls_num), dtype=np.int32)
        
        wrong = []
        for _pred, _lb, _name in zip(pred, ds.label, ds.path_name):
            cfm[_pred, _lb] += 1
            
            if _pred ^ _lb:
                wrong.append([_name, ds.classes[_lb], ds.classes[_pred]])
        
        # binary acc
        if ds.has_multi_ng:
            bin_acc = ((cfm[-1, -1] + cfm[:-1, :-1].sum()) / cfm.sum() * 100).round(2)
        else:
            bin_acc = None
        acc = (cfm[range(cls_num), range(cls_num)].sum() / cfm.sum() * 100).round(2)
        
        pr_cfm = self._PR_confussionMatrix(cfm)
        
        return pr_cfm, acc, bin_acc, wrong 
        
    # ----------------------------------------------------------------------------

    def _PR_confussionMatrix(self, cfm):
        cls_num = len(self.ds.classes)
        
        total = cfm.sum()
        correct_num = cfm[range(cls_num), range(cls_num)]
        gt_num, pred_num = cfm.sum(axis=0), cfm.sum(axis=1)
        
        pr_cfm = np.zeros((cls_num + 2, cls_num + 2))
        pr_cfm[:] = None
        pr_cfm[:-2, :-2] = cfm
        pr_cfm[-2, :-2] = gt_num
        pr_cfm[:-2, -2] = pred_num
        pr_cfm[-2, -2] = cfm.sum()
        pr_cfm[-1, :-2] = (correct_num / gt_num * 100).round(2)
        pr_cfm[:-2, -1] = (correct_num / pred_num * 100).round(2)
        pr_cfm[-1, -1] = (correct_num.sum() / total * 100).round(2)
        
        return pr_cfm

    def _softmax(self, x):
        exp = np.exp(x)
        y = exp / exp.sum(axis=1, keepdims=True)
        return y

    def _sigmoid(self, x):
        y = 1 / (1 + np.exp(-x))
        return y
