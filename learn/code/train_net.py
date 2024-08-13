# ============== HOW TO RUN ==================
#   input: particle, mom, tof, ene
#   output: particle_ML
#
#  created by K.Amemiya (2024/07/01)
# ==========================================
import torch.multiprocessing
torch.multiprocessing.set_sharing_strategy('file_system')
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.data import DataLoader, Dataset
import uproot3
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
import time

n_epoch = 50
num_workers = 8
input_size = 3
hidden1_size = 1024
hidden2_size = 512
output_size = 3


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

        # データのバッチ読み込み
        self.particles = self.tree["pid"].array()
        self.mom = self.tree["mom"].array()
        self.tof = self.tree["tof"].array()
        self.energy = self.tree["ene"].array()
        
        # データのサンプリング
        num_samples = int(len(self.particles) * self.sample_fraction)
        print(f"Sampling {num_samples} out of {len(self.particles)} samples")
        indices = torch.randperm(len(self.particles))[:num_samples]  # Use PyTorch for indexing
        self.particles = self.particles[indices]
        self.mom = self.mom[indices]
        self.tof = self.tof[indices]
        self.energy = self.energy[indices]

        # Convert to PyTorch tensors
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

class ExampleNN(nn.Module):
    def __init__(self, input_size, hidden1_size, hidden2_size, output_size):
        super().__init__()
        self.fc1 = nn.Linear(input_size, hidden1_size)
        self.fc2 = nn.Linear(hidden1_size, hidden2_size)
        self.fc3 = nn.Linear(hidden2_size, output_size)

    def forward(self, x):
        z1 = F.relu(self.fc1(x))
        z2 = F.relu(self.fc2(z1))
        return self.fc3(z2)


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


"""'正答率と損失関数をプロット"""


def plot_figures(
    train_loss_list, val_loss_list, train_accuracy_list, val_accuracy_list, fig_path
):

    plt.figure(figsize=(10, 4))
    plt.subplot(1, 2, 1)
    plt.plot(train_accuracy_list, c="blue", label="train", linestyle="--")
    plt.plot(val_accuracy_list, c="green", label="val", linestyle="-")
    plt.legend()
    plt.xlabel("epoch", fontsize=10)
    plt.ylabel("accuracy", fontsize=10)
    plt.title("Training and validation accuracy")
    plt.grid()
    # plt.tight_layout()
    
    plt.subplot(1, 2, 2)
    plt.plot(train_loss_list, c="blue", label="train", linestyle="--")
    plt.plot(val_loss_list, c="green", label="val", linestyle="-")
    plt.legend()
    plt.xlabel("epoch", fontsize=10)
    plt.ylabel("loss", fontsize=10)
    plt.title("Training and validation loss")
    plt.grid()
    plt.savefig(fig_path)


""" leaning function """


def learning(
    checkpoint_path,
    model,
    train_loader,
    val_loader,
    loss_function,
    optimizer,
    n_epoch,
    device="cpu",
):
    train_loss_list = []
    val_loss_list = []
    train_accuracy_list = []
    val_accuracy_list = []
    epoch_list = []

    if os.path.exists(checkpoint_path):
        checkpoint = torch.load(checkpoint_path)
        model.load_state_dict(checkpoint["model"])
        optimizer.load_state_dict(checkpoint["optimizer"])
        start_epoch = checkpoint["epoch"] + 1  # 学習を前回のエポックから再開
        train_loss_list = checkpoint["train_loss"]
        val_loss_list = checkpoint["val_loss"]
        train_accuracy_list = checkpoint["train_accuracy"]
        val_accuracy_list = checkpoint["val_accuracy"]
        epoch_list = checkpoint["epoch_list"]
        min_loss = checkpoint["min_loss"]
        print(f"Previous checkpoint loaded. Resuming training from epoch {start_epoch}")
    else:
        start_epoch = 1
        min_loss = float("inf")

    # epoch loop
    for epoch in range(start_epoch, n_epoch + 1, 1):
        train_loss, train_accuracy = train_model(
            model, train_loader, loss_function, optimizer, device=device
        )
        val_loss, val_accuracy = val_model(
            model, val_loader, loss_function, device=device
        )
        print(
            f"epoch : {epoch}, train_loss : {train_loss:.5f}, val_loss : {val_loss:.5f}"
        )
        print(
            f"train_accuracy : {train_accuracy:.5f}, val_accuracy : {val_accuracy:.5f}"
        )
        print(
            "-----------------------------------------------"
        )
        train_loss_list.append(train_loss)
        val_loss_list.append(val_loss)
        train_accuracy_list.append(train_accuracy)
        val_accuracy_list.append(val_accuracy)
        epoch_list.append(epoch)
        # ------- pthの保存 -------
        if val_loss < min_loss:
            min_loss = val_loss
            torch.save(
                {
                    "epoch": epoch,
                    "model": model.state_dict(),
                    "optimizer": optimizer.state_dict(),
                    "train_loss": train_loss_list,
                    "val_loss": val_loss_list,
                    "train_accuracy": train_accuracy_list,
                    "val_accuracy": val_accuracy_list,
                    "epoch_list": epoch_list,
                    "min_loss": min_loss,
                },
                checkpoint_path,
            )
    return train_loss_list, val_loss_list, train_accuracy_list, val_accuracy_list



