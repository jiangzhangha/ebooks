import os

# 平台
PLATFORM = 'Android'

# 设备名称 通过 adb devices -l 获取
DEVICE_NAME = 'MI_NOTE_Pro'

# APP路径
APP = os.path.abspath('.') + 'jd/.apk'

# Appium地址
DRIVER_SERVER = 'http://localhost:4723/wd/hub'
# 等待元素加载时间
TIMEOUT = 300

# 滑动点
FLICK_START_X = 300
FLICK_START_Y = 300
FLICK_DISTANCE = 700


# 滑动间隔
SCROLL_SLEEP_TIME = 1

KEYWORD = '手机'