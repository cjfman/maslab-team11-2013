import serial
import sys, time

#Codes
cStatus = "201"
cRightLimit = "202"
cLeftLimit = "203"
cForwardSpeed = "301" # speed between 0 255
cRightSpeed = "302"   # speed between 0 255
cLeftSpeed = "303"    # speed between 0 255
cAutoStop = "304"
cMoveDistance = "305" # inches
cTurn = "306"         # degrees

#States
sStart = "start"
sBallDemo = "ballDemo"

#Keys
kRightLimit = "rightLimit"
kLeftLimit = "leftLimit"
kBalls = "balls"
kBusy = "busy"

#Thresholds
ball_proximity_th = 50
x=320
y=240

#Speed Constants
forward_speed = 100
turn_speed = 200

def poll():
    try:
        f = open("coord",'r');
        val = f.read()
        print "File: " + val
        f.close();
        return int(val)
    except:
        print sys.exc_info()[1]
        return None

class Master:

    def __init__(self, port_name = None, baud = 9600, x = 0, y = 0):
        self.baud = baud
        self.port = None
        self.state = sBallDemo #sStart
        self.x = x
        self.y = y
        self.port_name = port_name
            
    def connect(self):
            print "Connecting"
            #if self.port: self.close()
            # Loop through possible values of ACMX, and try to connect on each one
            for i in range(4):
                try:
                    # Try to create the serial connection
                    if self.port_name:
                        port_name = self.port_name
                    
                    else:
                        port_name = '/dev/ttyACM{0}'.format(i)
                
                    self.port=serial.Serial(port=port_name, baudrate=self.baud, timeout=1)
                    if self.port.isOpen():
                        time.sleep(2) # Wait for Arduino to initialize
                        print "Connected"
                        return True
        
                except:
                    # Some debugging prints
                    print sys.exc_info()[1]
                    print "Arduino not connected on : " + port_name

            print "Failed to connect"
            return False

    def write(self, message):
        self.port.write(message + '\n')

    def read(self):
        return self.port.readline()[:-1]        

    def sendCommand(self, code, parameter = None):
        message = code + ":"
        if parameter != None:
            message += str(parameter)

        print "Send: " + message
        self.write(message)
                
        message = self.read()
        print "Receive: " + message
        if message:
            return (message[0:3], message[4:])
        
        else:
            return (None, None)
    
        
    def checkBoolean(self, code):
        r_code, r_message = self.sendCommand(code)
        return r_message == '1'

    def turnRight(self, speed = turn_speed):
        self.sendCommand(cRightSpeed, -1*turn_speed)
        self.sendCommand(cLeftSpeed, turn_speed)

    def turnLeft(self, speed = turn_speed):
        self.sendCommand(cRightSpeed, turn_speed)
        self.sendCommand(cLeftSpeed, -1*turn_speed)

    def ballDemoState(self, input):
        print "Ball Position: " + str(input[kBalls])
        if input[kBalls]:
            print "Demo: Found Ball"
            diff = input[kBalls] - self.x
            print "Position Difference: " + str(diff)
            if abs(diff) < ball_proximity_th:
                self.sendCommand(cForwardSpeed, forward_speed)

            elif diff > 0:
                self.turnRight()

            else:
                self.turnLeft()

        else:
            print "Demo: Searching"
            self.turnRight()

        return self.state

    def startState(self, input):
        return self.state

    def nextState(self, input):
        state = self.state
        if state == sStart: state = self.startState(input)
        elif state == sBallDemo: state = self.ballDemoState(input)
        else: state = self.state

        if state != self.state:
            print "New State: " + state

        return state

    def run(self):
        if not self.connect(): return
        #run CV thread
        print "waitin for ready signal"
        while not "100:" in self.port.readline(): pass
        print "starting main loop"
        while True:
            input = {}

            # Check Busy Status
            #input[kBusy] = self.checkBoolean(cStatus)
            
            # Check Limit Switches
            #input[kRightLimit] = self.checkBoolean(cRightLimit)
            #input[kLeftLimit] = self.checkBoolean(cLeftLimit)

            # Check IR Sensors

            # Check Image Vision
            input[kBalls] = poll() #self.vision.areBalls()

            self.state = self.nextState(input)
            time.sleep(1)
            print "..."

master = Master("/dev/ttyACM0", 115200, x, y)
master.run()
