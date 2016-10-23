#Learning:

```
./citation-model-MT-inverse ../data_tokenized/vec_tokenized_10_600_neg_100.txt ../data_tokenized/df_title ../data_tokenized/context_adj_noun ../data_tokenized/title new_ctm/ctm.model 600_inverse
```

## File standards:

* argv[1] = word_vec

This is the output from word2vec, use citation context to train the word2vec model. 


File should looks like this:

num_of_word dim_of_embedding
f1 f2 ... f_n
...


* argv[2] = df

This is the df file for words appear in citation context.

File should looks like:
word_0 df_0
...

* argv[3] = traindata

this is the citation context and cited document pair. 

File should looks like:
citing_document_id\tcited_document_id\tw_0 w_1 w_2 w_3 ... w_m\n
...

* argv[4] = title

This is deprecated, but you can test with title information.

File should looks like:
cited_document_id w_0 w_1 w_2 ... w_n\n

* argv[5] = ctm file

This is result of citation translation model.
word_0 document_id_0 translation_probability\n
...
or just use normalized co-occurrence 

This helps training


* argv[6] = outfile

name of outfile for example: 
{n_dim}_w2v


#Create indexing p(word|paper):

```
./index-compute 600_w2v.word.60 600_w2v.paper.60.cleaned test_voc ../data_tokenized/word.voc ../data_tokenized/paper.freq
./index-soft 600_w2v.word.60 600.paper.60.cleaned test_voc ../data_tokenized/word.voc ../data_tokenized/paper.freq
./index-soft-bayes 600_w2v.word.60 600.paper.60.cleaned test_voc ../data_tokenized/word.voc ../data_tokenized/paper.freq
```

* 600_w2v.word is word embedding
* 600.paper.60 is paper embeding 60 is iteration 
* test_voc is all words in test set
* word.voc is word2vec output vocabulary 
* paper.freq is paper frequency





