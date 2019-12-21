from bson.objectid import ObjectId
import pymongo

client = pymongo.MongoClient(host='localhost', port=27017)
db = client.test
collection = db.students
collection.find({'_id': {'$gt': ObjectId('593278c815c2602678bb2b8d')}})

condition = {'name': 'Kevin'}
student = collection.find_one(condition)
student['age'] = 25
result = collection.update(condition, student)
print(result)