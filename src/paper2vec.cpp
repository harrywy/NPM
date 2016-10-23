#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <math.h>
#include <pthread.h>
#include <vector>
#include <boost/algorithm/string.hpp>

#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define D 8992476
using namespace std;

typedef double real;

const int num_thread = 24;

vector<string> paper;
string* word;
vector<int> paper_count;
long long word_num, paper_num, size;
real **word_vec, **paper_vec, **m_word, **m_paper;
unordered_map<string, int> word_idx, paper_idx;
real *idf;

int negative = 15;
real alpha = 0.005, starting_alpha = 0.005, sample = 0;
real *expTable;
const int table_size = 1e8;
int *table;


void InitUnigramTable() {
    int a, i;
    long long train_word_pow = 0;
    real d1, power = 0.75;
    table = (int *)malloc(table_size * sizeof(int));
    for (a = 0; a < word_num; a++) train_word_pow += pow(idf[a], power);
    i = 0;
    d1 = pow(idf[i], power) / (real)train_word_pow;
    for (a = 0; a < table_size; a++) {
        table[a] = i;
        if (a / (real)table_size > d1) {
            i++;
            d1 += pow(idf[i], power) / (real)train_word_pow;
        }
        if (i >= word_num) i = word_num - 1;
    }
}

void loadWord(char* model_file, char* word_file) {
    ifstream f(model_file);
    f >> word_num >> size;
    word_vec = (real **) malloc(word_num * sizeof(real *));
    word = new string[word_num];
    for (int i = 0; i < word_num; ++i) {
        f >> word[i];
        real len = 0.0;
        word_vec[i] = (real *) malloc(size * sizeof(real));
        for (int j = 0; j < size; j++) {
            f >> word_vec[i][j];
            len += word_vec[i][j] * word_vec[i][j];
        }        
		len = sqrt(len);
        for (int j= 0; j < size; j++) word_vec[i][j] /= len;
        word_idx[word[i]] = i;
    }
    f.close();

    idf = (real *) malloc(word_num * sizeof(real));
    f.open(word_file);
    for (int i = 0; i < word_num; ++i) {
        string w;
        f >> w >> idf[i];
        idf[i] = log(D / idf[i]);
    }
    f.close();
    printf("%d words vec loaded!\n", word_num);
}

long long train_size = 0;
struct train_data {
    int cited;
    vector<int> context;
};
vector<train_data> train;

void loadTrainData(char* train_file) {
    ifstream f(train_file);
    string w;
    paper_num = 0;
    while (getline(f, w)) {
        int i_1 = w.find('\t');
        int i_2 = w.rfind('\t');
        string cited = w.substr(i_1 + 1, i_2 - i_1 -1);
        int id;
        if (paper_idx.count(cited) == 0) {
            id = paper_num++;
            paper_idx[cited] = id;
            paper_count.push_back(1);
            paper.push_back(cited);
        } else{
            id = paper_idx[cited];
            paper_count[id] += 1;
        }

        string context = w.substr(i_2 + 1);
        //add to train
        train.push_back(train_data());
        train[train_size].cited = id;

        vector<string> tmp;
        boost::split(tmp, context, boost::is_any_of(" "));
        for (vector<string>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
            if (word_idx.count(*it)) {
                id = word_idx[*it];
                train[train_size].context.push_back(id);
            }
        }            
        train_size++;
    }
    f.close();   
    printf("%d Training data Loaded!\n", train_size);    
}

void initNet() {
    //init word matrix
    m_word = (real **) malloc(word_num * sizeof(real *));
    for (int i = 0; i < word_num; ++i) {
        m_word[i] = (real *) malloc(size * sizeof(real));
        for (int j = 0; j < size; ++j) m_word[i][j] = cbrt((rand() / (real)RAND_MAX - 0.5) / size);
    }
    //init paper matrix
    m_paper = (real **) malloc(paper_num * sizeof(real *));
    for (int i = 0; i < paper_num; ++i) {
        m_paper[i] = (real *) malloc(size * sizeof(real));
        for (int j = 0; j < size; ++j) m_paper[i][j] = cbrt((rand() / (real)RAND_MAX - 0.5) / size);
    }

    //init paper vector
    paper_vec = (real **) malloc(paper_num * sizeof(real *));
    for (int i = 0; i < paper_num; ++i) {
        paper_vec[i] = (real *) malloc(size * sizeof(real));
        for (int j = 0; j < size; ++j) paper_vec[i][j] = (rand() / (real)RAND_MAX - 0.5) / size;
    }
}

struct thread_data {
    int id;
    int start;
    int end;
};

struct thread_data thread_data_array[num_thread];

