#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <random>

#define MAX_ITER 1500
#define MAX_ITER_SEM_MELHORA 50
#define PARTICULAS 10000
#define alpha 0.4 // Coeficiente de aceleracao individual
#define beta 0.7 // Coeficiente de aceleração social

using namespace std;

struct Cidade{
    int x;
    int y;
};

struct Troca{
    int pos1;
    int pos2;

    bool operator==(const Troca &other) const {
        return pos1 == other.pos1 && pos2 == other.pos2;
    }
};

struct Particula{
    vector<int> melhor_caminho;
    vector<int> caminho_atual;
};

vector<Cidade> ler_arquivo(char **argv){
    ifstream arq(argv[1]);
    int n;
    if (!arq.is_open()) {
        cerr << "Arquivo não encontrado." << endl;
        exit(1);
    }
    arq >> n;
    vector<Cidade> cidades(n);
    for (int i = 0; i < n; i++) {
        arq >> cidades[i].x;
        arq >> cidades[i].y;
    }
    return cidades;
}

double **gerar_matriz_distancias(vector<Cidade> cidades){
    int n = cidades.size();
    double **matriz_distancias = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        matriz_distancias[i] = (double*) malloc(n * sizeof(double));
        for (int j = 0; j < n; j++) {
            matriz_distancias[i][j] = sqrt(pow(cidades[i].x - cidades[j].x, 2) + pow(cidades[i].y - cidades[j].y, 2));
        }
    }
    return matriz_distancias;
}

void liberar_matriz_distancias(double **matriz_distancias, int n){
    for (int i = 0; i < n; i++) {
        free(matriz_distancias[i]);
    }
    free(matriz_distancias);
}

vector<int> gerar_caminho_aleatorio(int n) {
    vector<int> caminho(n);
    for (int i = 0; i < n; i++) {
        caminho[i] = i;
    }
    random_device rd;
    mt19937 g(rd());
    shuffle(caminho.begin(), caminho.end(), g);
    return caminho;
}

vector<Particula> gerar_particulas(double **matriz_distancias, int n){
    vector<Particula> particulas(PARTICULAS);
    for (int i = 0; i < particulas.size(); i++){
        particulas[i].caminho_atual = particulas[i].melhor_caminho = gerar_caminho_aleatorio(n);
    }
    return particulas;
}

double custo_caminho(double **matriz_distancias, vector<int> caminho) {
    int n = caminho.size();
    double custo = 0;
    for (int i = 0; i < n - 1; i++) {
        custo += matriz_distancias[caminho[i]][caminho[i+1]]; 
    }
    custo += matriz_distancias[caminho[n-1]][caminho[0]]; // Volta para a cidade inicial
    return custo;
}

int achar_correspondente(int p, vector<int> caminho){
    for (int i = 0; i < caminho.size(); i++){
        if (caminho[i] == p){
            return i;
        }
    }
}

vector<Troca> calcular_trocas(vector<int> &melhor_caminho, vector<int> &pior_caminho){
    vector<Troca> trocas;
    int n = melhor_caminho.size();
    for( int i = 0; i < n; i++){
        int correspondente = achar_correspondente(pior_caminho[i], melhor_caminho);
        if (correspondente != i){
            Troca nova_troca;
            nova_troca.pos1 = i;
            nova_troca.pos2 = correspondente;
            trocas.push_back(nova_troca);
        }
    }
    return trocas;
}

int encontrar_vizinho_mais_proximo(double **matriz_distancias, int cidade_atual, const vector<bool>& visitados, int n) {
    int visinho_mais_proximo = -1;
    int menor_distancia = numeric_limits<int>::max();
    for (int i = 0; i < n; ++i) {
        if (!visitados[i] && matriz_distancias[cidade_atual][i] < menor_distancia) {
            menor_distancia = matriz_distancias[cidade_atual][i];
            visinho_mais_proximo = i;
        }
    }

    return visinho_mais_proximo;
}

