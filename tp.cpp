#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <random>

#define MAX_ITER 500

using namespace std;

struct Cidade{
    int x;
    int y;
};

struct Troca{
    int pos1;
    int pos2;
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
    vector<int> caminho(n);
    vector<Particula> particulas((int)(n/4));
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
    vector<Particula> particulas((int)(n/4));
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

vector<Troca> calcula_trocas(vector<int> &melhor_caminho, vector<int> &pior_caminho){
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

vector<int> PSO(vector<Particula> particulas, vector<Cidade> cidades, double** matriz_distancias){
    int n = cidades.size();
    int m = particulas.size();
    vector<int> melhor_global = particulas[0].caminho_atual;
    int iteracoes = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);
    double alpha = dist(gen);
    double beta = dist(gen);

    // Gera um primeiro caminho aleatório para um melhor global  
    for (int i = 1; i < m; i++){
        if (custo_caminho(matriz_distancias, melhor_global) > custo_caminho(matriz_distancias, particulas[i].melhor_caminho)){
            melhor_global = particulas[i].melhor_caminho;
        }
    }
    
    while(iteracoes < MAX_ITER){

        // Pega um vizinho do caminho atual com base nas trocas de inteiros
        for(int i = 0; i < m; i++){
            double alpha = dist(gen);
            double beta = dist(gen);
            vector<Troca> trocas_locais;
            vector<Troca> trocas_globais;
            vector<Troca> trocas_finais;
            trocas_locais = calcula_trocas(particulas[i].melhor_caminho, particulas[i].caminho_atual);
            trocas_globais = calcula_trocas(melhor_global, particulas[i].caminho_atual);
            for (int j = 0; j < trocas_locais.size(); j++){
                if (dist(gen) < alpha){
                    trocas_finais.push_back(trocas_locais[i]);
                }
            }
            for (int j = 0; j < trocas_globais.size(); j++){
                if (dist(gen) < beta){
                    trocas_finais.push_back(trocas_globais[i]);
                }
            }
            for (int j = 0; j < trocas_finais.size(); j++){
                swap(particulas[i].caminho_atual[trocas_finais[j].pos1], particulas[i].caminho_atual[trocas_finais[j].pos2]);
            }
            if (custo_caminho(matriz_distancias, particulas[i].caminho_atual) < custo_caminho(matriz_distancias, particulas[i].melhor_caminho)){
                particulas[i].melhor_caminho = particulas[i].caminho_atual;
                if (custo_caminho(matriz_distancias, melhor_global) > custo_caminho(matriz_distancias, particulas[i].melhor_caminho)){
                    melhor_global = particulas[i].melhor_caminho;
                }
            }
        }
        iteracoes++;
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
    vector<int> caminho = PSO(particulas, cidades, matriz_distancias);
    imprimir_caminho(caminho);
    cout << "Custo do caminho: " << custo_caminho(matriz_distancias, caminho) << endl;
    liberar_matriz_distancias(matriz_distancias, cidades.size());

    return 0;
}