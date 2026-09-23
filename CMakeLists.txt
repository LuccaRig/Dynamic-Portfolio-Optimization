cmake_minimum_required(VERSION 3.12)
project(PortifolioMPC VERSION 1.0 LANGUAGES CXX)

# Define o padrão C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontra e configura o suporte ao OpenMP
find_package(OpenMP REQUIRED)

# Inclui os diretórios de cabeçalhos
include_directories(include)

# Arquivos fonte do projeto
set(SOURCES
    src/main.cpp
    src/Mercado.cpp
    src/SimuladorPortfolio.cpp
    src/DifferentialEvolution.cpp
)

# Cria o executável
add_executable(PortifolioMPC ${SOURCES})

# Vincula o OpenMP ao executável compilado
target_link_libraries(PortifolioMPC PRIVATE OpenMP::OpenMP_CXX)