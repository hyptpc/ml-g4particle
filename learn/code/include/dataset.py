import torch
from torch.utils.data import Dataset
import numpy as np
import uproot3
from include.utils import calculate_beta


class RegressionDataset(Dataset):
    def __init__(self, root_file_path, tree_name, layer_num, sample_fraction=1.0):
        self.root_file_path = root_file_path
        self.tree_name = tree_name
        self.layer_num = layer_num
        self.sample_fraction = sample_fraction  # データのサンプリング割合
        self._load_data()

    def _load_data(self):
        self.root_file = uproot3.open(self.root_file_path)
        self.tree = self.root_file[self.tree_name]

        # データのバッチ読み込み
        self.particles = self.tree["pid"].array()
        self.mom = self.tree["mom"].array()

        self.energy_layers = []
        for i in range(self.layer_num):
            energy_layer = self.tree[f"ene_layer{i}"].array()
            self.energy_layers.append(energy_layer)

        self.energy_layers = np.stack(self.energy_layers, axis=-1)

        # データのサンプリング
        num_samples = int(len(self.particles) * self.sample_fraction)
        print(f"Sampling {num_samples} out of {len(self.particles)} samples")
        indices = torch.randperm(len(self.particles))[:num_samples]
        self.particles = self.particles[indices]
        self.mom = self.mom[indices]
        self.energy_layers = self.energy_layers[indices]
        self.beta = np.array([calculate_beta(m, p) for m, p in zip(self.mom, self.particles)])

        # 標準化
        self.energy_layers_mean = np.mean(self.energy_layers, axis=0)
        self.energy_layers_std = np.std(self.energy_layers, axis=0)
        self.energy_layers = (self.energy_layers - self.energy_layers_mean) / self.energy_layers_std
        # self.beta_mean = np.mean(self.beta)
        # self.beta_std = np.std(self.beta)
        # self.beta = (self.beta - self.beta_mean) / self.beta_std

        # Convert to PyTorch tensors
        self.energy_layers = torch.tensor(self.energy_layers, dtype=torch.float32)
        self.beta = torch.tensor(self.beta, dtype=torch.float32)

    def __len__(self):
        return len(self.mom)

    def __getitem__(self, idx):
        inputs = self.energy_layers[idx]
        target = self.beta[idx]
        return {"input": inputs, "target": target}



class ClassificationDataset(Dataset):
    def __init__(self, root_file_path, tree_name, layer_num, sample_fraction=1.0):
        self.root_file_path = root_file_path
        self.tree_name = tree_name
        self.layer_num = layer_num
        self.sample_fraction = sample_fraction  # データのサンプリング割合
        self._load_data()

    def _load_data(self):
        self.root_file = uproot3.open(self.root_file_path)
        self.tree = self.root_file[self.tree_name]

        # データのバッチ読み込み
        self.particles = self.tree["pid"].array()
        self.mom = self.tree["mom"].array()
        self.tof = self.tree["tof"].array()

        self.energy_layers = []
        for i in range(self.layer_num):
            energy_layer = self.tree[f"ene_layer{i}"].array()
            self.energy_layers.append(energy_layer)

        self.energy_layers = np.stack(self.energy_layers, axis=-1)

        # データのサンプリング
        num_samples = int(len(self.particles) * self.sample_fraction)
        print(f"Sampling {num_samples} out of {len(self.particles)} samples")
        indices = torch.randperm(len(self.particles))[:num_samples]
        self.particles = self.particles[indices]
        self.mom = self.mom[indices]
        self.tof = self.tof[indices]
        self.energy_layers = self.energy_layers[indices]
        
        # 標準化
        # self.mom_mean = np.mean(self.mom)
        # self.mom_std = np.std(self.mom)
        # self.mom = (self.mom - self.mom_mean) / self.mom_std
        # self.tof_mean = np.mean(self.tof)
        # self.tof_std = np.std(self.tof)
        # self.tof = (self.tof - self.tof_mean) / self.tof_std
        self.energy_layers_mean = np.mean(self.energy_layers, axis=0)
        self.energy_layers_std = np.std(self.energy_layers, axis=0)
        self.energy_layers = (self.energy_layers - self.energy_layers_mean) / self.energy_layers_std
        
        # Convert to PyTorch tensors
        self.particles = torch.tensor(self.particles, dtype=torch.long)
        self.mom = torch.tensor(self.mom, dtype=torch.float32)
        self.tof = torch.tensor(self.tof, dtype=torch.float32)
        self.energy_layers = torch.tensor(self.energy_layers, dtype=torch.float32)

    def __len__(self):
        return len(self.particles)

    def __getitem__(self, idx):
        inputs = {
            "mom": self.mom[idx],
            "tof": self.tof[idx],
            "energy_layers": self.energy_layers[idx],
        }
        target = self.particles[idx]  # pid
        return {"input": inputs, "target": target}