""" 時間表示フォーマット """


def format_time(seconds):
    m, s = divmod(seconds, 60)
    h, m = divmod(m, 60)
    return f"{int(h):02d}:{int(m):02d}:{int(s):02d}"


""" メイン関数 """


def main():
    if len(sys.argv) < 2:
        print("Please provide the layer number as an argument (e.g., 'python3 train_net.py 10').")
        sys.exit(1)
    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    checkpoint_path = f"/home/had/kohki/work/ML/2024/learn/pth/train_{layer_num}layer.pth"
    fig_path = f"/home/had/kohki/work/ML/2024/learn/figures/train_{layer_num}layer.png"
    input_root_path = "/home/had/kohki/work/ML/2024/geant/rootfiles/input_nn.root"
    sample_fraction = 1 # データのサンプリング割合
    
    start_time = time.time()  # 計算時間計測開始

    # データを訓練用とテスト用に8:2で分割し、データセット作成
    print("Loading data ...")
    full_dataset = CustomRootDataset(input_root_path, tree_name, sample_fraction)
    dataset_size = len(full_dataset)
    train_size = int(0.8 * dataset_size)
    val_size = dataset_size - train_size
    train_dataset, val_dataset = torch.utils.data.random_split(
        full_dataset, [train_size, val_size]
    )
    train_loader = DataLoader(
        train_dataset, batch_size=256, shuffle=True, num_workers=num_workers
    )
    val_loader = DataLoader(
        val_dataset, batch_size=256, shuffle=True, num_workers=num_workers
    )

    # モデルの再定義と初期化
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = ExampleNN(input_size, hidden1_size, hidden2_size, output_size).to(device)

    print("*********************************")
    print("number of CPU core: ", os.cpu_count())
    print(f"now using {num_workers} core")
    print("---------------------------------")
    if torch.cuda.is_available():
        n_gpu = torch.cuda.device_count()
        print("number of GPU available: ", n_gpu)  # gpuの使用可能数を取得
        print("current gpu: ", torch.cuda.get_device_name(torch.cuda.current_device()))
        if n_gpu > 1:
            model = nn.DataParallel(model)
    else:
        print("no gpu available")
    print("*********************************")

    model = model.to(device)
    # 損失関数とオプティマイザの定義
    loss_function = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(), lr=0.01)
    # optimizer = optim.Adam(model.parameters(), lr=0.01)

    # learning process
    train_loss_list, val_loss_list, train_accuracy_list, val_accuracy_list = learning(
        checkpoint_path,
        model,
        train_loader,
        val_loader,
        loss_function,
        optimizer,
        n_epoch,
        device=device,
    )

    plot_figures(
        train_loss_list, val_loss_list, train_accuracy_list, val_accuracy_list, fig_path
    )

    end_time = time.time()  # 計算時間計測終了
    total_time = end_time - start_time
    print(f"All finished! processing time:  {format_time(total_time)}")


if __name__ == "__main__":
    main()
