# ==========================================
#   input: ene (multiple layers)
#   output: beta
#
#  created by K.Amemiya (2024/11/14)
# ==========================================

import torch.multiprocessing

torch.multiprocessing.set_sharing_strategy("file_system")
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.data import DataLoader
import os
import sys
import matplotlib.pyplot as plt
from include.dataset import RegressionDataset
from include.models import Regressor
from include.utils import reg_train_model, reg_val_model, load_params


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
    epoch_list = []
    early_stopping_counter = 0
    patience = 20  # 検証損失が改善しないエポック数の上限

    if os.path.exists(checkpoint_path):
        checkpoint = torch.load(checkpoint_path)
        model.load_state_dict(checkpoint["model"])
        optimizer.load_state_dict(checkpoint["optimizer"])
        start_epoch = checkpoint["epoch"] + 1  # 学習を前回のエポックから再開
        train_loss_list = checkpoint["train_loss"]
        val_loss_list = checkpoint["val_loss"]
        epoch_list = checkpoint["epoch_list"]
        min_loss = checkpoint["min_loss"]
        print(f"Previous checkpoint loaded. Resuming training from epoch {start_epoch}")
    else:
        start_epoch = 1
        min_loss = float("inf")

    # epoch loop
    for epoch in range(start_epoch, n_epoch + 1, 1):
        train_loss = reg_train_model(
            model, train_loader, loss_function, optimizer, device=device
        )
        val_loss  = reg_val_model(
            model, val_loader, loss_function, device=device
        )
        print(f"Epoch [{epoch}/{n_epoch}] | Train loss:{train_loss:.5f} | Val loss:{val_loss:.5f}")
        train_loss_list.append(train_loss)
        val_loss_list.append(val_loss)
        epoch_list.append(epoch)
        # 更新されたモデルを保存
        if val_loss < min_loss:
            min_loss = val_loss
            early_stopping_counter = 0
            torch.save(
                {
                    "epoch": epoch,
                    "model": model.state_dict(),
                    "optimizer": optimizer.state_dict(),
                    "train_loss": train_loss_list,
                    "val_loss": val_loss_list,
                    "epoch_list": epoch_list,
                    "min_loss": min_loss,
                },
                checkpoint_path,
            )
        else:
            early_stopping_counter += 1
            if early_stopping_counter >= patience:
                print("Early stopping")
                break

    return train_loss_list, val_loss_list

""" loss function のプロット """

def plot_loss(
    train_loss_list, val_loss_list, fig_path
):

    plt.figure()
    plt.plot(train_loss_list, c="blue", label="train", linestyle="--")
    plt.plot(val_loss_list, c="red", label="val", linestyle="-")
    plt.legend()
    plt.xlabel("epoch", fontsize=10)
    plt.ylabel("loss", fontsize=10)
    plt.title("Training and validation loss")
    plt.grid()
    plt.savefig(fig_path)



""" メイン関数 """


def main():

    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    checkpoint_path = f"../pth/train_{layer_num}layer.pth"
    loss_fig_path = f"../figures/reg_train_{layer_num}layer.png"
    input_root_path = "../../data/rootfiles/input_nn.root"
    n_epoch = 100

    # データセットの作成
    full_dataset = RegressionDataset(
        input_root_path, tree_name, layer_num, sample_fraction=1.0
    )
    dataset_size = len(full_dataset)
    train_size = int(
        0.8 * dataset_size
    )  # データを訓練用とテスト用に8:2で分割し、データセット作成
    val_size = dataset_size - train_size
    train_dataset, val_dataset = torch.utils.data.random_split(
        full_dataset, [train_size, val_size]
    )
    num_workers = 8
    train_loader = DataLoader(
        train_dataset, batch_size=256, shuffle=True, num_workers=num_workers
    )
    val_loader = DataLoader(
        val_dataset, batch_size=256, shuffle=True, num_workers=num_workers
    )

    # モデルの初期化
    params = load_params("regressor_params.json")
    regressor_hidden_sizes = [
        params[f"regressor_hidden_size_{i}"]
        for i in range(params["regressor_hidden_layers"])
    ]
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = Regressor(
        input_size=layer_num,
        hidden_sizes=regressor_hidden_sizes
    ).to(device)
    loss_function = nn.MSELoss()
    optimizer = optim.SGD(model.parameters(), lr=0.01)

    # GPUの設定
    if torch.cuda.is_available():
        n_gpu = torch.cuda.device_count()
        print("number of GPU available: ", n_gpu)  # gpuの使用可能数を取得
        print("current gpu: ", torch.cuda.get_device_name(torch.cuda.current_device()))
        if n_gpu > 1:
            model = nn.DataParallel(model)

    # 学習
    train_loss_list, val_loss_list = learning(
        checkpoint_path,
        model,
        train_loader,
        val_loader,
        loss_function,
        optimizer,
        n_epoch,
        device=device,
    )

    plot_loss(
        train_loss_list, val_loss_list, loss_fig_path
    )

if __name__ == "__main__":
    main()
