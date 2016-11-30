import sys
import re
input = open('context','r');
output = open('contextcleaned','w');

i = 0;
for line in input:
	i = i + 1
	if re.match('\d+\t\d+\t.*',line):
		strs = line.split('\t')		
		if (len(strs) >= 3):
			context = strs[2]
			for j in range(3,len(strs)):
				context = context + " " + strs[j]
		else:
			print i, ': data format error 1: less than 2 tabs.'
			
		context = context.strip()
		first = context.find(' ')
		last = context.rfind(' ')
		c = context[first:last].strip()
		c = c.replace("-=-","")
		c = c.replace("=-=","")
		c = re.sub("\[[^\]]*\]","",c)
		c = re.sub("()","",c)
        c = c.strip()
		if len(c.split(' ')) < 20:
			continue
		output.write(strs[0] + '\t' + strs[1] + '\t' + c + '\n')
	else:
		print i, ': Error !!!!!'
		break
input.close();
output.close();
