# -*- coding: utf-8 -*-
import scrapy
from scrapy.linkextractors import LinkExtractor
from scrapy.spiders import CrawlSpider, Rule
from scrapycrawlspidertest.utils import get_config
from scrapycrawlspidertest.items import *
from scrapycrawlspidertest.wraps import *
from scrapycrawlspidertest import urls


class UniversalSpider(CrawlSpider):
    
    name = 'universal'
    
    def __init__(self, name, *args, **kwargs):
        config = get_config(name)
        self.config = config
        self.rules = eval(config.get('rules'))
        start_urls = config.get('start_urls')
        if start_urls:
            if start_urls.get('type') == 'static':
                self.start_urls = start_urls.get('value')
            elif start_urls.get('type') == 'dynamic':
                self.start_urls = list(eval('urls.' + start_urls.get('method'))(*start_urls.get('args', [])))
        self.allowed_domains = config.get('allowed_domains')
        super(UniversalSpider, self).__init__(*args, **kwargs)
    
    def parse_item(self, response):
        # 获取item配置
        item = self.config.get('item')
        if item:
            data = eval(item.get('class') + '()')
            # 动态获取属性配置
            for key, value in item.get('attrs').items():
                data[key] = response
                for process in value:
                    type = process.get('type', 'chain')
                    if type == 'chain':
                        # 动态调用函数和属性
                        if process.get('method'):
                            data[key] = getattr(data[key], process.get('method'))(*process.get('args', []))
                    elif type == 'wrap':
                        args = [data[key]] + process.get('args', [])
                        data[key] = eval(process.get('method'))(*args)
            yield data