vector<int> metodo_guloso(double** matriz_distancias, int n) {
    vector<int> solucao_gulosa;
    vector<bool> visitados(n, false);
    int cidade_atual = 0;
    solucao_gulosa.push_back(cidade_atual);
    visitados[cidade_atual] = true;
    for (int i = 1; i < n; ++i) {
        int proxima_cidade = encontrar_vizinho_mais_proximo(matriz_distancias, cidade_atual, visitados, n);
        solucao_gulosa.push_back(proxima_cidade);
        visitados[proxima_cidade] = true;
        cidade_atual = proxima_cidade;
    }

    return solucao_gulosa;
}

vector<int> PSO(vector<Particula> &particulas, double** matriz_distancias, int numero_cidades){
    int n = particulas.size();
    vector<int> melhor_global = particulas[0].caminho_atual;
    int iteracoes = 0;
    int iteracoes_sem_melhora = 0;
    bool houve_melhora = false;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);

    // Gera um primeiro caminho aleatório para um melhor global  
    for (int i = 1; i < n; i++){
        if (custo_caminho(matriz_distancias, melhor_global) > custo_caminho(matriz_distancias, particulas[i].melhor_caminho)){
            melhor_global = particulas[i].melhor_caminho;
        }
    }
    
    // Gera uma solucao gulosa
    vector<int> solucao_gulosa = metodo_guloso(matriz_distancias, numero_cidades);
    if(custo_caminho(matriz_distancias, solucao_gulosa) < custo_caminho(matriz_distancias, melhor_global)){
        melhor_global = solucao_gulosa;
    }
    
    while(iteracoes < MAX_ITER && iteracoes_sem_melhora != MAX_ITER_SEM_MELHORA){
        // Pega um vizinho do caminho atual com base nas trocas de inteiros
        for(int i = 0; i < n; i++){
            vector<Troca> trocas_locais;
            vector<Troca> trocas_globais;
            vector<Troca> trocas_finais;
            trocas_locais = calcular_trocas(particulas[i].melhor_caminho, particulas[i].caminho_atual);
            trocas_globais = calcular_trocas(melhor_global, particulas[i].caminho_atual);
           for (int j = 0; j < trocas_locais.size(); j++){
                if (dist(gen) < alpha){
                    trocas_finais.push_back(trocas_locais[j]);
                }
            }
            for (int j = 0; j < trocas_globais.size(); j++){
                if (dist(gen) < beta){
                    bool troca_existente = false;
                    for(int k = 0; k < trocas_locais.size(); k++){
                        if(trocas_globais[j] == trocas_locais[k]){
                            troca_existente = true;
                            break;
                        }
                    }
                    if(!troca_existente)
                        trocas_finais.push_back(trocas_globais[j]);
                }
            }
            for (int j = 0; j < trocas_finais.size(); j++){
                swap(particulas[i].caminho_atual[trocas_finais[j].pos1], particulas[i].caminho_atual[trocas_finais[j].pos2]);
            }
            if (custo_caminho(matriz_distancias, particulas[i].caminho_atual) < custo_caminho(matriz_distancias, particulas[i].melhor_caminho)){
                particulas[i].melhor_caminho = particulas[i].caminho_atual;
                if (custo_caminho(matriz_distancias, melhor_global) > custo_caminho(matriz_distancias, particulas[i].melhor_caminho)){
                    melhor_global = particulas[i].melhor_caminho;
                    houve_melhora = true;
                }
            }
        }
        iteracoes++;
        if(houve_melhora){
            iteracoes_sem_melhora = 0;
            houve_melhora = false;
        }
        else{
            iteracoes_sem_melhora++;
        }
    }

    return melhor_global;
}

void imprimir_caminho(const vector<int> caminho) {
    int n = caminho.size();
    cout << "Caminho" << endl;
    for (int i = 0; i < n; i++) {
        cout << caminho[i] + 1<< " ";
    }
    cout << endl;
}

int main(int argc, char *argv[]){
    vector<Cidade> cidades = ler_arquivo(argv);
    double **matriz_distancias = gerar_matriz_distancias(cidades);
    vector<Particula> particulas = gerar_particulas(matriz_distancias, cidades.size());
    vector<int> caminho = PSO(particulas, matriz_distancias, cidades.size());
    imprimir_caminho(caminho);
    cout << "Custo do caminho: " << custo_caminho(matriz_distancias, caminho) << endl;
    liberar_matriz_distancias(matriz_distancias, cidades.size());

    return 0;
}