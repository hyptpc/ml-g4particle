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
def load_rootfile(root_file, tree_name):
    with uproot.open(root_file) as file:
        tree = file[tree_name]
        data = tree.arrays(["mom", "pid", "latent"], library="np")
        return data


# βを計算する関数
def calculate_beta(mom, pid):
    mass = get_mass(pid)
    beta = mom / np.sqrt(mass**2 + mom**2)
    return beta


# メイン処理
def main():
    root_file = "../../geant/rootfiles/output_1.root"
    tree_name = "tree_32layer"

    # データの読み込み
    data = load_rootfile(root_file, tree_name)
    mom = data["mom"]
    pid = data["pid"]
    latent = data["latent"]

    # βを計算
    betas = []
    latents = []
    for p, pid_val, latent_val in zip(mom, pid, latent):
        beta = calculate_beta(p, pid_val)
        betas.append(beta)
        latents.append(latent_val)

    # グラフのプロット
    plt.figure(figsize=(10, 6))
    plt.scatter(betas, latents, alpha=0.6, edgecolor="k")
    plt.xlabel("initial Beta", fontsize=14)
    plt.ylabel("Encoder output", fontsize=14)
    plt.grid(True)
    plt.show()
    plt.savefig("../figures/latent.png")


if __name__ == "__main__":
    main()
