import cv2
import numpy as np
import subprocess
import sys

H, W = 64, 64

# Mesma imagem sintetica gerada em src/main.c.
img = np.fromfunction(lambda r, c: (r + c) & 0xFF, (H, W), dtype=np.int32)

def build_integral_opencv(img):
    """
    Constroi a Integral Image com OpenCV para servir como referencia.

    A funcao cv2.integral() retorna uma matriz com padding extra na primeira
    linha e na primeira coluna. Esse padding e removido para que o resultado
    fique no mesmo formato usado pelo codigo em C.

    Complexidade: O(H x W)
    """
    integral = cv2.integral(img.astype(np.uint8))
    return integral[1:, 1:].astype(np.int64)


integral_py = build_integral_opencv(img)

#Exportar como .txt da mesma maneira do codigo em C, para comparar os arquivos gerados.
def export_integral_python(integral, filename="integral_py.txt"):
    """
    Exporta a matriz `integral` para um arquivo de texto no mesmo formato
    usado por `src/main.c` (cada elemento seguido de espaço, newline ao fim
    de cada linha).
    """
    with open(filename, "w") as f:
        for r in range(H):
            for c in range(W):
                f.write(f"{int(integral[r, c])} ")
            f.write("\n")


# escreve `integral_py.txt` para facilitar comparacoes externas
export_integral_python(integral_py)

def ensure_integral_from_c():
    """
    Compila e executa o programa C que gera os arquivos de teste.

    O Python nao escreve `integral_c.txt`; ele apenas chama o binario C.
    A matriz carregada depois vem do algoritmo implementado em C.
    """
    compile_cmd = ["gcc", "-Wall", "-Wextra", "-o", "teste", "src/main.c", "src/integral_image.c"]
    print("Compilando programa C para gerar integral_c.txt...")
    res = subprocess.run(compile_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        print("Erro ao compilar src/main.c:\n", res.stderr)
        sys.exit(1)

    print("Executando binario C para gerar arquivos...")
    res = subprocess.run(["./teste"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        print("Erro ao executar ./teste:\n", res.stderr)
        sys.exit(1)

    print("Arquivo integral_c.txt gerado pelo programa C.")

ensure_integral_from_c()

# Carrega a matriz exportada pelo C.
integral_c = np.loadtxt("integral_c.txt", dtype=np.int64)

# ---------------------------
# 1. Validação pixel a pixel
# ---------------------------

diff = np.abs(integral_c - integral_py)

if np.all(diff == 0):
    print("Matrizes iguais (pixel a pixel)")
else:
    print("Diferença encontrada")
    print("Erro máximo:", diff.max())

# ---------------------------
# 2. Validação das queries
# ---------------------------

def get_sum_python(r1, c1, r2, c2):
    """
    Calcula a soma de uma regiao usando a Integral Image do Python.

    Identidade usada:
      soma = A - B - C + D

    Complexidade: O(1)
    """
    if r1 > r2 or c1 > c2:
        return 0

    A = integral_py[r2, c2]
    B = integral_py[r1 - 1, c2] if r1 > 0 else 0
    C = integral_py[r2, c1 - 1] if c1 > 0 else 0
    D = integral_py[r1 - 1, c1 - 1] if (r1 > 0 and c1 > 0) else 0

    return int(A - B - C + D)

ok = True

with open("queries.txt") as f:
    for i, line in enumerate(f):
        r1, c1, r2, c2, c_val = map(int, line.split())

        py_val = get_sum_python(r1, c1, r2, c2)

        if py_val != c_val:
            print(f"❌ ERRO query {i}")
            print(f"C={c_val}, PY={py_val}")
            ok = False
            break

print("Validação queries:", "✅ OK" if ok else "❌ ERRO")


def compare_integral_files(c_file="integral_c.txt", py_file="integral_py.txt"):
    """
    Compara os dois arquivos de saída: primeiro tenta carregar numericamente
    com NumPy e compara elemento a elemento; se falhar, faz uma comparação
    texto (linha a linha) e um comparador byte-a-byte.
    """
    print(f"\nComparando arquivos: {c_file} x {py_file}")

    # Comparacao numerica com NumPy (caso o formato seja carregavel)
    try:
        a_c = np.loadtxt(c_file, dtype=np.int64)
        a_py = np.loadtxt(py_file, dtype=np.int64)

        if a_c.shape != a_py.shape:
            print("Formato diferente:", a_c.shape, "vs", a_py.shape)
        else:
            diff = np.abs(a_c - a_py)
            if np.all(diff == 0):
                print("Comparacao numerica: arquivos equivalentes (todos os elementos iguais)")
            else:
                print("Comparacao numerica: diferencas encontradas")
                print("Erro maximo:", int(diff.max()))
                # mostra primeiro indice com diferenca
                idx = np.argwhere(diff != 0)[0]
                print(f"Primeira diferenca em (r,c)=({idx[0]},{idx[1]}): C={a_c[idx[0],idx[1]]} PY={a_py[idx[0],idx[1]]}")
        return
    except Exception as e:
        print("Comparacao numerica falhou:", e)

    # Fallback: comparacao linha-a-linha e byte-a-byte
    try:
        with open(c_file, "rb") as f1, open(py_file, "rb") as f2:
            b1 = f1.read()
            b2 = f2.read()
            if b1 == b2:
                print("Comparacao byte-a-byte: arquivos idênticos")
                return
            else:
                print("Comparacao byte-a-byte: arquivos diferentes")
    except Exception as e:
        print("Erro ao ler arquivos para comparacao byte-a-byte:", e)

    # Se ainda assim diferente, tenta mostrar primeiras linhas distintas
    try:
        with open(c_file, "r") as f1, open(py_file, "r") as f2:
            for i, (l1, l2) in enumerate(zip(f1, f2)):
                if l1 != l2:
                    print(f"Primeira linha diferente (linha {i}):")
                    print("C:", l1.rstrip())
                    print("PY:", l2.rstrip())
                    break
    except Exception as e:
        print("Erro na comparacao textual:", e)


# Executa a comparacao dos arquivos gerados
compare_integral_files()
