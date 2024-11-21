import torch
import matplotlib.pyplot as plt
import json

""" training function """


def train_model(model, train_loader, loss_function, optimizer, device="cpu"):
    train_loss = 0.0
    num_train = 0
    train_accuracy = 0.0
    model.train()
    for batch in train_loader:
        num_train += len(batch["target"])
        mom = batch["input"]["mom"].to(device)
        tof = batch["input"]["tof"].to(device)
        energy_layers = batch["input"]["energy_layers"].to(device)
        labels = batch["target"].to(device)
        optimizer.zero_grad()
        outputs = model(mom, tof, energy_layers)
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
            mom = batch["input"]["mom"].to(device)
            tof = batch["input"]["tof"].to(device)
            energy_layers = batch["input"]["energy_layers"].to(device)
            labels = batch["target"].to(device)
            outputs = model(mom, tof, energy_layers)
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
    plt.plot(val_accuracy_list, c="red", label="val", linestyle="-")
    plt.legend()
    plt.xlabel("epoch", fontsize=10)
    plt.ylabel("accuracy", fontsize=10)
    plt.title("Training and validation accuracy")
    plt.grid()

    plt.subplot(1, 2, 2)
    plt.plot(train_loss_list, c="blue", label="train", linestyle="--")
    plt.plot(val_loss_list, c="red", label="val", linestyle="-")
    plt.legend()
    plt.xlabel("epoch", fontsize=10)
    plt.ylabel("loss", fontsize=10)
    plt.title("Training and validation loss")
    plt.grid()

    plt.savefig(fig_path)


""" パラメータの読み込みとモデルの初期化 """


def load_params(path):
    with open(path, "r") as f:
        return json.load(f)
