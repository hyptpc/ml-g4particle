import torch
from torch import nn
import torch.nn.functional as F


class LSTMencoder(nn.Module):
    """
    LSTM Encoder
    引数：
    - hidden_size: LSTMの隠れ層のサイズ
    - input_size: 各LSTMモジュールへの入力サイズ
    - num_layers: LSTMの層の数
    - output_size: LSTMの出力のサイズ
    - batch_first: Trueの場合、入力の形状は(batch, seq_len, input_size)
    """
    def __init__(self, input_size, hidden_size, num_layers=1, output_size=1):
        super().__init__()
        self.lstm = nn.LSTM(input_size, hidden_size, num_layers, batch_first=True)
        self.fc = nn.Linear(hidden_size, output_size)

    def forward(self, x):
        x, _ = self.lstm(x)
        x = x[:, -1, :]
        return self.fc(x)


class Classifier(nn.Module):
    """
    Classifier
    引数：
    - input_size: 入力のサイズ
    - hidden_sizes: 隠れ層のサイズのリスト
    - output_size: 出力のサイズ
    """
    def __init__(self, input_size, hidden_sizes, output_size=3):
        super().__init__()
        self.hidden_layers = nn.ModuleList()
        self.hidden_layers.append(nn.Linear(input_size, hidden_sizes[0]))
        for i in range(1, len(hidden_sizes)):
            self.hidden_layers.append(nn.Linear(hidden_sizes[i - 1], hidden_sizes[i]))
        self.output_layer = nn.Linear(hidden_sizes[-1], output_size)

    def forward(self, x):
        for layer in self.hidden_layers:
            x = F.relu(layer(x))
        return self.output_layer(x)


class FullModel(nn.Module):
    """
    Full Model
    引数：
    - lstm_hidden_size: 最適化されたLSTMの隠れ層のサイズ
    - lstm_num_layers: 最適化されたLSTMの層の数
    - classifier_hidden_sizes: 最適化されたClassifierの隠れ層のサイズのリスト
    """
    def __init__(self, lstm_hidden_size, lstm_num_layers, classifier_hidden_sizes):
        super().__init__()
        self.encoder = LSTMencoder(
            input_size=1, hidden_size=lstm_hidden_size, num_layers=lstm_num_layers, output_size=1
        )
        self.classifier = Classifier(
            input_size=3, hidden_sizes=classifier_hidden_sizes, output_size=3
        )

    def forward(self, mom, tof, energy_layers):
        latent = self.encoder(energy_layers)  # 多chのde/dxを1次元の情報に変換
        x = torch.cat(
            (mom.unsqueeze(1), tof.unsqueeze(1), latent), dim=1
        )  # mom, tof, latentを結合
        return self.classifier(x)

    def forward_with_latent(self, mom, tof, energy_layers):
        """
        latentを出力して確認ためのforward
        """
        latent = self.encoder(energy_layers)
        x = torch.cat((mom.unsqueeze(1), tof.unsqueeze(1), latent), dim=1)
        return latent, self.classifier(x)
