import pandas as pd
import yfinance as yf
from bcb import sgs

# 1. Misturando Ações da B3 e BDRs globais negociados na B3
tickers_acoes = ["PETR4.SA", "VALE3.SA", "ITUB3.SA", "WEGE3.SA", "BBAS3.SA", "CMIG3.SA"]
tickers_bdrs  = ["AAPL34.SA", "MSFT34.SA", "NVDC34.SA", "GOGL34.SA", "TSMC34.SA", "COCA34.SA"] 
todos_os_ativos = tickers_acoes + tickers_bdrs

print("Baixando dados de ações e BDRs do Yahoo Finance...")
dados_mercado = yf.download(todos_os_ativos, period="3y", interval="1d")['Close']
dados_mercado = dados_mercado.dropna()

# 2. Baixar a Taxa CDI Diária do Banco Central do Brasil
print("Baixando histórico da Taxa CDI (Série 12) do Banco Central...")
data_inicio = dados_mercado.index[0].strftime('%Y-%m-%d')
data_fim = dados_mercado.index[-1].strftime('%Y-%m-%d')
cdi_diario = sgs.get({'CDI': 12}, start=data_inicio, end=data_fim) / 100.0

# 3. Sincronizar as datas
tabela_completa = dados_mercado.join(cdi_diario, how='inner').dropna()

retornos_ativos = tabela_completa[todos_os_ativos].pct_change().dropna()
taxa_cdi = tabela_completa['CDI'].loc[retornos_ativos.index]

# 4. Salvar em arquivos separados para o C++ ler
retornos_ativos.to_csv("historico_retornos.csv", index=False, header=False)
taxa_cdi.to_csv("historico_cdi.csv", index=False, header=False)

# Dica: salve também a lista dos nomes dos ativos para você saber quem é quem no C++
pd.Series(todos_os_ativos).to_csv("ativos_nomes.csv", index=False, header=False)

print("Dados de Ações, BDRs e CDI salvos com sucesso!")
print(f"Total de ativos na carteira: {len(todos_os_ativos)}")