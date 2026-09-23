#include "Dados.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

std::vector<double> carregarCSV1D(const std::string& nome_arquivo)
{
    std::vector<double> dados;
    std::ifstream arquivo(nome_arquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir arquivo: "
                  << nome_arquivo << std::endl;
        return dados;
    }

    std::string linha;

    while (std::getline(arquivo, linha)) {
        if (linha.empty())
            continue;

        try {
            dados.push_back(std::stod(linha));
        }
        catch (...) {
            // Ignora linhas inválidas/cabeçalhos.
        }
    }

    return dados;
}

std::vector<std::vector<double>> carregarCSV2D(
    const std::string& nome_arquivo)
{
    std::vector<std::vector<double>> dados;
    std::ifstream arquivo(nome_arquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir arquivo: "
                  << nome_arquivo << std::endl;
        return dados;
    }

    std::string linha;

    while (std::getline(arquivo, linha)) {
        if (linha.empty())
            continue;

        std::stringstream ss(linha);
        std::string valor;
        std::vector<double> linha_dados;

        while (std::getline(ss, valor, ',')) {
            try {
                linha_dados.push_back(std::stod(valor));
            }
            catch (...) {
                // Ignora valores inválidos.
            }
        }

        if (!linha_dados.empty())
            dados.push_back(linha_dados);
    }

    return dados;
}
