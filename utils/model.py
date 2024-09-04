import os
import numpy as np
from os.path import splitext
from PIL import Image
from time import time
from joblib import dump, load
from pathlib import Path as p
from sklearn.svm import SVC, LinearSVC, OneClassSVM
from sklearn.model_selection import cross_val_score, StratifiedGroupKFold

from .dataset import Dataset
# --------------------------------------------------
try:
    has_tflite = True
    import tflite_runtime.interpreter as tflite
except:
    has_tflite = False
    #logging.exception('\n--- ERROR')
# --------------------------------------------------
try:
    has_trt = True
    import tensorrt as trt
    import pycuda.driver as cuda
    import pycuda.autoinit
except:
    has_trt = False
    #logging.exception('\n--- ERROR')
# --------------------------------------------------




__all__ = ['Model', 'TRT']



class Model:    
    def __init__(self, cnn_path, width) -> None:
        self.cnn_path = cnn_path
        self.width = width
        self.save_path = None
        self.svm = None
        self.classes = None
        
        self.cnn = self._load_cnn(cnn_path)
        self.cnn.allocate_tensors()
        self.input_info = self.cnn.get_input_details()
        self.output_info = self.cnn.get_output_details()
        self._warmup()
    
    def _load_cnn(self, cnn_path):
        print('\nLoading the tflite model ...')
        cnn = tflite.Interpreter(model_path=cnn_path, experimental_delegates=None)
        print('Successfully load the tflite model !!!')
        return cnn
        
    def _warmup(self):
        print('\nWarmup ...')
        x = np.random.randn(1, self.width, self.width, 3).astype(np.float32)        
        for _ in range(1):
            self._infer(x)
        print('Warmup successfully!!!')

    def _infer(self, x):
        cnn = self.cnn
        cnn.set_tensor(self.input_info[0]['index'], x)
        cnn.invoke()
        output = cnn.get_tensor(self.output_info[0]['index'])
        return output

    def _preprocess(self, img):
        img = img.convert("RGB").resize((self.width, self.width), Image.BILINEAR)
        img_array = np.array(img, dtype=np.float32)[None, ...]
        return img_array

    def get_feature(self, data):
        feature = []
        
        if isinstance(data, np.ndarray):
            t1 = time()
            try:
                data = data.reshape((1, self.width, self.width, 3))
            except:
                raise ValueError(f'Error shape: {data.shape}')
            output = self._infer(data.astype(np.float32))
            feature.append(output.squeeze())
            t2 = time()
            print(f'infer time:  {t2 - t1:.4f}')
        
        elif isinstance(data, str):
            t1 = time()
            img_array = self._preprocess(Image.open(data))
            t2 = time()
            output = self._infer(img_array)
            feature.append(output.squeeze())
            t3 = time()
            print(f'load img & preprocess time:  {t2 - t1:.4f},    '
                    f'infer time:  {t3 - t2:.4f}')
        
        elif isinstance(data, Dataset):
            self.classes = data.classes
            
            for path in data:
                t1 = time()
                img_array = self._preprocess(Image.open(path))
                t2 = time()
                output = self._infer(img_array)
                feature.append(output.squeeze())
                t3 = time()
                print(f'load img & preprocess time:  {t2 - t1:.4f},    '
                      f'infer time:  {t3 - t2:.4f}')
        
        else:
            raise ValueError(f'unknown data type "{type(data)}"')
        
        return feature 
    
    def train(self, data, label, dtype=64):        
        dtype = np.float64 if dtype == 64 else np.float32
        data = np.asarray(data, dtype=dtype)        
        if not data.flags['C_CONTIGUOUS']:
            data = np.ascontiguousarray(data)

        svm = LinearSVC()
        svm.fit(data, label)
        self.svm = svm

    def k_cross_val(self, data, label, k, dtype=64):        
        dtype = np.float64 if dtype == 64 else np.float32
        data = np.asarray(data, dtype=dtype)        
        if not data.flags['C_CONTIGUOUS']:
            data = np.ascontiguousarray(data)

        svm = LinearSVC()
        score = cross_val_score(svm, data, label, cv=k)
        return score.mean(), score.std()

    def k_group_cross_val(self, data, label, group_label, k, dtype=64):        
        t1 = time()
        dtype = np.float64 if dtype == 64 else np.float32
        label = np.asarray(label)
        data = np.asarray(data, dtype=dtype)        
        if not data.flags['C_CONTIGUOUS']:
            data = np.ascontiguousarray(data)
        
        print('\nBegin group cross validation')
        
        sfg = StratifiedGroupKFold(k, True, 10)
        for tr, val in sfg.split(data, label, group_label):
            svm = LinearSVC()
            svm.fit(data[tr], label[tr])
            score = svm.score(data[val], label[val])
            print('\nscore:', score)
        t2 = time()
        print(t2 - t1)
        
    def _load_svm(self, project_path):
        if project_path is None:
            raise ValueError(f'You should set project first !!!\n')
        
        self.save_path = project_path/'elic'
        if self.save_path.exists():
            obj = load(project_path/'elic')
            self.svm = obj['model']
            self.classes = obj['class']

    def save(self):
        dump({"model": self.svm, 
              "cnn": self.cnn_path, 
              "class": self.classes}, 
             self.save_path)
          
    def predict_label(self, data):
        return self.svm.predict(data)
    
    def predict_class(self, data):
        return self.classes[self.svm.predict(data).item()]

    def predict_score(self, data):
        return self.svm.decision_function(data)
    
