# ***************************************
#
# hidden layer optimization with Optuna
# Using small dataset for tuning
#
#   other hyperparameters:
#   loss_function: CrossEntropyLoss
#   optimizer: SGD
#   learning_rate: 0.01
#   batch_size: 256
#
# 2024/11/14 K.Amemiya
# ***************************************

import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
import optuna
import os
import json
from include.dataset import CustomRootDataset
from include.models import FullModel
from include.utils import train_model, val_model

n_epoch = 50
input_root_path = "../../geant/rootfiles/input_nn.root"
layer_num = 32  # 簡単のために32layerのテータのみを考慮
tree_name = "tree_32layer"
sample_fraction = 0.01  # 効率化のためにテストデータのうち1%のデータのみを使用

# データセットの読み込み（全データ）
sampled_dt = CustomRootDataset(
    input_root_path, tree_name, layer_num=layer_num, sample_fraction=sample_fraction
)


""" 目的関数 """


def objective(trial):
    # EncoderとClassifierの隠れ層数とサイズを探索
    encoder_hidden_layers = trial.suggest_int("encoder_hidden_layers", 2, 6)
    classifier_hidden_layers = trial.suggest_int("classifier_hidden_layers", 2, 6)

    encoder_hidden_sizes = [
        trial.suggest_categorical(f"encoder_hidden_size_{i}", [128, 256, 512, 1024])
        for i in range(encoder_hidden_layers)
    ]
    classifier_hidden_sizes = [
        trial.suggest_categorical(f"classifier_hidden_size_{i}", [128, 256, 512, 1024])
        for i in range(classifier_hidden_layers)
    ]

    sampled_dt_size = len(sampled_dt)
    train_size = int(0.8 * sampled_dt_size)
    val_size = sampled_dt_size - train_size
    train_dataset, val_dataset = torch.utils.data.random_split(
        sampled_dt, [train_size, val_size]
    )

    train_loader = DataLoader(
        train_dataset, batch_size=256, shuffle=True, num_workers=8
    )
    val_loader = DataLoader(val_dataset, batch_size=256, shuffle=True, num_workers=8)

    # モデルの初期化
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = FullModel(
        layer_num=layer_num,
        encoder_hidden_sizes=encoder_hidden_sizes,
        classifier_hidden_sizes=classifier_hidden_sizes,
    ).to(device)
    loss_function = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(), lr=0.01)

    # 学習と評価
    for epoch in range(1, n_epoch + 1):
        train_loss, train_accuracy = train_model(
            model, train_loader, loss_function, optimizer, device=device
        )
        val_loss, val_accuracy = val_model(
            model, val_loader, loss_function, device=device
        )

        trial.report(val_loss, epoch)
        if trial.should_prune():
            print(f"Trial {trial.number} pruned at epoch {epoch}")
            raise optuna.exceptions.TrialPruned()

    return val_loss


""" 最適パラメータの保存 """


def save_params(study, output_path):
    best_params = study.best_trial.params
    with open(output_path, "w") as f:
        json.dump(best_params, f)


""" メイン """

if __name__ == "__main__":
    study_name = "hidden_layer_tuning"
    storage_name = "sqlite:///optuna.db"
    n_trials = 100  # 総試行回数

    if os.path.exists("optuna.db"):
        study = optuna.load_study(study_name=study_name, storage=storage_name)
        # 残りの試行数を計算
        completed_trials = len(
            [t for t in study.trials if t.state == optuna.trial.TrialState.COMPLETE]
        )
        remaining_trials = n_trials - completed_trials
        print(
            f"Resuming study from trial {completed_trials} with {remaining_trials} trials remaining."
        )
    else:
        study = optuna.create_study(
            study_name=study_name, storage=storage_name, direction="minimize"
        )
        remaining_trials = n_trials  # 初回実行時には全試行を実行

    def print_progress(study, trial):
        print(f"Trial {trial.number} completed with val_loss: {trial.value}")
        print(
            f"Best trial so far: {study.best_trial.number} with val_loss: {study.best_trial.value}"
        )

    # 残りの試行数のみ実行
    if remaining_trials > 0:
        study.optimize(objective, n_trials=remaining_trials, callbacks=[print_progress])

    save_params(study, "tuned_params.json")  # 最適なパラメータを保存
    print("parameters saved to json")

    best_trial = study.best_trial
    print(f"Best trial number: {best_trial.number}")
    print(f"Best trial value (validation loss): {best_trial.value}")
    print("Best hyperparameters:")
    for key, value in best_trial.params.items():
        print(f"  {key}: {value}")
