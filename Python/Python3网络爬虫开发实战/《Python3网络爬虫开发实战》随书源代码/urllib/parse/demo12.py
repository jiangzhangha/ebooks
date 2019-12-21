from urllib.parse import parse_qs

query = 'name=germey&age=22'
print(parse_qs(query))