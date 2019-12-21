import pymongo

client = pymongo.MongoClient(host='localhost', port=27017)
db = client.test
collection = db.students
result = collection.remove({'name': 'Kevin'})
print(result)