# ==========================================
#   input: particle, mom, tof, ene
#   output: particle_ML
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
from include.dataset import CustomRootDataset
from include.models import FullModel
from include.utils import train_model, val_model, plot_figures, load_params


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
    early_stopping_counter = 0
    patience = 20  # 検証損失が改善しないエポック数の上限

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
            "Epoch [{}/{}] | Train [loss:{:.5f}, acc:{:.5f}] | Val [loss:{:.5f}, acc:{:.5f}]".format(
                epoch + 1, n_epoch, train_loss, train_accuracy, val_loss, val_accuracy
            )
        )
        train_loss_list.append(train_loss)
        val_loss_list.append(val_loss)
        train_accuracy_list.append(train_accuracy)
        val_accuracy_list.append(val_accuracy)
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
                    "train_accuracy": train_accuracy_list,
                    "val_accuracy": val_accuracy_list,
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

    return train_loss_list, val_loss_list, train_accuracy_list, val_accuracy_list


""" メイン関数 """


def main():

    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    checkpoint_path = f"../pth/train_{layer_num}layer.pth"
    fig_path = f"../fig/train_{layer_num}layer.png"
    input_root_path = "../../geant/rootfiles/input_nn.root"
    n_epoch = 100

    # データセットの作成
    full_dataset = CustomRootDataset(
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
    params = load_params("tuned_params.json")
    lstm_hidden_size = params["lstm_hidden_size"]
    # lstm_num_layers = params["lstm_num_layers"]
    classifier_hidden_sizes = [
        params[f"classifier_hidden_size_{i}"]
        for i in range(params["classifier_hidden_layers"])
    ]
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = FullModel(
        lstm_hidden_size=lstm_hidden_size,
        # lstm_num_layers=lstm_num_layers,
        classifier_hidden_sizes=classifier_hidden_sizes,
    ).to(device)
    loss_function = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(), lr=0.01)

    # GPUの設定
    if torch.cuda.is_available():
        n_gpu = torch.cuda.device_count()
        print("number of GPU available: ", n_gpu)  # gpuの使用可能数を取得
        print("current gpu: ", torch.cuda.get_device_name(torch.cuda.current_device()))
        if n_gpu > 1:
            model = nn.DataParallel(model)

    # 学習
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


if __name__ == "__main__":
    main()
