# test_net.py
# created by K.Amemiya (2024/11/15)

import torch.multiprocessing

torch.multiprocessing.set_sharing_strategy("file_system")
import sys
import os
import torch
import uproot3
import numpy as np
import pandas as pd
from torch.utils.data import DataLoader
from include.dataset import ClassificationDataset
from include.models import FullModel
from include.utils import load_params


def save_to_files(
    model,
    test_loader,
    checkpoint_path,
    output_root_path,
    csv_path,
    tree_name,
    layer_num,
    device="cpu",
):
    checkpoint = torch.load(checkpoint_path)
    model.load_state_dict(checkpoint["model"])

    # Initialize
    write_mom = []
    write_tof = []
    write_ene = [[] for _ in range(layer_num)]
    write_particle = []
    write_particle_ML = []
    num_test = 0
    correct_predictions = 0

    # 推論
    model.eval()
    with torch.no_grad():
        for i, batch in enumerate(test_loader):
            num_test += len(batch["target"])
            mom = batch["input"]["mom"].to(device)
            tof = batch["input"]["tof"].to(device)
            energy_layers = batch["input"]["energy_layers"].to(device)
            labels = batch["target"].to(device)
            particle_ = labels.cpu().numpy()
            outputs = model(mom, tof, energy_layers)
            particle_ML = torch.argmax(outputs, dim=1).cpu().numpy()
            correct_predictions += np.sum(particle_ == particle_ML)

            for j in range(len(labels)):
                write_mom.append(mom[j].cpu().numpy())
                write_tof.append(tof[j].cpu().numpy())
                for layer in range(layer_num):
                    ene = energy_layers[j, layer]
                    write_ene[layer].append(ene.cpu().numpy())
                particle = particle_[j]
                write_particle.append(particle)
                write_particle_ML.append(particle_ML[j])

    accuracy = correct_predictions / num_test
    error = np.sqrt(accuracy * (1 - accuracy) / num_test)

    print(f"Accuracy: {accuracy:.4f}, Error: {error:.9f}")

    # csvの作成
    df = pd.DataFrame(columns=["layers", "acc", "err"])
    df = pd.concat(
        [
            df,
            pd.DataFrame(
                {
                    "layers": [layer_num],
                    "acc": [round(accuracy, 4)],
                    "err": [round(error, 9)],
                }
            ),
        ]
    )
    # 既存のCSVファイルが存在する場合は読み込む
    if os.path.exists(csv_path):
        existing_df = pd.read_csv(csv_path)
        df = pd.concat([existing_df, df])
    df = df.sort_values(by="layers").reset_index(drop=True)  # layersを昇順にソート
    df.to_csv(csv_path, index=False)  # CSVファイルに保存
    print(f"Results saved to {csv_path}")

    # Save the results to a ROOT file
    branches = {
        "pid": np.int32,
        "pid_ML": np.int32,
        "tof": np.float32,
        "mom": np.float32
    }
    for layer in range(layer_num):
        branches[f"ene_layer{layer}"] = np.float32
    file = uproot3.recreate(output_root_path)
    file[tree_name] = uproot3.newtree(branches)
    extend_data = {
        "pid": write_particle,
        "pid_ML": write_particle_ML,
        "tof": write_tof,
        "mom": write_mom
    }
    for layer in range(layer_num):
        extend_data[f"ene_layer{layer}"] = write_ene[layer]
    file[tree_name].extend(extend_data)
    print(f"Results saved to {output_root_path}")


def main():

    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    test_root_path = "../../geant/data/input_test.root"
    output_root_path = f"../../geant/data/output_{layer_num}layer.root"
    checkpoint_path = f"../pth/class/class_{layer_num}layer.pth"
    csv_path = "../../likelihood/csv/accuracy.csv"
    sample_fraction = 1  # Use all data

    print("Loading data ...")
    test_dataset = ClassificationDataset(
        test_root_path, tree_name, layer_num, sample_fraction
    )
    test_loader = DataLoader(test_dataset, batch_size=256, shuffle=False, num_workers=8)

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
    if torch.cuda.is_available() and torch.cuda.device_count() > 1:
        model = torch.nn.DataParallel(model)

    # save data to rootfile and csv
    save_to_files(
        model,
        test_loader,
        checkpoint_path,
        output_root_path,
        csv_path,
        tree_name,
        layer_num,
        device=device,
    )


if __name__ == "__main__":
    main()
