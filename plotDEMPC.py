import re
import pandas as pd
import matplotlib.pyplot as plt

# Dicionário mapeando os arquivos para seus respectivos tamanhos de população
files = {
    'results0-t4.txt': 80,
    'results0-t1.txt': 100,
    'results0-t5.txt': 120,
    'results0-t2.txt': 150,
    'results0-t3.txt': 200,
}

# Ordenar do menor para o maior tamanho de população para manter a legenda organizada
sorted_files = sorted(files.items(), key=lambda x: x[1])

plt.figure(figsize=(12, 6))

for filename, pop_size in sorted_files:
    days = []
    pnls = []
    
    # Ler o arquivo txt linha por linha buscando o Dia e o PnL
    with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            match = re.search(r'\[Dia (\d+)\]\s+CDI=.*?\|\s+PnL=([-\d\.]+)%', line)
            if match:
                days.append(int(match.group(1)))
                pnls.append(float(match.group(2)))
                
    df = pd.DataFrame({'Dia': days, 'PnL': pnls})
    
    # Cálculo do retorno acumulado composto (%)
    df['Retorno_Acumulado'] = ((1 + df['PnL'] / 100).cumprod() - 1) * 100
    
    # Plotar a linha correspondente ao tamanho da população
    plt.plot(df['Dia'], df['Retorno_Acumulado'], label=f'População: {pop_size}', linewidth=1.5)

# Configurações estéticas do gráfico
plt.title('Comparação de Retorno Acumulado por Tamanho da População (Evolução Diferencial)', fontsize=14, fontweight='bold', pad=15)
plt.xlabel('Dias de Simulação', fontsize=12)
plt.ylabel('Retorno Acumulado (%)', fontsize=12)
plt.legend(title='Tamanho da População', fontsize=10, title_fontsize=11)
plt.grid(True, linestyle='--', alpha=0.6)
plt.tight_layout()

# Salvar e exibir o gráfico
plt.savefig('comparacao_tamanho_populacao.png', dpi=300)
plt.show()