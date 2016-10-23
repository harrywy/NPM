#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <pthread.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#define EXP_TABLE_SIZE 2000
#define MAX_EXP 10
#define D 8965558
#define T 346479
using namespace std;

//typedef long double real;
typedef double real;

const int num_thread = 24;

vector<string> paper;
string* word;
int*  paper_count;
long long word_num, paper_num, size;
real **word_vec, **paper_vec;
unordered_map<string, int> word_idx, paper_idx;
unordered_map<int, real> *ctm;
vector<vector<int> > title;
real *idf;

int negative = 1000;

real alpha = 1.0, starting_alpha = 1.0;
real *expTable;
int table_size;
int *table;
real pos_sigmod, neg_sigmod, num_pos, num_neg;

void loadMT(char* MT_file) {
    ifstream f(MT_file);
    string str;
    while (getline(f, str)) {
        int l = str.find(' ');
        int r = str.rfind(' ');
        string w = str.substr(0, l);
        if (word_idx.count(w)) {
            int w_idx = word_idx[w]; 
            string p = str.substr(l+1, r-l-1);
            int p_idx = paper_idx[p];
            string s = str.substr(r+1);
            real score = atof(s.c_str());
            ctm[w_idx][p_idx] = score;
        }
    }
    printf("CTM file loaded!\n");
}

void loadTitle(char* title_file) {
    ifstream f(title_file);
    string str;
    paper_num = 0;
    while (getline(f, str)) {        
        int i = str.find(' ');
        string c = str.substr(0, i);
        vector<int> temp;
        string t = str.substr(i+1);
        vector<string> tmp;
        boost::split(tmp, t, boost::is_any_of(" "));        
        for (vector<string>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
            if (word_idx.count(*it)) {
                int w_idx = word_idx[*it];            
                temp.push_back(w_idx);
            }
        }
        //if (temp.size() != 0) {        
            title.push_back(temp);
            paper_idx[c] = paper_num;
            paper.push_back(c);        
            paper_num++;
        //}
        //cout << paper_num << ' ' << str << endl;        
    }
    f.close();
    // initial with title
    
    paper_count = (int *) calloc(paper_num, sizeof(int));
    paper_vec = (real **) malloc(paper_num * sizeof(real *));
    for (int i = 0; i < paper_num; ++i) {
        paper_vec[i] = (real *) calloc(size, sizeof(real));
        /*
        real weight = 0.0;
        for (vector<int>::iterator it = title[i].begin(); it != title[i].end(); ++it) {
            for (int j = 0; j < size; ++j)
                paper_vec[i][j] += word_vec[*it][j] * idf[*it];
            weight += idf[*it];   
        }
        if (weight ==0.0) weight = 1.0;
        for (int j = 0; j < size; ++j)
            paper_vec[i][j] /= weight;*/
    }
    printf("Title file loaded with %lld papers\n", paper_num); 
}

