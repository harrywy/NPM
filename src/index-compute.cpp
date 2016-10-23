#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <map>
#include <math.h>
#include <unordered_set>
#include <pthread.h>
#include <vector>
#define N 10
using namespace std;

const int num_thread = 24;

string *word, *paper;
long long num_word, num_paper, size;
double **word_vec, **paper_vec;
string **model_index;
double **model_score;

unordered_set<string> word_set;

void loadWordSet(char* word_file){
    ifstream f(word_file);
    string str;
    while (getline(f, str)) {
        word_set.insert(str);
    }
    f.close();
    printf("Num: %d:\n", word_set.size());
}

void init(char* word_file, char* paper_file){
    ifstream f(word_file);
    f >> num_word >> size;
    word_vec = new double*[num_word];
    word = new string[num_word];
    string str;
    int idx =0;
    for (int i = 0; i < num_word; ++i) {
        f >> str;
        if (word_set.count(str)) {
            word_vec[idx] = new double[size];
            word[idx] = str;
            double len = 0.0;
            for (int j = 0; j < size; j++) {
                f >> word_vec[idx][j];
                len += word_vec[idx][j] * word_vec[idx][j];
            }
            len = sqrt(len);
            //cout << len << endl;
            for (int j= 0; j < size; j++) word_vec[idx][j] /= len;
            idx++;
        } else {
            double tmp;
            for (int j = 0; j < size; ++j) {
                f >> tmp;
            }
        }
    }
    f.close();
    num_word = idx;

    f.open(paper_file);
    f >> num_paper >> size;
    paper_vec = new double*[num_paper];
    paper = new string[num_paper];
    for (int i = 0; i < num_paper; ++i) {
        paper_vec[i] = new double[size];
        f >> paper[i];
        double len = 0.0;
        for (int j = 0; j < size; j++) {
            f >> paper_vec[i][j];
            len += paper_vec[i][j] * paper_vec[i][j];
        }
        len = sqrt(len);
        //cout << len << endl;
        for (int j= 0; j < size; j++) paper_vec[i][j] /= len;
    }
    f.close();

    model_index = new string*[num_word];
    model_score = new double*[num_word];
    for (int i = 0; i < num_word; ++i) {
        model_index[i] = new string[N];
        model_score[i] = new double[N];
    }
    printf("%d words and %d papers\n", num_word, num_paper);
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
    printf("Thread %d process words from %d to %d\n", args->id, start, end);
    for (int i = start; i < end; ++i) {
        if (i%1000==0) printf("Thread %d processing %d words\n", args->id, i);
        for (int j = 0; j < N; ++j) {
            model_index[i][j] = "";
            model_score[i][j] = -1.0;
        }
        for (int j = 0; j < num_paper; ++j) {
            double dist = 0.0;
            for (int s = 0;  s < size; ++s)
                dist += word_vec[i][s] * paper_vec[j][s];
			/*
			if (dist > 1) {
				printf("dist %f:\n", dist);
				cout << word[i];
				for (int s = 0;  s < size; ++s) 
					printf("%f ", word_vec[i][s]);
				cout <<  paper[j];
				for (int  s = 0;  s < size; ++s)
                    printf("%f ", paper_vec[j][s]);
				int tmp;
				cin >> tmp;
			}*/
            for (int k = 0; k < N; ++k) {
                if (dist > model_score[i][k]) {
                    for (int l = N -1; l > k; --l) {
                        model_score[i][l] = model_score[i][l-1];
                        model_index[i][l] = model_index[i][l-1];
                    }
                    model_score[i][k] = dist;
                    model_index[i][k] = paper[j];
                    break;
                }
            }
        }
    }
    pthread_exit(NULL);
}

void output(){
    ofstream f;
    f.open("../index/w2v.index");
    for (int i = 0; i < num_word; ++i) {
        for (int j = 0; j < N; ++j)
            f << i << ' ' <<  model_index[i][j] << ' '
                << model_score[i][j] << endl;
    }
    f.close();
    
    f.open("../index/w2v.voc");
    for (int i =0 ; i < num_word; ++i)
        f << i << ' ' <<  word[i] << endl;
    f.close();
}

int main(int argc, char **argv) {
    if (argc < 3 ) { 
        printf("Useage: argv[1] = 'word_file', argv[2] = 'paper_file', argv[3] = 'word_set'\n");
        exit(EXIT_FAILURE);
    }
    printf("loading wordset...\n");
    loadWordSet(argv[3]);
    printf("WordSet loaded!\n");
    init(argv[1], argv[2]);
    printf("Modle Loaded!\n");
    pthread_t *pt = (pthread_t *)malloc(num_thread * sizeof(pthread_t));
    int each = num_word / num_thread; 
    for (int i = 0; i < num_thread; ++i) {
        thread_data_array[i].id = i;
        thread_data_array[i].start = i * each;
        thread_data_array[i].end = (i + 1) * each;
    }
    thread_data_array[num_thread-1].end = num_word;
    
    for (int i = 0; i < num_thread; ++i)
        pthread_create(&pt[i], NULL, prepareIndex, (void *)&thread_data_array[i]);
    for (int i = 0; i < num_thread; ++i)
        pthread_join(pt[i], NULL);
    printf("Output\n");
    output();
}

