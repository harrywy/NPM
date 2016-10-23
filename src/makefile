CC = g++
#The -Ofast might not work with older versions of gcc; in that case, use -O2
CFLAGS = -lm -pthread -O2 -march=native -Wall -std=c++0x

all: paper2vec index-compute citation-model citation-model-title citation-model-MT index-soft distance citation-model-MT-inverse index-soft-bayes index-soft-bayes-log \
	citation-model-MT-inverse_v2

paper2vec : paper2vec.cpp
	$(CC) paper2vec.cpp -o paper2vec $(CFLAGS)


citation-model-MT: citation-model-MT.cpp
	$(CC) citation-model-MT.cpp -o citation-model-MT $(CFLAGS)

citation-model-MT-inverse: citation-model-MT-inverse.cpp
	$(CC) citation-model-MT-inverse.cpp -o citation-model-MT-inverse $(CFLAGS)

index-compute : index-compute.cpp
	$(CC) index-compute.cpp -o index-compute $(CFLAGS)

index-soft: index-soft.cpp
	$(CC) index-soft.cpp -o index-soft $(CFLAGS)

index-soft-bayes: index-soft-bayes.cpp
	$(CC) index-soft-bayes.cpp -o index-soft-bayes $(CFLAGS)

index-soft-bayes-log: index-soft-bayes-log.cpp
	$(CC) index-soft-bayes-log.cpp -o index-soft-bayes-log $(CFLAGS)

distance: distance.cpp
	$(CC) distance.cpp -o distance $(CFLAGS)

clean:
	rm -rf paper2vec index-compute citation-model
