# NPM

Experiment code for the AAAI'15 paper: 

A Neural Probabilistic Model for Context Based Citation Recommendation

Please note that the code is experimental, so it contains two main part:

learning paper embeddings and calculate score (indexing)


# Raw data

The unprocessed data (SQL data) about the citation context and the cited papers are in:
https://psu.box.com/v/refseer


* You are welcome to use the code under the terms of the license, however please acknowledge its use by citation: 
W. Huang, Z. Wu, C. Liang, P. Mitra, and C. Lee Giles. A Neural Probabilistic Model for Context Based Citation Recommendation. In the Twenty-Ninth AAAI Conference on Artificial Intelligence (AAAI'15), 2015.

* Instruction:
The shared data is a SQL dump of citeseerx database with 3 tables: citations, citationContexts, and papers.
  * Important fields of table papers:
    1. id: each pdf will have a different id, this id is referred to as paperid in table citations;
    2. cluster: same paper (may be have more pdfs in our databases) will have a unique cluster number.
  * Important fields of table citations:
    1. id: this id is referred to as citationid in table citationContexts;
    2. cluster: the cluster number of the cited document;
    3. paperid: the id of citing document.
  * Important fields of table citationContexts:
    1. citationid: link to the citations table.
    2. context: citation contexts, citations are surrounded by =-= and -=-.




* Please use MySQL to import the data, I was told that there were some problems when importing 'citationContexts.sql' to Postgres.

* After the database is imported:
These are the steps that may help you:
  * create new data format, remove citations (surrounded by -=-  and =-=) :
  CitationContext      Cluster  (cited paper) 

  * learn word embedding  from citation context 

  * learn paper embedding  from citation context (initial paper embedding)

learn word embedding and paper embedding simultaneously. (when learn paper embedding only use adj. and noun. words in citation context )   

when learning paper embeddings, I assigned a normalized weight for each noun and adj word in an context

For example,
For one pair of citation and citation context:

w_1, w_2, ... , w_{n-1}, w_{n}              p_i

when learning embedding of paper p_i , word w_1 ,w_2... w_{n} has different learning weight.
I use the co-occurrence of word and paper in the whole corpus as weight. 


Should you have more questions, please email me at gmail start with harrywy 

# License
All codes are under Penn State ownership and is licensed under a [reative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/). 

![Alt](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)


