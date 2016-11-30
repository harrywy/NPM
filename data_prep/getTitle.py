import MySQLdb
import datetime
import sys

conn = MySQLdb.connect(#add code here)
cursor = conn.cursor()

w = open('title', 'w')
w2 = open('no_title', 'w')

with open('Target.vcb') as infile:
    for line in infile:
        c = line.split(' ')[1]
        stmt = 'select title from papers where cluster=\'' + str(c) + '\';'
        cursor.execute(stmt)
        tmp = cursor.fetchone()
        if tmp==None:
            print "Error!", c
            continue

        title = ""

        if len(tmp)> 1:
            print c, tmp

        for i in range(len(tmp)):
            if tmp[i]:
                title = title + ' ' + tmp[i]
        title = title.strip()

        if len(title)== 0:
            w2.write(c+'\n')
        else:
            w.write(c + ' ' + title + '\n')
conn.close()
w.close()
w2.close()
print "================================END======================================="