void loadWord(char* model_file, char* word_file) {
    ifstream f(model_file);
    f >> word_num >> size;
    word_vec = (real **) malloc(word_num * sizeof(real *));
    word = new string[word_num];
    for (int i = 0; i < word_num; ++i) {
        f >> word[i];
        word_vec[i] = (real *) malloc(size * sizeof(real));
        real len = 0.0;
        for (int j = 0; j < size; j++) {
            f >> word_vec[i][j];
            len += word_vec[i][j] * word_vec[i][j];
        }
        len = sqrt(len);
        for (int j = 0; j < size; j++) word_vec[i][j] /= len;
        word_idx[word[i]] = i;
    }
    f.close();

    ctm = new unordered_map<int, real>[word_num];
    idf = (real *) calloc(word_num, sizeof(real));
    f.open(word_file);
    string w;
    real tmp;
    while (true) {
        f >> w >> tmp;
        if (f.eof()) break;
        tmp = log(T / tmp);
        if (word_idx.count(w))
            idf[word_idx[w]] = tmp;
    }
    f.close();
    printf("%lld words vec loaded!\n", word_num);
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
    while (getline(f, w)) {
        int i_1 = w.find('\t');
        int i_2 = w.rfind('\t');
        string cited = w.substr(i_1 + 1, i_2 - i_1 -1);
        int id;
        if (paper_idx.count(cited) == 1) {
            id = paper_idx[cited];
            paper_count[id] +=1;
        } else{
            continue;
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
    printf("%lld Training data Loaded!\n", train_size);
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
    long long target;
    boost::random::mt19937 mt(args->id);
    boost::random::uniform_int_distribution<> dist(0, 281816);

    real f, g;
    string tmp;
    real *update_w = (real *)malloc(size * sizeof(real));
    real *update_p = (real *)malloc(size * sizeof(real));
    //printf("Thread %d process data from %d to %d\n", args->id, start, end);
    for (int i = start; i < end; ++i) {
        real total_s = 0.0;
        for (int j = 0; j < size; ++j) update_p[j] = 0.0;
        int cited_idx = train[i].cited;
        for (vector<int>::iterator it = train[i].context.begin();
                it != train[i].context.end(); ++it) {
            if (ctm[*it].count(cited_idx)) total_s += ctm[*it][cited_idx]; 
        }
        if (total_s == 0) continue;
        //printf("%d %d %f\n", i, cited_idx, total_s);
        //cout << paper[cited_idx];
        for (vector<int>::iterator it = train[i].context.begin();
                it != train[i].context.end(); ++it) {
            int w_idx = *it;
            real score = 0.0, param = 0.0;
            for (int j = 0; j < size; ++j) update_w[j] = 0.0;            
            if (ctm[w_idx].count(cited_idx)) {
                score = ctm[w_idx][cited_idx] / total_s;
            } else {
                continue;
            }
            //unordered_set<long long> tmp;
            for (int j = 0; j < negative; ++j) {
                if (j == 0) {
                    target = w_idx;
                } else {
                    target = dist(mt);
                    if (ctm[target].count(cited_idx)) continue;                   
                    //if (ctm[w_idx].count(target) || tmp.count(target)) continue;
                    //else tmp.insert(target);
                }
                f = 0.0;
                for (int k = 0; k < size; k++) f += word_vec[target][k] * paper_vec[cited_idx][k];
                //printf("No. %d: Target = %lld, f = %f\n", j, target, f);
                real sigmod;
                if (f > MAX_EXP) sigmod = 1.0;
                else if (f < -MAX_EXP) sigmod = 0.0;
                else sigmod = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                
                if (j == 0) param = (real)negative / (real)(word_num * sigmod + negative);
                else param = -(real)(word_num * sigmod) / (real)(word_num * sigmod + negative);
                //printf("Param: = %f, sigmod = %f, score = %f\n", param, sigmod, score); 

                g = (1 - sigmod) * param * alpha;
                if (j == 0) { 
                    pos_sigmod += abs(sigmod - ctm[w_idx][cited_idx]); 
                    num_pos += 1.0; 
                } else { 
                    neg_sigmod += sigmod; 
                    num_neg += 1.0;
                }

                //printf("g: = %f \n", g);
                //if (i >= 41) while (getchar()!='\n');
                for (int k = 0; k < size; k++) {
                    //update_w[k] += g * paper_vec[target][k];
                    //printf("%f\n", g * paper_vec[target][k]);
                    //while (getchar()!='\n');
                    update_p[k] += g * word_vec[target][k] * score;                    
                }
            }
            //cout << endl;
            //for (int k = 0; k < size; k++) word_vec[w_idx][k] += update_w[k];
        }
        //cout << endl;
        for (int k = 0; k < size; k++) {
            paper_vec[cited_idx][k] += update_p[k];
            //printf("%f ", update_p[k]);
        }
    }
    //printf("\n");
    free(update_p);
    free(update_w);
    pthread_exit(NULL);
}

void output(char* paper_file){
    ofstream f(paper_file);
    f << paper_num << ' ' << size << endl;
    for (int i = 0; i < paper_num; ++i) {
        f << paper[i];
        for (int j =0; j < size; ++ j)
            f << ' ' << paper_vec[i][j];
        f << endl;
    }
    f.close();
}

void training(int t, char* dump) {
    real pre_pos = 100;
    real pre_neg = 1.0;
    boost::random::mt19937 gen(419);
    string str(dump);
    str = str + ".word";
    char* word_file = new char[str.length()+1];
    strcpy(word_file, str.c_str());
    ofstream f(word_file);
    f << word_num << ' ' << size << endl;
    for (int i = 0; i < word_num; ++i) {
        f << word[i];
        for (int j =0; j < size; ++ j)
            f << ' ' << word_vec[i][j];
        f << endl;
    }
    f.close();

    for (int iter = 0; iter < t; ++iter) {
        pos_sigmod = 0.0; num_pos = 0.0;
        neg_sigmod = 0.0; num_neg = 0.0;
        pthread_t *pt = (pthread_t *)malloc(num_thread * sizeof(pthread_t));
        int each = train_size / num_thread;
        for (int i = 0; i < num_thread; ++i) {
            thread_data_array[i].id = gen();
            thread_data_array[i].start = i * each;
            thread_data_array[i].end = (i + 1) * each;
        }
        thread_data_array[num_thread-1].end = train_size;

        for (int i = 0; i < num_thread; ++i)
            pthread_create(&pt[i], NULL, prepareIndex, (void *)&thread_data_array[i]);
        for (int i = 0; i < num_thread; ++i)
            pthread_join(pt[i], NULL); 
        pos_sigmod /= num_pos;
        neg_sigmod /= num_neg;
        printf("Iteration %d: rate = %f, pos_sigmod = %f, neg_sigmod = %e\n", iter, alpha, pos_sigmod, neg_sigmod);

        pre_pos = pos_sigmod;
        pre_neg = neg_sigmod; 

        if (iter % 10 == 0) {
            printf("Dumping %d \n", iter);        
            str = string(dump);
            str = str +  ".paper." + to_string((long long)iter);
            char* paper_file = new char[str.length()+1];
            strcpy(paper_file, str.c_str());
            output(paper_file);
        }
    }
}

int main(int argc, char **argv) {
    //input
    if (argc < 6) {
        printf("Useage: argv[1] = word_vec,  argv[2]=df, argv[3] = traindata, argv[4] = title, argv[5] = ctm file,  argv[6] = outfile\n");
        exit(-1);
    }
    loadWord(argv[1], argv[2]);
    loadTitle(argv[4]);
    loadTrainData(argv[3]);
    loadMT(argv[5]);
    // Precompute f(x) = e^x / (e^x + 1)
    expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    for (int i = 0; i < EXP_TABLE_SIZE; i++) {
        expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); 
        expTable[i] = expTable[i] / (expTable[i] + 1); 
    }
    //training
    training(1000, argv[6]);
}
