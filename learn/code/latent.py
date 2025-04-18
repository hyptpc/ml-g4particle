import uproot
import numpy as np
import matplotlib.pyplot as plt


# PDG粒子データベースから質量を取得する関数
def get_mass(pid):
    # GeV単位の質量値
    if pid == 0:
        return 0.938272  # proton mass in GeV
    elif pid == 1:
        return 0.139570  # pion mass in GeV
    elif pid == 2:
        return 0.493677  # kaon mass in GeV


# ROOTファイルを読み込み、データを取得
def load_encoderfile(encoderfile, tree_name):
    with uproot.open(encoderfile) as file:
        tree = file[tree_name]
        data = tree.arrays(["mom", "pid", "latent"], library="np")
        return data

def load_meanfile(meanfile, tree_name):
    with uproot.open(meanfile) as file:
        tree = file[tree_name]
        data = tree.arrays(["mom", "pid", "ene"], library="np")
        return data


# βを計算する関数
def calculate_beta(mom, pid):
    mass = get_mass(pid)
    beta = mom / np.sqrt(mass**2 + mom**2)
    return beta


# メイン処理
def main():
    file1 = "../../geant/rootfiles/output.root" # latent
    file2 = "/home/had/kohki/work/ML/2024/geant/rootfiles/input_test.root" # mean
    tree_name = "tree_32layer"
    
    # データの読み込み
    data1 = load_encoderfile(file1, tree_name)
    mom1= data1["mom"]
    pid1 = data1["pid"]
    latent = data1["latent"]

    data2 = load_meanfile(file2, tree_name)
    mom2= data2["mom"]
    pid2 = data2["pid"]
    mean = data2["ene"]

    # βを計算
    betas1 = []
    latents = []
    for p, pid_val, latent_val in zip(mom1, pid1, latent):
        beta1 = calculate_beta(p, pid_val)
        betas1.append(beta1)
        latents.append(latent_val)

    betas2 = []
    means = []
    for p, pid_val, mean_val in zip(mom2, pid2, mean):
        beta2 = calculate_beta(p, pid_val)
        betas2.append(beta2)
        means.append(mean_val)


    # グラフのプロット
    plt.figure(figsize=(10, 6))
    plt.scatter(betas1, latents, marker=".", alpha=0.6, c= "blue", label="LSTM Encoder output")
    plt.scatter(betas2, means, marker=".", alpha=0.6, c= "red", label="Mean output")
    plt.xlabel("initial Beta", fontsize=14)
    plt.grid(True)
    # plt.legend(loc="upper right", fontsize=14)
    plt.savefig("../fig/latent.png")
    
    
    proton_betas = [beta for beta, pid in zip(betas1, pid1) if pid == 0]  # proton
    pion_betas = [beta for beta, pid in zip(betas1, pid1) if pid == 1]    # pion
    kaon_betas = [beta for beta, pid in zip(betas1, pid1) if pid == 2]    # kaon
    plt.figure(figsize=(10, 6))
    plt.hist(proton_betas, bins=100, alpha=0.7, label="proton", color="red")
    plt.hist(pion_betas, bins=100, alpha=0.7, label="pion", color="blue")
    plt.hist(kaon_betas, bins=100, alpha=0.7, label="kaon", color="green")
    plt.title("Truncated Mean", fontsize=16)
    plt.xlabel("Beta", fontsize=14)
    plt.ylabel("Count", fontsize=14)
    plt.legend(loc="upper left", fontsize=12)
    plt.grid(True)
    plt.savefig("../fig/beta.png")


if __name__ == "__main__":
    main()