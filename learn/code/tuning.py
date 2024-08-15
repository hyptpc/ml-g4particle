# ***************************************
#
# Hyperparameter optimization with Optuna
#
# 2024/08/14 K.Amemiya
# ***************************************

import torch.multiprocessing
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.data import DataLoader, Dataset
import uproot3
import matplotlib.pyplot as plt
import optuna
import optuna.visualization as vis
import os

"""データセットの定義"""


class CustomRootDataset(Dataset):
    def __init__(self, root_file_path, tree_name, sample_fraction):
        self.root_file_path = root_file_path
        self.tree_name = tree_name
        self.sample_fraction = sample_fraction
        self._load_data()

    def _load_data(self):
        self.root_file = uproot3.open(self.root_file_path)
        self.tree = self.root_file[self.tree_name]

        self.particles = self.tree["pid"].array()
        self.mom = self.tree["mom"].array()
        self.tof = self.tree["tof"].array()
        self.energy = self.tree["ene"].array()

        num_samples = int(len(self.particles) * self.sample_fraction)
        print(f"Sampling {num_samples} out of {len(self.particles)} samples")
        indices = torch.randperm(len(self.particles))[:num_samples]
        self.particles = self.particles[indices]
        self.mom = self.mom[indices]
        self.tof = self.tof[indices]
        self.energy = self.energy[indices]

        self.particles = torch.tensor(self.particles, dtype=torch.long)
        self.mom = torch.tensor(self.mom, dtype=torch.float32)
        self.tof = torch.tensor(self.tof, dtype=torch.float32)
        self.energy = torch.tensor(self.energy, dtype=torch.float32)

    def __len__(self):
        return len(self.particles)

    def __getitem__(self, idx):
        inputs = torch.stack((self.mom[idx], self.tof[idx], self.energy[idx]))
        target = self.particles[idx]
        return {"input": inputs, "target": target}


""" モデルの定義 """


class MLP_NN(nn.Module):
    def __init__(self, input_size, hidden_sizes, output_size):
        super().__init__()
        self.hidden_layers = nn.ModuleList()
        self.hidden_layers.append(nn.Linear(input_size, hidden_sizes[0]))
        for i in range(1, len(hidden_sizes)):
            self.hidden_layers.append(nn.Linear(hidden_sizes[i - 1], hidden_sizes[i]))
        self.output_layer = nn.Linear(hidden_sizes[-1], output_size)

    def forward(self, x):
        for layer in self.hidden_layers:
            x = F.relu(layer(x))
        x = self.output_layer(x)
        return x


""" training function """


def train_model(model, train_loader, loss_function, optimizer, device="cpu"):
    train_loss = 0.0
    num_train = 0
    train_accuracy = 0.0
    model.train()
    for batch in train_loader:
        num_train += len(batch["target"])
        inputs, labels = batch["input"].to(device), batch["target"].to(device)
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = loss_function(outputs, labels)
        loss.backward()
        optimizer.step()
        predictions = torch.argmax(outputs, dim=1)
        train_loss += loss.item()
        train_accuracy += (predictions == labels).sum().item()
    train_loss /= len(train_loader)
    train_accuracy /= num_train
    return train_loss, train_accuracy


""" val function """


def val_model(model, val_loader, loss_function, device="cpu"):
    val_loss = 0.0
    val_accuracy = 0.0
    num_val = 0
    model.eval()
    with torch.no_grad():
        for batch in val_loader:
            num_val += len(batch["target"])
            inputs, labels = batch["input"].to(device), batch["target"].to(device)
            outputs = model(inputs)
            loss = loss_function(outputs, labels)
            val_loss += loss.item()
            predictions = torch.argmax(outputs, dim=1)
            val_accuracy += (predictions == labels).sum().item()
    val_loss /= len(val_loader)
    val_accuracy /= num_val
    return val_loss, val_accuracy


""" Optuna objective function """


