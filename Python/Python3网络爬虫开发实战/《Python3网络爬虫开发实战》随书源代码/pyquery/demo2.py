from pyquery import PyQuery as pq
doc = pq(url='http://cuiqingcai.com')
print(doc('title'))