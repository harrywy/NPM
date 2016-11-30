import MySQLdb
import datetime
import sys

conn = MySQLdb.connect(#add code here)
cursor = conn.cursor()

now = datetime.datetime.now()
w = open(now.strftime("%Y-%m-%d %H:%M"), 'w')

for i in range(27680, 92735362):
#for i in range(27680, 34646714):
#for i in range(27680, 27700):
    if i%20000==0:
        print i
    stmt = 'select citationid,context from citationContexts where id=\'' + str(i) + '\';'
    cursor.execute(stmt)
    tmp = cursor.fetchone()
    if tmp==None or len(tmp)!=2:
        continue
    context = tmp[1]
    if not "=-=" in context or not "-=-" in context:
        continue

    stmt = 'select cluster,paperid from citations where id=\'' + str(tmp[0]) + '\';'
    cursor.execute(stmt)
    tmp = cursor.fetchone()
    if tmp==None or len(tmp)!=2:
        continue
    cited = tmp[0]
    paperid = tmp[1]
    if cited==0:
        continue

    stmt = 'select ncites from papers where cluster=\'' + str(cited) + '\';'
    cursor.execute(stmt)
    tmp = cursor.fetchone()
    if tmp==None or tmp[0]<2:
        continue

    stmt = 'select cluster from papers where id=\'' + str(paperid) + '\';'
    cursor.execute(stmt)
    tmp = cursor.fetchone()
    if tmp==None:
        continue
    citing = tmp[0]
    if citing==0:
        continue
    print >> w, citing, '\t', cited, '\t', context
conn.close()
w.close()
print "================================END======================================="
