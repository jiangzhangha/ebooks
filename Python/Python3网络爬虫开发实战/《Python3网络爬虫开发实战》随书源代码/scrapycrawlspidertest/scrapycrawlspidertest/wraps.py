def list2str(list):
    """
    列表拼合成字符串
    :param result: 列表
    :return: 字符串
    """
    return ''.join(list)


def set_value(obj, value):
    """
    设置固定数值
    :param value: 数值
    :return: 数值
    """
    return value


def get_attr(obj, attr):
    """
    获取属性
    :param obj: 对象
    :param attr: 属性
    :return: 属性值
    """
    return getattr(obj, attr)
