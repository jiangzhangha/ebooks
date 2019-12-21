import requests

r = requests.get("https://www.zhihu.com/explore")
print(r.text)