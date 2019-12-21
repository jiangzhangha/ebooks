import pymongo

client = pymongo.MongoClient(host='localhost', port=27017)
db = client.test
collection = db.students

result = collection.find_one({'name': 'Mike'})
print(type(result))
print(result)













