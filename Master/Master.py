import serial
import sys, time
import random

from CVCom import CVCom
from Ball import Ball

#Computer Vision Info
cv_host = "localhost"
cv_port = 5555

#Codes
cStatus = "201"
cRightLimit = "202"
cLeftLimit = "203"
cIR1 = "211"
cIR2 = "212"
cGyroX = "220"
cGyroY = "221"
cGyroZ = "222"
cHeading = "230"
cClearDistance = "230"
cForwardSpeed = "301" # speed between 0 255
cRightSpeed = "302"   # speed between 0 255
cLeftSpeed = "303"    # speed between 0 255
cAutoStop = "304"
cAngularVelocity = "305"
#cMoveDistance = "305" # inches
#cTurn = "306"         # degrees

#States
sStart = "start"
sBallDemo = "ballDemo"
sLookInCircle = "lookInCircle"
sWander = "wander"
sTrackBall = "trackBall"
sFindYellowWall = "findYellowWall"
sEvadeFront = "evadeFront" # unimplemented
sEvadeRight = "evadeRight" # unimplemented
sEvadeLeft = "evadeLeft" # unimplemented

#Actions
aForward = "forward"
aBackwards = "backwards"
aRight = "right"
aLeft = "left"
aTurn = "turn"

#Keys
kRightLimit = "rightLimit"
kLeftLimit = "leftLimit"
kBalls = "balls"
kBusy = "busy"
kIR1 = "ir1"
kIR2 = "ir2"
kHeading = "heading"
kYellow = "yellow"

#Thresholds
ball_proximity_th = 10
x=320
y=240

