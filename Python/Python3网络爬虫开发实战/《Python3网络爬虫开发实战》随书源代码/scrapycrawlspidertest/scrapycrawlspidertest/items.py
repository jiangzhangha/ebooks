# -*- coding: utf-8 -*-

# Define here the models for your scraped items
#
# See documentation in:
# http://doc.scrapy.org/en/latest/topics/items.html

from scrapy import Field, Item


class NewsItem(Item):
    title = Field()
    text = Field()
    datetime = Field()
    source = Field()
    url = Field()
    website = Field()
