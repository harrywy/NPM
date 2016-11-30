source = open('Source', 'w')
target = open('Target', 'w')

with open('contextshorten') as infile:
    for line in infile:
        tmp = line.split('\t')
        source.write(tmp[2])
        target.write(tmp[1] + '\n')
 
source.close()
target.close()