#Speed Constants
forward_speed = 125
turn_speed = 60

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
        self.start_time = None
        self.find_wall_time = None
        self.timer = None
        self.timeout = None
        self.last_action = None
        self.hold_flag = False
        self.start_heading = None
        self.next_heading = None
    
    
    ####################
    ## Serial Functions
    ####################
    
    def connect(self):
            print "Connecting"
            if self.port: self.port.close()
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
                    pass
                    # Some debugging prints
                    #print sys.exc_info()[1]
                    #print "Arduino not connected on : " + port_name

            print "Failed to connect"
            return False

    
    
    #####################
    ## Command Functions
    #####################
    
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
    
        if "000:" in message:
            print "Arduino restarted"
            self.hold_flag = True
            raise ArduinoResetError(message)
    
        if not message:
            print "No communication from Arduino"
            print "Reestablishing Connection"
            self.connect()
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


    ####################
    ## Motion Functions
    ####################
    
    def forward(self, speed = forward_speed, dist = None):
        const = 0.1
        if dist:
            dist = abs(dist)
            speed = forward_speed + int(speed * dist * const)
        
        self.sendCommand(cForwardSpeed, speed)
    
    def backwards(self, speed = forward_speed, dist = None):
        const = 0.1
        if dist:
            dist = abs(dist)
            speed = forward_speed + int(speed * dist * const)
        
        self.sendCommand(cForwardSpeed, -1 * speed)

    def turnRight(self, speed = turn_speed, diff = None):
        const = .1
        if diff:
            diff = abs(diff)
            speed = turn_speed + (speed * diff * const)
        
        self.sendCommand(cRightSpeed, -1*speed)
        self.sendCommand(cLeftSpeed, speed)

    def turnLeft(self, speed = turn_speed, diff = None):
        const = .1
        if diff:
            diff = abs(diff)
            speed = turn_speed + (speed * diff * const)
        
        self.sendCommand(cRightSpeed, speed)
        self.sendCommand(cLeftSpeed, -1*speed)

    def angularVelocity(speed, direction):
        self.sendCommand(cAngularVelocity, speed*direction)
    
    ####################
    ## Other
    ####################

    def sensorCheck(self, input, frontEn = True):
        right = input[kRightLimit]
        left = input[kLeftLimit]
        both = right and left
        
        right_ir = input[kIR1]
        left_ir = input[kIR2]
        
        front_IR = False #(input[kIR1] < 6) and input[kIR1] != -1 and frontEn
        
        #print input[kIR1]
        
        evade = right \
            or left \
            or front_IR \
            or (right_ir < 6 and right_ir >= 0) \
            or (left_ir < 6 and left_ir >= 0)
        
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

            if right_ir < 6:
                if left_ir < 7:
                    print "Both IR tripped: ", left_ir, right_ir
                    return sEvadeFront
                    
                else:
                    print "Right IR tripped: ", right_ir
                    return sEvadeRight
                    
            if left_ir < 6:
                if right_ir < 7:
                    print "Both IR tripped: ", left_ir, right_ir
                    return sEvadeFront
                    
                else:
                    print "Left IR tripped: ", left_ir
                    return sEvadeLeft

        else:
            return None

    def makeHeading(self, num):
        return num%360;
        
    def fastestDirection(self, current, next):
        if (next - current)%360 < 180:
            return aRight
            
        else:
            return aLeft
        

    ####################
    ## State Functions
    ####################

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
    
        heading = input[kHeading]
    
        if not self.next_heading:
            self.next_heading = self.makeHeading(heading + 350);
    
        elif abs(self.next_heading - heading) < 5:
            return sWander

        self.turnRight()
        return self.state

    def trackBallState(self, input):
        evade = self.sensorCheck(input, frontEn = False)
        if evade:
            return evade
    
        if time.time() > self.find_wall_time and input[kYellow]:
            return sFindYellowWall
    
        if input[kBalls]:
            #print "Demo: Found Ball"
            ##ball = Ball.closestPrimary()
            ##if not ball:
            ##    ball = Ball.closestSecondary()
            ##
            ##diff = ball.x - self.x
            #print "Position Difference: " + str(diff)
            ##if abs(diff) < (ball_proximity_th + ball.radius):
            
            radius = input[kBalls][1]
            ball_x = input[kBalls][0]
            self.last_ball_x = ball_x
            self.last_ball_radius = radius
            print ball_x, radius
            diff = ball_x - self.x
            if abs(diff) < (ball_proximity_th + radius):
                self.sendCommand(cForwardSpeed, forward_speed)

            elif diff > 0:
                #self.turnRight()
                self.angularVelocity(diff/10, 1)

            else:
                #self.turnLeft()
                self.angularVelocity(diff/10, 0)

            return self.state

        else:
            if abs(diff) < (ball_proximity_th + radius):
                self.sendCommand(cForwardSpeed, forward_speed)
                time.sleep(1)
            
            return sLookInCircle

    def findYellowState(self, input):
        evade = self.sensorCheck(input, frontEn = False)
        if evade:
            return evade
    
        if input[kYellow]:
            
            width = input[kBalls][1]
            wall_x = input[kBalls][0]
            print wall_x, width
            diff = wall_x - self.x
            if abs(diff) < (width):
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
        
        if time.time() > self.find_wall_time and input[kYellow]:
            return sFindYellowWall
        
        if input[kBalls]:
            return sTrackBall
            
        heading = input[kHeading]
        
        if not self.timer or time.time() - self.timer > self.timeout:
            action = random.randint(0, 1)
            
            if self.last_action == aTurn:
                print "Wander Forward"
                self.sendCommand(cForwardSpeed, forward_speed)
                self.last_action = aForward
                
            else:
                self.last_action = aTurn
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
        
        """if self.last_action == None:
            self.next_heading = self.makeHeading(heading + random.randint(90, 359))
            if self.fastestDirection(heading, self.next_heading) == aRight:
                print "Wander Right"
                self.turnRight()
            
            else:
                print "Wander Left"
                self.turnLeft()
            
            last_action = aTurn
            
        elif self.last_action == aTurn:
            if abs(heading - self.next_heading) < 10:
                print "Wander Forward"
                self.next_heading == None
                self.forward()
                self.last_action = aForward
                self.timer = time.time()
                self.timeout = random.randint(1, 5)
            
        elif self.last_action == aForward:
            if time.time() - self.timer > self.timeout:
                self.forward(0)
                self.last_action = None
"""
        return self.state

    ####################
    ## Evade Functions
    ####################

    def evadeFrontState(self, input):
        if not self.timer:
            self.timer = time.time()
            self.timeout = 1
    
        elif time.time() - self.timer > self.timeout:
            return sWander

        self.backwards()
        return self.state

    def evadeRightState(self, input):
        if not self.timer:
            self.backwards()
            self.timer = time.time()
            self.timeout = 1
            self.last_action = aBackwards
    
        elif time.time() - self.timer > self.timeout:
            if self.last_action == aTurn:
                self.forward(0)
                return sWander

            else:
                self.turnLeft()
                self.timer = time.time()
                self.timeout = 1
                self.last_action = aTurn
        
        return self.state
                
        
    def evadeLeftState(self, input):
        if not self.timer:
            self.backwards()
            self.timer = time.time()
            self.timeout = 1
            self.last_action = aBackwards
    
        elif time.time() - self.timer > self.timeout:
            if self.last_action == aTurn:
                self.forward(0)
                return sWander

            else:
                self.turnLeft()
                self.timer = time.time()
                self.timeout = 1
                self.last_action = aTurn
        
        return self.state

    ############################
    ## Next State and Main Loop
    ############################

    def nextState(self, input):
        state = self.state
        if state == sStart: state = self.startState(input)
        elif state == sBallDemo: state = self.ballDemoState(input)
        elif state == sLookInCircle: state = self.lookInCircleState(input)
        elif state == sWander: state = self.wanderState(input)
        elif state == sTrackBall: state = self.trackBallState(input)
	elif state == sFindYellowWall: state = self.findYellowState(input)
        elif state == sEvadeFront: state = self.evadeFrontState(input)
        elif state == sEvadeRight: state = self.evadeRightState(input)
        elif state == sEvadeLeft: state = self.evadeLeftState(input)
        else:
            print "State '%s' does not exist"%(self.state)
            state = sStart

        if state != self.state:
            self.timer = None
            self.last_action = None
            self.next_heading = None
            print "New State: " + state

        return state

    def run(self):
        if not self.connect(): return
        #run CV thread
        self.cv.connect()
        
        # Start communication with arduino
        print "Start Handshake"
        while not "100:" in self.port.readline(): pass
        self.write("100")
        while not "101:" in self.port.readline(): pass
        
        # Main Loop
        self.start_time = time.time()
        self.find_wall_time = self.start_time + 180
        print "Starting main loop"
        while True:
            try:
                if self.hold_flag:
                    print "Redo handshake"
                    while not "100:" in self.port.readline(): pass
                    self.write("100")
                    while not "101:" in self.port.readline(): pass
                    print "Handshake complete"
                    self.hold_flag = False
                
                input = {}

                # Check Busy Status
                input[kBusy] = self.checkBoolean(cStatus)
                
                # Check Limit Switches
                input[kRightLimit] = self.checkBoolean(cRightLimit)
                input[kLeftLimit] = self.checkBoolean(cLeftLimit)

                # Check IR Sensors
                input[kIR1] = self.checkFloat(cIR1)
                input[kIR2] = self.checkFloat(cIR2)

                # Check Image Vision
                input[kBalls] = self.cv.getBalls()
                #input[kYellow] = self.cv.getYellowWall()
            
                # Check Heading
                input [kHeading] = self.checkInt(cHeading)

                self.state = self.nextState(input)
                ##time.sleep(.25)
                ##print "..."

            except (ArduinoResetError):
                pass

master = Master(None, 115200, x, y)
master.run()

