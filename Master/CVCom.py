import socket
import sys
import time

from Ball import Ball

class CVCom:
    def __init__(self, host = "localhost", port = 5555):
        self.host = host
        self.port = port
        self.connected = False
        Ball.setPrimary('green')
        self.time = None
        self.timeout = None

    def connect(self):
        try:
            ip = socket.gethostbyname(self.host)
            self.connection = socket.socket() #(socket.AF_INET, socket.SOCK_STREAM)
            self.connection.connect((ip , self.port))
            self.connected = True
            return True
        

        except socket.error, msg:
            print 'Failed to create socket. Error code: ' + str(msg[0]) + ' , Error message : ' + msg[1]

        except socket.gaierror:
            print 'Hostname could not be resolved. Exiting'

        finally:
            return False

    def getBalls(self):
    
        if not self.time or self.time - time.time() > self.timeout:
            self.time = time.time()
            try:
                Ball.clear()
                self.connection.sendall("ball")
                json_data = self.recv_timeout(self.connection)
                if len(json_data) == 0 or json_data == "None":
                    return False
                
                i = json_data.index(':')
                dict = {}
                dict['color'] = 'red'
                dict['y'] = 0
                dict['x'] = json_data[i+1:]
                dict['width'] = json_data[:i]
                
                ##balls = json.loads(json_data)
                ##for ball in data:
                ##    Ball.add(Ball(ball))
                ##
                ##return Ball.exist()

            except:
                print sys.exc_info()[1]
                return False

        else:
            return Ball.exist()

    def recv_timeout(self, the_socket,timeout=2):
        the_socket.setblocking(0)
        total_data=[];data='';begin=time.time()
        while 1:
            #if you got some data, then break after wait sec
            if total_data and time.time()-begin>timeout:
                break
            #if you got no data at all, wait a little longer
            elif time.time()-begin>timeout*2:
                break
            try:
                data=the_socket.recv(8192)
                if data:
                    total_data.append(data)
                    begin=time.time()
                else:
                    time.sleep(0.1)
            except:
                pass

        return ''.join(total_data)