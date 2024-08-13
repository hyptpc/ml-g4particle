import pandas as pd
import matplotlib.pyplot as plt

# CSVファイルを読み込む
df = pd.read_csv('../csv/accuracy.csv')

# グラフを作成する
plt.figure(figsize=(10, 6))

# ML accuracyをプロットする
plt.plot(df['layers'], df['ML_acc'], label='Machine Learning', marker='o')

# Likelihood accuracyをプロットする
plt.plot(df['layers'], df['Likekihood_acc'], label='Likelihood', marker='x')

# グラフのタイトルとラベルを設定する
plt.title('Accuracy transition', fontsize=20)
plt.xlabel('Number of Layers', fontsize=16)
plt.ylabel('Accuracy', fontsize=16)

plt.ylim(0.8, 1.00)

# 軸の文字サイズを設定し、小数点2桁で表示する
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)

# 凡例を右下に表示する
plt.legend(loc='lower right', fontsize=14)

# グリッドを表示する
plt.grid(True)

# グラフを表示する
plt.savefig('../figures/comp_acc.png')
