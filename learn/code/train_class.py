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
from include.dataset import ClassificationDataset
from include.models import FullModel
from include.utils import class_train_model, class_val_model, plot_figures, load_params


""" leaning function """


def learning(
    reg_pth,
    class_pth,
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
    
    if os.path.exists(reg_pth):
        reg_ckpt = torch.load(reg_pth, map_location=device)
        model.regressor.load_state_dict(reg_ckpt["model"])
        print(f"Regressor weights loaded from {reg_pth}")
    else:
        print(f"Warning: {reg_pth} not found")
        sys.exit(1)
        
    # Regressorの重みを固定（フリーズ）
    for param in model.regressor.parameters():
        param.requires_grad = False

    if os.path.exists(class_pth):
        class_ckpt = torch.load(class_pth)
        model.load_state_dict(class_ckpt["model"])
        optimizer.load_state_dict(class_ckpt["optimizer"])
        start_epoch = class_ckpt["epoch"] + 1  # 学習を前回のエポックから再開
        train_loss_list = class_ckpt["train_loss"]
        val_loss_list = class_ckpt["val_loss"]
        train_accuracy_list = class_ckpt["train_accuracy"]
        val_accuracy_list = class_ckpt["val_accuracy"]
        epoch_list = class_ckpt["epoch_list"]
        min_loss = class_ckpt["min_loss"]
        print(f"Previous checkpoint loaded. Resuming training from epoch {start_epoch}")
    else:
        start_epoch = 1
        min_loss = float("inf")

    # epoch loop
    for epoch in range(start_epoch, n_epoch + 1, 1):
        train_loss, train_accuracy = class_train_model(
            model, train_loader, loss_function, optimizer, device=device
        )
        val_loss, val_accuracy = class_val_model(
            model, val_loader, loss_function, device=device
        )
        print(
            "Epoch [{}/{}] | Train [loss:{:.5f}, acc:{:.5f}] | Val [loss:{:.5f}, acc:{:.5f}]".format(
                epoch, n_epoch, train_loss, train_accuracy, val_loss, val_accuracy
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
                class_pth,
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
    reg_pth = f"../pth/reg/reg_{layer_num}layer.pth"
    class_pth = f"../pth/class/class_{layer_num}layer.pth"
    fig_path = f"../figures/class_train/class_train_{layer_num}layer.png"
    input_root_path = "../../geant/data/input_nn.root"
    n_epoch = 100

    # データセットの作成
    full_dataset = ClassificationDataset(
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
    params = load_params("params.json")
    regressor_hidden_sizes = [
        params[f"regressor_hidden_size_{i}"]
        for i in range(params["regressor_hidden_layers"])
    ]
    classifier_hidden_sizes = [
        params[f"classifier_hidden_size_{i}"]
        for i in range(params["classifier_hidden_layers"])
    ]
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = FullModel(
        layer_num=layer_num,
        regressor_hidden_sizes=regressor_hidden_sizes,
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
        reg_pth,
        class_pth,
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