def objective(trial):
    hidden_layers = trial.suggest_int("hidden_layers", 2, 6)
    # 各hidden_layerごとに異なるhidden_sizeを探索
    hidden_sizes = [
    trial.suggest_categorical(f"hidden_size_{i}", [128, 256, 512, 1024])
    for i in range(hidden_layers)
]

    full_dataset = CustomRootDataset(
        input_root_path, tree_name, sample_fraction=0.1
    )  # 50,000 samples
    dataset_size = len(full_dataset)
    train_size = int(0.8 * dataset_size)
    val_size = dataset_size - train_size
    train_dataset, val_dataset = torch.utils.data.random_split(
        full_dataset, [train_size, val_size]
    )
    train_loader = DataLoader(
        train_dataset, batch_size=256, shuffle=True, num_workers=8
    )
    val_loader = DataLoader(val_dataset, batch_size=256, shuffle=True, num_workers=8)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = MLP_NN(input_size=3, hidden_sizes=hidden_sizes, output_size=3).to(device)

    loss_function = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(), lr=0.01)

    min_val_loss = float("inf")
    for epoch in range(1, n_epoch + 1):
        train_loss, train_accuracy = train_model(
            model, train_loader, loss_function, optimizer, device=device
        )
        val_loss, val_accuracy = val_model(
            model, val_loader, loss_function, device=device
        )

        trial.report(val_loss, epoch)
        if trial.should_prune():
            raise optuna.exceptions.TrialPruned()

    return val_loss


""" Studyの履歴を保存して再開できるようにする """


def save_trial_history(study, output_path):
    trials = study.trials
    history = {"trial": [], "hidden_layers": [], "hidden_sizes": [], "val_loss": []}
    for trial in trials:
        history["trial"].append(trial.number)
        history["hidden_layers"].append(trial.params["hidden_layers"])
        hidden_sizes = [
            trial.params.get(f"hidden_size_{i}", None)
            for i in range(trial.params["hidden_layers"])
        ]
        history["hidden_sizes"].append(hidden_sizes)
        history["val_loss"].append(trial.value)

    with open(output_path, "w") as f:
        for key in history:
            f.write(f"{key}," + ",".join(map(str, history[key])) + "\n")


""" 再開用の機能を持った履歴の読み込み """


def load_trial_history(input_path):
    history = {
        "trial": [],
        "hidden_layers": [],
        "hidden_sizes": [],
        "val_loss": [],
    }
    with open(input_path, "r") as f:
        lines = f.readlines()
        keys = lines[0].strip().split(",")
        # Initialize history lists for each key
        for key in keys:
            history[key] = []
        # Read data lines, skipping the header
        for line in lines[1:]:
            values = line.strip().split(",")
            for i, key in enumerate(keys):
                if key == "hidden_sizes":
                    history[key].append([int(v) for v in values[i].strip("[]").split()])
                elif key == "trial":
                    history[key].append(int(values[i]))
                else:
                    history[key].append(float(values[i]))
    return history


""" Contour PlotとHyperparameter Importancesの保存 """


def plot_hyperparameter_importances(study):
    fig = vis.plot_param_importances(study)
    fig.write_image(
        "/home/had/kohki/work/ML/2024/learn/figures/hyperparameter_importances.png"
    )


def plot_contour(study):
    fig = vis.plot_contour(study)
    fig.write_image("/home/had/kohki/work/ML/2024/learn/figures/contour_plot.png")


if __name__ == "__main__":
    n_epoch = 50
    input_root_path = "/home/had/kohki/work/ML/2024/geant/rootfiles/input_test.root"
    tree_name = "tree_32layer"

    # Check if a previous study exists
    study_name = "hidden_study"
    storage_name = "sqlite:///optuna.db"
    if os.path.exists("optuna.db"):
        study = optuna.load_study(study_name=study_name, storage=storage_name)
    else:
        study = optuna.create_study(
            study_name=study_name, storage=storage_name, direction="minimize"
        )

    study.optimize(objective, n_trials=100)

    # トライアル履歴を保存
    save_trial_history(
        study, "/home/had/kohki/work/ML/2024/learn/csv/trial_history.csv"
    )

    # ハイパーパラメータの重要性をプロットして保存
    plot_hyperparameter_importances(study)

    # Contour Plotをプロットして保存
    plot_contour(study)
