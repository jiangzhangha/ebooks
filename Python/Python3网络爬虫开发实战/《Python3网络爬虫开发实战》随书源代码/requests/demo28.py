import requests

r = requests.get('http://localhost:5000', auth=('username', 'password'))
print(r.status_code)