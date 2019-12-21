import pymongo

client = pymongo.MongoClient(host='localhost', port=27017)
db = client.test
collection = db.students
results = collection.find({'age': 20})
print(results)
for result in results:
    print(result)