# test_net.py
# created by K.Amemiya (2024/11/15)

import torch.multiprocessing

torch.multiprocessing.set_sharing_strategy("file_system")
import sys
import torch
import uproot3
import numpy as np
import pandas as pd
from torch.utils.data import DataLoader
from include.dataset import CustomRootDataset
from include.models import FullModel
from include.utils import load_params


def fill_rootfile(
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

    for i, batch in enumerate(test_loader):
        num_test += len(batch["target"])
        inputs, labels = batch["input"].to(device), batch["target"].to(device)
        particle_ = labels.cpu().numpy()
        outputs = model(inputs)
        particle_ML = torch.argmax(outputs, dim=1).cpu().numpy()
        correct_predictions += np.sum(particle_ == particle_ML)
        for j in range(len(labels)):
            mom = inputs[j, 0]
            write_mom.append(mom.cpu().numpy())
            tof = inputs[j, 1]
            write_tof.append(tof.cpu().numpy())
            for layer in range(layer_num):
                ene = inputs[j, 2 + layer]  # Energy layers start from index 2
                write_ene[layer].append(ene.cpu().numpy())
            particle = particle_[j]
            write_particle.append(particle)
            write_particle_ML.append(particle_ML[j])

    accuracy = correct_predictions / num_test
    error = np.sqrt(accuracy * (1 - accuracy) / num_test)

    print(f"Accuracy: {accuracy:.4f}, Error: {error:.9f}")

    # Save the results to a CSV file
    df = pd.read_csv(csv_path)
    df.loc[df["layers"] == layer_num, "ML3_acc"] = round(accuracy, 4)
    df.loc[df["layers"] == layer_num, "ML3_err"] = round(error, 9)
    df.to_csv(csv_path, index=False)
    print(f"Results saved to {csv_path}")

    # Save the results to a ROOT file
    branches = {
        "pid": np.int32,
        "pid_ML": np.int32,
        "tof": np.float32,
        "mom": np.float32,
    }
    for layer in range(layer_num):
        branches[f"ene_layer{layer}"] = np.float32

    file = uproot3.recreate(output_root_path)
    file[tree_name] = uproot3.newtree(branches)

    extend_data = {
        "pid": write_particle,
        "pid_ML": write_particle_ML,
        "tof": write_tof,
        "mom": write_mom,
    }
    for layer in range(layer_num):
        extend_data[f"ene_layer{layer}"] = write_ene[layer]

    file[tree_name].extend(extend_data)


def main():

    if len(sys.argv) < 2:
        print(
            "Please provide the layer number as an argument (e.g., 'python3 test_net.py 10')."
        )
        sys.exit(1)
    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    test_root_path = "../../geant/rootfiles/input_test.root"
    output_root_path = f"../../geant/rootfiles/output_{layer_num}layer.root"
    checkpoint_path = f"../pth/train_{layer_num}layer.pth"
    csv_path = "../csv/accuracy.csv"
    sample_fraction = 1  # Use all data

    print("Loading data ...")
    test_dataset = CustomRootDataset(test_root_path, tree_name, sample_fraction)
    test_loader = DataLoader(test_dataset, batch_size=256, shuffle=False, num_workers=8)

    # モデルの初期化
    params = load_params("tuned_params.json")
    encoder_hidden_sizes = [
        params[f"encoder_hidden_size_{i}"]
        for i in range(params["encoder_hidden_layers"])
    ]
    classifier_hidden_sizes = [
        params[f"classifier_hidden_size_{i}"]
        for i in range(params["classifier_hidden_layers"])
    ]
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = FullModel(
        layer_num=layer_num,
        encoder_hidden_sizes=encoder_hidden_sizes,
        classifier_hidden_sizes=classifier_hidden_sizes,
    ).to(device)
    if torch.cuda.is_available() and torch.cuda.device_count() > 1:
        model = torch.nn.DataParallel(model)

    # save data to rootfile and csv
    fill_rootfile(
        model,
        test_loader,
        checkpoint_path,
        output_root_path,
        csv_path,
        tree_name,
        layer_num,
        device=device,
    )
    print(f"Results saved to {output_root_path}")


if __name__ == "__main__":
    main()
