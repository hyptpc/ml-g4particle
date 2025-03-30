import torch
from torch.utils.data import Dataset
import numpy as np
import uproot3


class CustomRootDataset(Dataset):
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
            "energy_layers": self.energy_layers[idx].unsqueeze(-1),  # LSTMモデルへの受け渡し用に(batch, 32) → (batch, 32, 1)に変換
        }
        target = self.particles[idx]
        return {"input": inputs, "target": target}
