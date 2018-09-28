import socket


sock = socket.socket()

sock.bind(('', 59002))
sock.listen(1)

while True:

    conn, adress = sock.accept()

    print('connected:', adress)

    while True:
        data = conn.recv(1024).decode()
        print(data)
        if not data:
            break

        ls = data.split()

        while ls:
            if ls[0] == '1':
                if len(ls) < 9:
                    break
                for i in range(1, 7):
                    ls[i] = ls[i][:-3]
                conn.send((f" {ls[1]}.000 {ls[2]}.000 {ls[3]}.000 {ls[4]}.000 {ls[5]}.000 \
                            {ls[6]}.000").encode())
                print('send:', f" {ls[1]}.000 {ls[2]}.000 {ls[3]}.000 {ls[4]}.000 {ls[5]}.000 \
                            {ls[6]}.000")
                if len(ls[8]) == 1:
                    ls = ls[9:]
                else:
                    ls = ls[8:]
                    ls[0] = ls[0][1:]
            elif ls[0] == '2':
                if len(ls) < 7:
                    break
                conn.send(b'YES')
                print('send:', 'YES')
                ls = ls[7:]
            else:
                ls = []