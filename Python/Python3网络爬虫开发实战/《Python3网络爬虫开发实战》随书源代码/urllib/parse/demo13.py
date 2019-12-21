from urllib.parse import parse_qsl

query = 'name=germey&age=22'
print(parse_qsl(query))