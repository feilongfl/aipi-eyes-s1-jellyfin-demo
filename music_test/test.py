import socket

# 1. 打开socket到192.168.10.109:8096
host = "192.168.10.109"
port = 8096

# 创建一个socket对象
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 连接到指定的主机和端口
s.connect((host, port))

# 2. 发送指定内容
data_to_send = "GET /Audio/84f7cdda408d432361c29d6bda9cbc27/universal?Container=pcm&TranscodingContainer=pcm&AudioCodec=pcm_s16le&audioBitRate=16000&api_key=d1e79804ea3b4215aecfc7d9a2043e23 HTTP/1.1\r\n"+\
"Host: 192.168.10.109:8096\r\n"+\
"Accept: */*\r\n"+\
"Cache-Control: no-cache\r\n"+\
"Connection: keep-alive\r\n"+\
"DNT: 1\r\n"+\
"Pragma: no-cache\r\n"+\
"Range: bytes=0-\r\n"+\
"User-Agent: feilong JellyfinBox\r\n"+\
"\r\n"

s.send(data_to_send.encode())  # 发送数据

# 3. 接收数据并保存到test.bin
with open("test.pcm", "wb") as file:
    while True:
        received_data = s.recv(1024)  # 接收数据
        if not received_data:
            break
        file.write(received_data)

# 关闭socket连接
s.close()
