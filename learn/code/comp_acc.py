import pandas as pd
import matplotlib.pyplot as plt

# CSVファイルを読み込む
df = pd.read_csv('../csv/accuracy.csv')

# フォントとスタイルの設定
plt.rcParams['font.family'] = 'Times New Roman'
plt.rcParams['mathtext.fontset'] = 'stix'
plt.rcParams["font.size"] = 15
plt.rcParams['axes.grid'] = True
plt.rcParams["legend.fancybox"] = False
plt.rcParams["legend.framealpha"] = 1
plt.rcParams["legend.edgecolor"] = 'black'
plt.rcParams["legend.handlelength"] = 1
plt.rcParams["legend.labelspacing"] = 0.5
plt.rcParams["legend.handletextpad"] = 0.5
plt.rcParams["legend.markerscale"] = 2

# グラフを作成する
plt.figure(figsize=(10, 6))

# ML accuracyをプロットする（誤差バー付き）
plt.errorbar(df['layers'], df['ML_acc'], yerr=df['ML_err'], label='Machine Learning', fmt='o', capsize=5, linestyle='-', markerfacecolor='white', markeredgewidth=1)

# Likelihood accuracyをプロットする（誤差バー付き）
plt.errorbar(df['layers'], df['Likelihood_acc'], yerr=df['Likelihood_err'], label='Likelihood', fmt='^', capsize=5, linestyle='--', markerfacecolor='white', markeredgewidth=1)

# グラフのタイトルとラベルを設定する
plt.title('Accuracy Transition', fontsize=20, fontweight='bold')
plt.xlabel('Number of Layers', fontsize=16)
plt.ylabel('Accuracy', fontsize=16)

plt.ylim(0.9, 1.00)

# 軸の文字サイズを設定し、小数点2桁で表示する
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)

# 凡例を右下に表示する
plt.legend(loc='lower right', fontsize=14)

# グリッドを表示する
plt.grid(True)

# グラフを保存する
plt.savefig('../figures/comp_acc.png', bbox_inches="tight", pad_inches=0.05)

# # グラフを表示する
# plt.show()
