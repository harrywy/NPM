import sys
import re

ADJ="JJ|JJR|JJS"
NOUN="NN|NNS|NNP|NNPS"
ADV="RB|RBR|RBS"
VERB="VB|VBD|VBG|VBN|VBP|VBZ"
ADD="PRP|PRP$|RP"

input = open('contexttagged', 'r')
output = open('contextshorten', 'w')
outfile = open('data.w2v', 'w')

while True:
    line = input.readline().strip()
    if not line:
        break
    before = line
    
    context = input.readline().strip().lower()
    tag = input.readline().strip()
    word = context.split(' ')
    tags = tag.split(' ')
    ret = ""
    for i in range(0, len(tags)):
        if re.match(ADJ+"|"+NOUN, tags[i]):
            w = word[i]#.replace("\\/","/")
            pattern = re.compile("[`~!@#$%^&*()=|{}':;',\\[\\].<>?]")
            result = pattern.search(w);
            if len(w)>=2 and not result:
                ret = ret + ' ' +  w
    ret = ret.strip()
    if len(ret) == 0:
        print before, context
    else:
        output.write(before + '\t')
        output.write(ret + '\n')
        outfile.write(context + '\n')

outfile.close()
input.close()
output.close()
	
			
