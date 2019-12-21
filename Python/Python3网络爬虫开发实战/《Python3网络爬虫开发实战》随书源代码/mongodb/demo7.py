import pymongo

client = pymongo.MongoClient(host='localhost', port=27017)
db = client.test
collection = db.students
results = collection.find().sort('name', pymongo.ASCENDING).skip(2)
print([result['name'] for result in results])