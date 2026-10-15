#!/bin/bash

# Configurações de diretórios e arquivos
EXECUTAVEL_NOME="PortifolioMPC"
ARQUIVO_SAIDA="results0-t5.txt"
ARQUIVO_TEMPOS="tempos_execucao.csv"

echo "=== 1. Compilando o projeto (CMake) ==="
cmake --build .

# Localiza o executável gerado
if [ -f "./$EXECUTAVEL_NOME" ]; then
    EXECUTAVEL="./$EXECUTAVEL_NOME"
elif [ -f "build/$EXECUTAVEL_NOME" ]; then
    EXECUTAVEL="build/$EXECUTAVEL_NOME"
else
    echo "Erro: O executável '$EXECUTAVEL_NOME' não foi encontrado após a compilação."
    exit 1
fi

echo "=== 2. Iniciando execução do sistema MPC ==="
echo "Salvando saída em: $ARQUIVO_SAIDA"

# Marca o tempo inicial em segundos com alta precisão
inicio=$(date +%s.%N)

# Executa o programa redirecionando a saída para o arquivo de texto desejado
$EXECUTAVEL > "$ARQUIVO_SAIDA"

# Marca o tempo final
fim=$(date +%s.%N)

# Calcula a diferença do tempo decorrido
tempo_decorrido=$(echo "$fim - $inicio" | bc)

echo "=== 3. Execução concluída ==="
echo "Tempo de execução: ${tempo_decorrido} segundos"

# Registra o tempo em um CSV para futuros gráficos de desempenho computacional
if [ ! -f "$ARQUIVO_TEMPOS" ]; then
    echo "experimento,tempo_segundos" > "$ARQUIVO_TEMPOS"
fi

# Extrai o identificador do arquivo de saída (ex: t1 de results0-t1.txt)
id_experimento=$(echo "$ARQUIVO_SAIDA" | sed -E 's/results0-(.*)\.txt/\1/')
echo "$id_experimento,$tempo_decorrido" >> "$ARQUIVO_TEMPOS"

echo "Tempo registrado com sucesso em $ARQUIVO_TEMPOS"