from mitmproxy import ctx


def request(flow):
    request = flow.request
    flow.request.headers['User-Agent'] = 'MitmProxy'
    ctx.log.info(str(request.headers))
