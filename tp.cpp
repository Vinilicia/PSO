#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <random>


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
    vector<int> melhor_solucao;
    vector<int> solucao_atual;
};

double custo_caminho(double **matriz_dist, vector<int> caminho, int n) {
    double custo = 0;
    for (int i = 0; i < n - 1; i++) {
        cout << caminho[i] << " " << caminho[i+1] << endl;
        custo += matriz_dist[caminho[i]][caminho[i+1]]; 
    }
    custo += matriz_dist[caminho[n-1]][caminho[0]]; // Volta para a cidade inicial
    return custo;
}

int acha_correspondente(int p, vector<int> solucao){
    for ( int i = 0; i < solucao.size(); i++){
        if (solucao[i] == p){
            return i;
        }
    }
    return -1;
}

vector<int> solucao_aleatoria(int n) {
    vector<int> solucao(n);
    for (int i = 0; i < n; i++) {
        solucao[i] = i + 1;
    }
    random_device rd;
    mt19937 g(rd());
    shuffle(solucao.begin(), solucao.end(), g);
    return solucao;
}

vector<Troca> calcula_trocas(vector<int> solucao_melhor, vector<int> solucao_pior){
    vector<Troca> trocas;
    int n = solucao_melhor.size();
    for( int i = 0; i < n; i++){
        int correspondente = acha_correspondente(solucao_pior[i], solucao_melhor);
        if (correspondente != i){
            Troca nova_troca;
            nova_troca.pos1 = i;
            nova_troca.pos2 = correspondente;
            trocas.push_back(nova_troca);
        }
    }
    return trocas;
}

vector<int> pso(vector<Particula> particulas, vector<Cidade> cidades, double** matriz_dist, int n){
    cout << "Bug" << endl;
    vector<int> melhor_global = particulas[0].solucao_atual;
    int iteracoes = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);
    double alpha = dist(gen);
    double beta = dist(gen);
    for (int i = 1; i < n; i++){
        if (custo_caminho(matriz_dist, melhor_global, n) > custo_caminho(matriz_dist, particulas[i].melhor_solucao, n)){
            melhor_global = particulas[i].melhor_solucao;
        }
    }
    
    while(iteracoes < 500){
        for(int i = 0; i < particulas.size(); i++){
            double alpha = dist(gen);
            double beta = dist(gen);
            vector<Troca> trocas_locais;
            vector<Troca> trocas_globais;
            vector<Troca> trocas_finais;
            trocas_locais = calcula_trocas(particulas[i].melhor_solucao, particulas[i].solucao_atual);
            trocas_globais = calcula_trocas(melhor_global, particulas[i].solucao_atual);
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
                swap(particulas[i].solucao_atual[trocas_finais[j].pos1], particulas[i].solucao_atual[trocas_finais[j].pos2]);
            }
            if (custo_caminho(matriz_dist, particulas[i].solucao_atual, n) < custo_caminho(matriz_dist, particulas[i].melhor_solucao, n)){
                particulas[i].melhor_solucao = particulas[i].solucao_atual;
                if (custo_caminho(matriz_dist, melhor_global, n) > custo_caminho(matriz_dist, particulas[i].melhor_solucao, n)){
                    melhor_global = particulas[i].melhor_solucao;
                }
            }
        }
        iteracoes++;
    }
    return melhor_global;
}

void printar_solucao(const vector<int>& solucao, int n) {
    for (int i = 0; i < n; i++) {
        cout << solucao[i] << " ";
    }
    cout << endl;
}

int main(){
    ifstream arq("Input_TSP.txt");
    int n;
    if (!arq.is_open()) {
        cerr << "Arquivo não encontrado." << endl;
        return 1;
    }
    arq >> n;

    vector<Cidade> cidades(n);
    vector<int> solucao(n);
    vector<Particula> particulas((int)(n/4));
    for (int i = 0; i < n; i++) {
        arq >> cidades[i].x;
        arq >> cidades[i].y;
        solucao[i] = i;
    }

    double **matriz_dist = (double**)malloc(n * sizeof(double*)); // Matriz de distâncias
    for (int i = 0; i < n; i++) {
        matriz_dist[i] = (double*) malloc(n * sizeof(double));
        for (int j = 0; j < n; j++) {
            matriz_dist[i][j] = sqrt(pow(cidades[i].x - cidades[j].x, 2) + pow(cidades[i].y - cidades[j].y, 2));
        }
    }
    for ( int i = 0; i < n; i++){
        particulas[i].solucao_atual = particulas[i].melhor_solucao = solucao_aleatoria(n);
    }
    cout << "Bug" << endl;
    
    solucao = pso(particulas, cidades, matriz_dist, n);
    printar_solucao(solucao, n);
    cout << "Custo do caminho: " << custo_caminho(matriz_dist, solucao, n) << endl;






    for (int i = 0; i < n; i++) {
        free(matriz_dist[i]);
    }
    free(matriz_dist);
    return 0;
}