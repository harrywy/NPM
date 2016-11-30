import sys
import re
f = open('context', 'w')
with open('92735362_2014-12-13') as infile:
    context = ""
    citing = ""
    cited = ""
    for i, line in enumerate(infile):
        if re.match('\d+ & \d+ & .*', line):
            if len(context) >= 100:
                f.write(citing + '\t' + cited + '\t'+  context + '\n')
            else:
                print i, context
            strs = line[:-1].split(" & ")
            if (len(strs) >= 3):
                citing = strs[0]
                cited = strs[1]
                context = strs[2]
                for j in range(3, len(strs)):
                    context = context + " & " + strs[j]
            else:
                print  i, ': data format error 1: less than 2 " & "s .'
        else:
            context = context + "fl" + line[:-1]
            

f.close()
