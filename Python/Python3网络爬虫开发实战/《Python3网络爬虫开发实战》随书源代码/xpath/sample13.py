from lxml import etree

html = etree.parse('./test.html', etree.HTMLParser())
result = html.xpath('//ul/a')
print(result)