void *prepareIndex(void *arg){
    struct thread_data *args = (struct thread_data *) arg;
    int start = args->start;
    int end = args->end;
    long long target, label;
    unsigned long long next_random = (long long) args->id;
    real f, g;
    string tmp;
    real *update = (real *)malloc(size * sizeof(real));
    real *m_1 =  (real *)malloc(size * sizeof(real));
    real *m_2 =  (real *)malloc(size * sizeof(real));

    //printf("Thread %d process data from %d to %d\n", args->id, start, end);
    for (int i = start; i < end; ++i) {
        for (int j = 0; j < size; ++j) update[j] = 0;
        //alpha  = starting_alpha * (1 - i / (real)(train_size+ 1));
        //if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
        int cited_idx = train[i].cited;
        //cout << i << ' ' <<paper[cited_idx] << endl;
        for (vector<int>::iterator it = train[i].context.begin();
                it != train[i].context.end(); ++it) {
            int w_idx = *it;
            for (int j = 0; j < size; ++j) { m_1[j] = 0; m_2[j] = 0; }
            //cout << word[*it] << ' ';
            for (int j = 0; j < negative; ++j) {
                if (j == 0) {
                    target = w_idx;
                    label = 1;
                } else {
                    next_random = next_random * (unsigned long long)25214903917 + rand();
                    target = table[(next_random >> 16) % table_size];
                    if (target == w_idx) continue;
                    label = 0;
                }

                f = 0.0;
                for (int k = 0; k < size; k++) {
                    //printf("%f = %f * %f * %f * %f\n", (real)(word_vec[target][k] * m_word[target][k]) * (real)(paper_vec[cited_idx][k] * m_paper[cited_idx][k]),
                    //        word_vec[target][k], m_word[target][k], paper_vec[cited_idx][k], m_paper[cited_idx][k]);
                    //f += (word_vec[target][k] * m_word[target][k]) * (paper_vec[cited_idx][k] * m_paper[cited_idx][k]);
                    f +=  word_vec[target][k] * paper_vec[cited_idx][k];
                }
                //printf("No. %d: f = %f\n", i, f);
                if (f > MAX_EXP) g = (label - 1) * alpha;
                else if (f < -MAX_EXP) g = (label - 0) * alpha;
                else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;

                for (int k = 0; k < size; k++) {
                    update[k] += g * word_vec[target][k] * idf[target];
                    //update[k] += g * word_vec[target][k] * m_paper[cited_idx][k] * m_word[target][k];
                    //m_1[k] += g * word_vec[target][k] * m_paper[cited_idx][k] * paper_vec[cited_idx][k];
                    //m_2[k] += g * word_vec[target][k] * m_word[target][k] * paper_vec[cited_idx][k];
                    //printf("update %f\n", g * word_vec[target][k] * m_paper[cited_idx][k] * m_word[target][k]);
                }
                /*
                for (int k = 0; k < size; k++) {
                    m_word[target][k] += m_1[k];
                    m_paper[cited_idx][k] += m_2[k];
                }*/
            }
        }
        //real len = 0.0;
        for (int k = 0; k < size; k++) {
            paper_vec[cited_idx][k] += update[k];
            //printf("%f ", update[k]);
            //len += paper_vec[cited_idx][k] * paper_vec[cited_idx][k];
        }      
        //len = sqrt(len);
        //for (int k = 0; k < size; k++) paper_vec[cited_idx][k] /= len;
    }
    //printf("\n");
    free(update);
    free(m_1);
    free(m_2);
    pthread_exit(NULL);
}

void output(char* file){
    ofstream f(file);
    f << paper_num << ' ' << size << endl;
    for (int i = 0; i < paper_num; ++i) {
        f << paper[i];
        for (int j =0; j < size; ++ j)
            f << ' ' << paper_vec[i][j];
        f << endl;
    }
    f.close();
}

void training(int t, char* dump){
    for (int iter = 0; iter < t; ++iter) {        
        printf("Iteration %d: \n", iter);
        pthread_t *pt = (pthread_t *)malloc(num_thread * sizeof(pthread_t));
        int each = train_size / num_thread;
        for (int i = 0; i < num_thread; ++i) {
            thread_data_array[i].id = i;
            thread_data_array[i].start = i * each;
            thread_data_array[i].end = (i + 1) * each;
        }
        thread_data_array[num_thread-1].end = train_size;

        for (int i = 0; i < num_thread; ++i)
            pthread_create(&pt[i], NULL, prepareIndex, (void *)&thread_data_array[i]);
        for (int i = 0; i < num_thread; ++i)
            pthread_join(pt[i], NULL);
        alpha = alpha / 2;
        if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
        
        if (iter % 10 == 0) {
            printf("Dumping %d \n", iter);
        
            string str(dump);
            str = str +  "." + to_string((long long)iter);
            char* file = new char[str.length()+1];
            strcpy(file, str.c_str());
            output(file); 
        }
    }
}

int main(int argc, char **argv) {
    //input
    loadWord(argv[1], argv[2]);
    loadTrainData(argv[3]);
    // Precompute f(x) = x / (x + 1)
    expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    for (int i = 0; i < EXP_TABLE_SIZE; i++) {
        expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); 
        expTable[i] = expTable[i] / (expTable[i] + 1); 
    }
    InitUnigramTable();
    initNet();
    //training
    training(31, argv[4]);
}

