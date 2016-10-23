import sys

if __name__ == "__main__":
    oufile = open(sys.argv[1] + '.cleaned', 'w')
    oufile_0 = open('removed_paper', 'w')
    num = 0
    tmp = ' '.join(['0']*100)
    with open(sys.argv[1]) as infile:
        for line in infile:
            if not tmp in line:
                num += 1
    num = num-1
    with open(sys.argv[1]) as infile:
        total, dim = infile.readline().strip().split(' ')
        oufile.write(str(num) + ' ' + dim + '\n');
        for i in range(int(total)):
            line = infile.readline()
            if not tmp in line:
                oufile.write(line)
            else:
                oufile_0.write(line[:line.find(' ')] + '\n')

    oufile.close()
    oufile_0.close()
