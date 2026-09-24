#pragma once

#include <string>
#include <vector>

std::vector<double> carregarCSV1D(const std::string& nome_arquivo);

std::vector<std::vector<double>> carregarCSV2D(const std::string& nome_arquivo);

std::vector<std::string> carregarCSVTickers(const std::string& nome_arquivo);