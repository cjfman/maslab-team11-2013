import serial
import sys, time
import random

from CVCom import CVCom
from Ball import Ball

#Computer Vision Info
cv_host = "localhost"
cv_port = 5556

#Codes
cStatus = "201"
cRightLimit = "202"
cLeftLimit = "203"
cIR1 = "211"
cGyroX = "220"
cGyroY = "221"
cGyroZ = "222"
cClearDistance = "230"
cForwardSpeed = "301" # speed between 0 255
cRightSpeed = "302"   # speed between 0 255
cLeftSpeed = "303"    # speed between 0 255
cAutoStop = "304"
cMoveDistance = "305" # inches
cTurn = "306"         # degrees

#States
sStart = "start"
sBallDemo = "ballDemo"
sLookInCircle = "lookInCircle"
sWander = "wander"
sTrackBall = "trackBall"
sEvadeFront = "evadeFront" # unimplemented
sEvadeRight = "evadeRight" # unimplemented
sEvadeLeft = "evadeLeft" # unimplemented

#Keys
kRightLimit = "rightLimit"
kLeftLimit = "leftLimit"
kBalls = "balls"
kBusy = "busy"
kIR1 = "ir1"

#Thresholds
ball_proximity_th = 50
x=320
y=240

#Speed Constants
forward_speed = 75
turn_speed = 65

##def poll():
##    try:
##        f = open("coord",'r');
##        val = f.read()
##        print "File: " + val
##        f.close();
##        return int(val)
##    except:
##        print sys.exc_info()[1]
##        return None

class ArduinoResetError(Exception):
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return repr(self.value)

