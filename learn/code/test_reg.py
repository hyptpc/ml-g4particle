import torch
import numpy as np
import matplotlib.pyplot as plt
import sys
from torch.utils.data import DataLoader
from include.dataset import RegressionDataset
from include.models import Regressor
from include.utils import load_params

def test_reg_model(model_path, test_data_path, fig_path, layer_num, batch_size=256):
    # テストデータの作成
    tree_name = f"tree_{layer_num}layer"
    test_dataset = RegressionDataset(test_data_path, tree_name, layer_num)
    test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

    # モデルのロード
    params = load_params("regressor_params.json")
    regressor_hidden_sizes = [
        params[f"regressor_hidden_size_{i}"]
        for i in range(params["regressor_hidden_layers"])
    ]
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = Regressor(input_size=layer_num, hidden_sizes=regressor_hidden_sizes).to(device)

    # 学習済みモデルの重みをロード
    checkpoint = torch.load(model_path, map_location=device)
    model.load_state_dict(checkpoint["model"])
    model.eval()

    # テストデータで予測
    actual_beta = []
    predicted_beta = []
    with torch.no_grad():
        for batch in test_loader:
            inputs = batch["input"].to(device)
            targets = batch["target"].to(device)
            predictions = model(inputs).squeeze()
            actual_beta.extend(targets.cpu().numpy())
            predicted_beta.extend(predictions.cpu().numpy())

    # MSEの計算
    actual_beta = np.array(actual_beta)
    predicted_beta = np.array(predicted_beta)
    mse = np.mean((actual_beta - predicted_beta) ** 2)

    print(f"Mean Squared Error (MSE) on test data: {mse:.4f}")

    # 実測値 vs 予測値のプロット
    plt.figure(figsize=(8, 8))
    plt.scatter(actual_beta, predicted_beta, alpha=0.5)
    plt.xlabel("True")
    plt.ylabel("Prediction")
    plt.title(f"Prediction accuracy (MSE: {mse:.4f})")
    plt.grid(True)
    plt.savefig(fig_path)
    plt.show()


if __name__ == "__main__":
    # ファイルパスとパラメータ
    layer_num = int(sys.argv[1])
    tree_name = f"tree_{layer_num}layer"
    test_data_path = "../../geant/data/input_test.root"
    model_path = f"../pth/reg_{layer_num}layer.pth"  # 学習済みモデルの保存先
    fig_path = f"../figures/reg_test_{layer_num}layer.png"  # プロット保存先

    # 評価の実行
    test_reg_model(model_path, test_data_path, fig_path, layer_num)