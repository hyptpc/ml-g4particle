import uproot3
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# ROOTファイルのパスを指定
root_file_path = "/ghi/fs02/had/sks/Users/amemiya/rootfiles/g4ml/2024/input_test.root"

# ROOTファイルを開く
with uproot3.open(root_file_path) as file:
    # tree_32layerのツリーを取得
    tree = file["tree_32layer"]
    
    # 必要なデータを取得
    mom = tree.array("mom")  # 運動量
    tof = tree.array("tof")  # 飛行時間
    ene = tree.array("ene")  # エネルギー
    pid = tree.array("pid")  # 粒子ID
    
    # pidに基づく色付け
pid_colors = {
    0: 'red',    # Particle ID 0 -> Red
    1: 'blue',     # Particle ID 1 -> Blue
    2: 'green',   # Particle ID 2 -> Green
}

# pidに基づく色のリストを作成
colors = [pid_colors.get(p, 'black') for p in pid]


# 3次元プロット
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# 散布図を作成
sc = ax.scatter(mom, tof, ene, c=colors, s=1, alpha=0.7)

# 軸ラベルを設定
ax.set_xlabel("Momentum")
ax.set_ylabel("Time of Flight")
ax.set_zlabel("Energy Loss (truncated mean)")

# プロットを表示
# plt.ion()
# plt.show()
plt.savefig("3d_mapping.png")