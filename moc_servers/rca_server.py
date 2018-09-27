import socket


sock = socket.socket()

sock.bind(('', 9099))
sock.listen(1)

while True:

    conn, adress = sock.accept()

    print('connected:', adress)

    while True:
        conn.send(input().encode())

        data = conn.recv(1024).decode()
        if not data:
            break
        print(data)
