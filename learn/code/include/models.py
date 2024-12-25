import torch
from torch import nn
import torch.nn.functional as F


class Regressor(nn.Module):
    """dE/dxからβを推定する回帰モデル"""
    def __init__(self, input_size, hidden_sizes, output_size=1):
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


class Classifier(nn.Module):
    """β, ToF, momをもとにPIDを行う分類モデル"""
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
    """RegressorとClassifierを統合した全体モデル"""
    def __init__(self, layer_num, encoder_hidden_sizes, classifier_hidden_sizes):
        super().__init__()
        self.regressor = Regressor(
            input_size=layer_num, hidden_sizes=encoder_hidden_sizes, output_size=1
        )
        self.classifier = Classifier(
            input_size=3, hidden_sizes=classifier_hidden_sizes, output_size=3
        )

    def forward(self, mom, tof, energy_layers):
        beta = self.encoder(energy_layers)  # 多chのde/dxをβに変換
        x = torch.cat(
            (mom.unsqueeze(1), tof.unsqueeze(1), beta), dim=1
        )  # mom, tof, βを結合
        return self.classifier(x)
