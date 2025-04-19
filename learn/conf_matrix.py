# ============== INFO ==================
#
#  confusion matrixを作成
#  input数に対して規格化

#   last update: 2024/07/11
# ==========================================

import uproot3
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import confusion_matrix, accuracy_score
from sklearn.utils.multiclass import unique_labels
from sklearn.metrics import accuracy_score
import sys

if len(sys.argv) < 2:
        print("Please provide the layer number as an argument (e.g., 'python3 confmatrix.py 10').")
        sys.exit(1)
num_layer = int(sys.argv[1])

# ROOTファイルからデータを読み込む
file = uproot3.open(f"/home/had/kohki/work/ML/2024/geant/rootfiles/output.root")
tree = file[f"tree_{num_layer}layer"]
# ブランチからデータを取得
particle_data = tree.array("pid")
particle_ml_data = tree.array("pid_ML")

class_labels = {0: r"$P$", 1: r"$\pi$", 2: r"$K$"}

# 混同行列を計算
conf_matrix = confusion_matrix(particle_data, particle_ml_data)

# 規格化された混同行列を計算
norm_conf_matrix = conf_matrix.astype(
    'float') / conf_matrix.sum(axis=1)[:, np.newaxis]

# ヒートマップを作成
sns.heatmap(
    norm_conf_matrix,
    annot=True,
    fmt=".2f",
    cmap="Blues",
    xticklabels=[class_labels[i] for i in unique_labels(particle_ml_data)],
    yticklabels=[class_labels[i] for i in unique_labels(particle_data)],
    vmin=0.0,
    vmax=1.0,
    annot_kws={"size": 14},  # フォントサイズを指定
)

plt.title("Confusion Matrix", fontsize=18)
plt.xlabel("output", fontsize=14)
plt.ylabel("input", fontsize=14)
# plt.show()

# 各クラスの識別率（efficiency）の計算
class_accuracies = conf_matrix.diagonal() / conf_matrix.sum(axis=1)
for i, accuracy in enumerate(class_accuracies):
    print(f"Class '{class_labels[i]}': Efficiency = {accuracy * 100:.2f}%")

# 正答率を計算
accuracy = accuracy_score(particle_data, particle_ml_data)
print(f"Accuracy: {accuracy:.4f}")
plt.savefig(f"../figures/conf_matrix_{num_layer}.png")