class Master:

    def __init__(self, port_name = None, baud = 9600, x = 0, y = 0):
        self.baud = baud
        self.port = None
        self.state = sStart
        self.x = x
        self.y = y
        self.port_name = port_name
        self.cv = CVCom(cv_host, cv_port)
        self.timer = None
        self.timeout = None
        self.last_action = None
        self.hold_flag = False
            
    def connect(self):
            print "Connecting"
            #if self.port: self.close()
            # Loop through possible values of ACMX, and try to connect on each one
            for i in range(6):
                try:
                    # Try to create the serial connection
                    if self.port_name:
                        port_name = self.port_name
                    
                    elif i == 4:
                        port_name = "/dev/tty.usbmodemfd131"
                            
                    elif i == 5:
                        port_name = "/dev/tty.usbmodemfa141"
                    
                    else:
                        port_name = '/dev/ttyACM{0}'.format(i)
                
                    self.port=serial.Serial(port=port_name, baudrate=self.baud, timeout=1)
                    if self.port.isOpen():
                        time.sleep(2) # Wait for Arduino to initialize
                        print "Connected on %s"%(port_name)
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
        w_message = code + ":"
        if parameter != None:
            w_message += str(parameter)

        self.write(w_message)
        
        message = self.read()
        if "404:" in message:
            print "Send: " + w_message
            print "Receive: " + message
    
        if not message or "000:" in message or "100:" in message:
            print "Set hold flag"
            self.hold_flag = True
            raise ArduinoResetError(message)
        
        return (code, message[4:-1])
    
        
    def checkBoolean(self, code):
            r_code, r_message = self.sendCommand(code)
            return r_message == '1'
                
    def checkInt(self, code):
        r_code, r_message = self.sendCommand(code)
        
        try:
            return int(r_message)

        except:
            print sys.exc_info()[1]
            return -1

    def checkFloat(self, code):
        r_code, r_message = self.sendCommand(code)
        try:
            return float(r_message)

        except:
            sys.exc_info()[1]
            return -1

    def forward(self, speed = forward_speed):
        self.sendCommand(cForwardSpeed, speed)

    def turnRight(self, speed = turn_speed):
        self.sendCommand(cRightSpeed, -1*turn_speed)
        self.sendCommand(cLeftSpeed, turn_speed)

    def turnLeft(self, speed = turn_speed):
        self.sendCommand(cRightSpeed, turn_speed)
        self.sendCommand(cLeftSpeed, -1*turn_speed)

    def sensorCheck(self, input, frontEn = True):
        right = input[kRightLimit]
        left = input[kLeftLimit]
        both = right and left
        
        front_IR = (input[kIR1] < 6) and input[kIR1] != -1 and frontEn
        
        print input[kIR1]
        
        evade = right \
            or left \
            or front_IR
        
        if evade:
            self.forward(0)
            self.sendCommand(cClearDistance)
            if front_IR:
                print "Front IR tripped: " + str(input[kIR1])
                return sEvadeFront
            
            if both:
                print "Both limit switches tripped"
                return sEvadeFront
                
            if right:
                print "Right limit switch tripped"
                return sEvadeRight

            if left:
                print "Left limit switch tripped"
                return sEvadeLeft

        else:
            return None

    def ballDemoState(self, input):
        #print "Ball Position: " + str(input[kBalls])
        if input[kBalls]:
            #print "Demo: Found Ball"
            ball = Ball.closestPrimary()
            diff = ball.x - self.x
            #print "Position Difference: " + str(diff)
            if abs(diff) < (ball_proximity_th + ball.radius):
                self.sendCommand(cForwardSpeed, forward_speed)

            elif diff > 0:
                self.turnRight()

            else:
                self.turnLeft()

        else:
            #print "Demo: Searching"
            self.turnRight()

        return self.state

    def startState(self, input):
        return sLookInCircle
    
    def lookInCircleState(self, input):
        evade = self.sensorCheck(input)
        if evade:
            return evade
                
        if input[kBalls]:
            return sTrackBall
    
        if not self.timer:
            self.timer = time.time()
            self.timeout = 5
    
        elif time.time() - self.timer > self.timeout:
            return sWander

        self.turnRight()
        return self.state

    def trackBallState(self, input):
        evade = self.sensorCheck(input, frontEn = False)
        if evade:
            return evade
    
        if input[kBalls]:
            #print "Demo: Found Ball"
            ball = Ball.closestPrimary()
            if not ball:
                ball = Ball.closestSecondary()
            
            diff = ball.x - self.x
            #print "Position Difference: " + str(diff)
            if abs(diff) < (ball_proximity_th + ball.radius):
                self.sendCommand(cForwardSpeed, forward_speed)

            elif diff > 0:
                self.turnRight()

            else:
                self.turnLeft()

            return self.state

        else:
            return sWander

    def wanderState(self, input):
        evade = self.sensorCheck(input)
        if evade:
            return evade
        
        if input[kBalls]:
            return sTrackBall

        if not self.timer or time.time() - self.timer > self.timeout:
            action = random.randint(0, 1)
            
            if self.last_action == "turn":
                print "Wander Forward"
                self.sendCommand(cForwardSpeed, forward_speed)
                self.last_action = "forward"
                
            else:
                self.last_action = "turn"
                if action == 0:
                    print "Wander Right"
                    self.turnRight()
                    
                elif action == 1:
                    print "Wander Left"
                    self.turnLeft()
            
                else:
                    print "Wander Default"
                    self.sendCommand(cForwardSpeed, forward_speed)
            
            self.timer = time.time()
            self.timeout = random.randint(1, 5)

        return self.state

    def evadeFrontState(self, input):
        return self.state

    def evadeRightState(self, input):
        return self.state
        
    def evadeLeftState(self, input):
        return self.state


    def nextState(self, input):
        state = self.state
        if state == sStart: state = self.startState(input)
        elif state == sBallDemo: state = self.ballDemoState(input)
        elif state == sLookInCircle: state = self.lookInCircleState(input)
        elif state == sWander: state = self.wanderState(input)
        elif state == sTrackBall: state = self.trackBallState(input)
        elif state == sEvadeFront: state = self.evadeFrontState(input)
        elif state == sEvadeRight: state = self.evadeRightState(input)
        elif state == sEvadeLeft: state = self.evadeLeftState(input)
        else:
            print "State '%s' does not exist"%(self.state)
            state = sStart

        if state != self.state:
            self.timer = None
            self.last_action = None
            print "New State: " + state

        return state

    def run(self):
        if not self.connect(): return
        #run CV thread
        print "Start Handshake"
        while not "100:" in self.port.readline(): pass
        self.write("100")
        while not "101:" in self.port.readline(): pass
        print "Starting main loop"
        while True:
            try:
                if self.hold_flag:
                    print "waitin for ready signal"
                    while not "100:" in self.port.readline(): pass
                    print "Ready Signal Received"
                    self.hold_flag = False
                
                input = {}

                # Check Busy Status
                #input[kBusy] = self.checkBoolean(cStatus)
                
                # Check Limit Switches
                input[kRightLimit] = self.checkBoolean(cRightLimit)
                input[kLeftLimit] = self.checkBoolean(cLeftLimit)

                # Check IR Sensors
                input[kIR1] = self.checkFloat(cIR1)

                # Check Image Vision
                input[kBalls] = False #self.cv.getBalls()

                self.state = self.nextState(input)
                ##time.sleep(.25)
                #print "..."

            except (ArduinoResetError):
                pass

master = Master(None, 115200, x, y)
master.run()