# ------------------------------------------------------------------------

class TRT:    
    def __init__(self, cnn_path, width) -> None:
        self.cnn_path = cnn_path
        self.width = width
        self.save_path = None
        self.svm = None
        self.cnn = self._load_cnn(cnn_path)
        self._warmup()
    
    def _preprocess(self, img):
        img = img.convert("RGB").resize((self.width, self.width), Image.BILINEAR)
        img_array = np.array(img, dtype=np.float32)[None, ...]
        img_array /= 255
        img_array = img_array.transpose(0, 3, 1, 2)
        return img_array

    def _load_cnn(self, cnn_path):
        print('\nLoading the TensorRT model ...')
        with open(cnn_path, "rb") as f, trt.Runtime(trt.Logger()) as runtime:
            cnn = runtime.deserialize_cuda_engine(f.read())
        print('Successfully load the TensorRT model !!!')
        return cnn
        
    def _warmup(self):
        print('\nWarmup ...')
        x = np.random.randn(1, 3, self.width, self.width).astype(np.float32)        
        for _ in range(1):
            self._infer(x)
        print('Warmup successfully!!!')
        
    def _infer(self, x):
        cnn = self.cnn
        with cnn.create_execution_context() as context:
            context.set_binding_shape(cnn.get_binding_index("input"), 
                                      (1, 3, self.width, self.width))
            bindings = []
            for binding in cnn:
                binding_idx = cnn.get_binding_index(binding)
                size = trt.volume(context.get_binding_shape(binding_idx))
                dtype = trt.nptype(cnn.get_binding_dtype(binding))
                if cnn.binding_is_input(binding):
                    input_buffer = np.ascontiguousarray(x)
                    input_memory = cuda.mem_alloc(x.nbytes)
                    bindings.append(int(input_memory))
                else:
                    output_buffer = cuda.pagelocked_empty(size, dtype)
                    output_memory = cuda.mem_alloc(output_buffer.nbytes)
                    bindings.append(int(output_memory))

            stream = cuda.Stream()
            cuda.memcpy_htod_async(input_memory, input_buffer, stream)
            context.execute_async_v2(bindings=bindings, stream_handle=stream.handle)
            cuda.memcpy_dtoh_async(output_buffer, output_memory, stream)
            stream.synchronize()

        return output_buffer

    def get_feature(self, dataset):
        self.ds = dataset
        feature_vector = []
        for path in dataset:
            t1 = time()
            img_array = self._preprocess(Image.open(path))
            t2 = time()
            output = self._infer(img_array)
            feature_vector.append(output.squeeze())
            t3 = time()
            print(f'load img & preprocess time:  {t2 - t1:.4f},    infer time:  {t3 - t2:.4f}')
        return feature_vector 
    
    def train(self, data, dtype=64):        
        dtype = np.float64 if dtype == 64 else np.float32
        data = np.asarray(data, dtype=dtype)        
        if not data.flags['C_CONTIGUOUS']:
            data = np.ascontiguousarray(data)
        #model = SVC(kernel=kernel, decision_function_shape="ovr")
        svm = LinearSVC()
        svm.fit(data, self.ds.label)
        self.svm = svm

    def save(self):
        dump({"model": self.svm, 
              "cnn": self.cnn_path, 
              "class": self.ds.classes}, 
             self.save_path)
        
    def _load(self):
        obj = load(self.save_path)
        svm = obj['model']
        self.classes = obj['class']
        return svm
        
    def predict_label(self, data):
        return self.svm.predict(data)
    
    def predict_class(self, data):
        return self.classes[self.svm.predict(data).item()]

    
