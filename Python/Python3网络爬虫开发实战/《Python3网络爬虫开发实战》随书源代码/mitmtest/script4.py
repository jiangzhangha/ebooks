from mitmproxy import ctx

def response(flow):
    print('Url', flow.request.url)
    response = flow.response
    info = ctx.log.info
    info(str(response.status_code))
    info(str(response.headers))
    info(str(response.cookies))
    info(str(response.text))