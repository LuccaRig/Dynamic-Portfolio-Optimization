import matplotlib.pyplot as plt
import numpy as np

# Dados dos experimentos
horizontes = ['3 Anos', '7 Anos', '10 Anos']
tempo_sem_paralelismo = [242.37, 586.19, 672.79]
tempo_com_paralelismo = [50.65, 112.68, 142.68]

x = np.arange(len(horizontes))
width = 0.35

# Configurações de estilo IEEE/Acadêmico
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = 10

fig, ax = plt.subplots(figsize=(6, 4.2), dpi=300)

rects1 = ax.bar(x - width/2, tempo_sem_paralelismo, width, label='Sem Paralelismo', color='#d95f02', edgecolor='black', linewidth=0.8)
rects2 = ax.bar(x + width/2, tempo_com_paralelismo, width, label='Com Paralelismo (OpenMP)', color='#1b9e77', edgecolor='black', linewidth=0.8)

# Rótulos e Título
ax.set_ylabel('Tempo de Execução (segundos)', fontsize=11, fontweight='bold')
ax.set_xlabel('Horizonte de Teste', fontsize=11, fontweight='bold')
ax.set_xticks(x)
ax.set_xticklabels(horizontes, fontsize=10)
ax.legend(frameon=True, facecolor='white', edgecolor='none', fontsize=9)
ax.grid(axis='y', linestyle='--', alpha=0.5)

# Adicionar os valores numéricos e o speedup no topo das barras
def autolabel(rects, speedups=None):
    for i, rect in enumerate(rects):
        height = rect.get_height()
        ax.annotate(f'{height:.1f}s',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=8.5, fontweight='bold')

speedups = [s / c for s, c in zip(tempo_sem_paralelismo, tempo_com_paralelismo)]

autolabel(rects1)
autolabel(rects2, speedups)

# Limite do eixo Y com margem para as anotações
ax.set_ylim(0, max(tempo_sem_paralelismo) * 1.15)

plt.tight_layout()
plt.savefig('paralelismo_speedup.png', dpi=300)
plt.show()