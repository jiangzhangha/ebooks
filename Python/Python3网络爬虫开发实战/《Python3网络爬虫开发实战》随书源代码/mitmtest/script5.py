def tcp_message(flow):
    print('ssssss')
    print(flow.message)
    
    
def response(flow):
    print(flow.request.url)
    
    
def websocket_message(flow):
    print('sddsfsdfdfs')
    print(flow.message)