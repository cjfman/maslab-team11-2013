import socket
ip='127.0.0.1'
port=5567
message='Hai!'

s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((ip, port))
while (1):
    s.sendall(raw_input())
