# test_net.py
import torch.multiprocessing
torch.multiprocessing.set_sharing_strategy('file_system')
import sys
import torch
import uproot3
import numpy as np
from torch.utils.data import DataLoader
from train_net import (
    ExampleNN,
    CustomRootDataset,
    input_size,
    hidden1_size,
    hidden2_size,
    output_size,
    num_workers,
)

def fill_rootfile(model, test_loader, checkpoint_path, output_root_path, tree_name, device="cpu"):
    checkpoint = torch.load(checkpoint_path)
    model.load_state_dict(checkpoint["model"])

    # Initialize
    write_mom = []
    write_tof = []
    write_ene = []
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
            ene = inputs[j, 2]
            write_ene.append(ene.cpu().numpy())
            particle = particle_[j]
            write_particle.append(particle)
            write_particle_ML.append(particle_ML[j])

    file = uproot3.recreate(output_root_path)
    file[tree_name] = uproot3.newtree(
        {
            "pid": np.int32,
            "pid_ML": np.int32,
            "tof": np.float32,
            "mom": np.float32,
            "ene": np.float32,
        }
    )
    file[tree_name].extend(
        {
            "pid": write_particle,
            "pid_ML": write_particle_ML,
            "tof": write_tof,
            "mom": write_mom,
            "ene": write_ene,
        }
    )

    accuracy = correct_predictions / num_test
    print(f"Accuracy: {accuracy:.4f}")


# Main function
def main():
    if len(sys.argv) < 2:
        print("Please provide the layer number as an argument (e.g., 'python3 evaluate_net.py 10').")
        sys.exit(1)
    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    test_root_path = "/home/had/kohki/work/ML/2024/geant/rootfiles/input_test.root"
    output_root_path = f"/home/had/kohki/work/ML/2024/geant/rootfiles/output_{layer_num}layer.root"
    checkpoint_path = f"/home/had/kohki/work/ML/2024/learn/pth/train_{layer_num}layer.pth"
    sample_fraction = 1  # Use all data

    # Load the data
    print("Loading data ...")
    test_dataset = CustomRootDataset(test_root_path, tree_name, sample_fraction)
    test_loader = DataLoader(
        test_dataset, batch_size=256, shuffle=False, num_workers=num_workers)

    # Initialize the model
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = ExampleNN(input_size, hidden1_size, hidden2_size, output_size).to(device)
    if torch.cuda.is_available() and torch.cuda.device_count() > 1:
        model = torch.nn.DataParallel(model)

    fill_rootfile(model, test_loader, checkpoint_path, output_root_path, tree_name, device=device)
    print(f"Results saved to {output_root_path}")


if __name__ == "__main__":
    main()