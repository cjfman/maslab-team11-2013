import serial

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
ball_proximity_th = 10

#Speed Constands
forward_speed = 100
turn_speed = 50


class Master()

    def __init__(self, baud = 9600, x = 0, y = 0):
        self.baud = baud
        self.port = None
        self.state = sBallDemo #sStart
        self.x = x
        self.y = y
            
    def connect(self):
            print "Connecting"
            #if self.port: self.close()
            # Loop through possible values of ACMX, and try to connect on each one
            for i in range(4):
                try:
                    # Try to create the serial connection
                    self.port=serial.Serial(port='/dev/ttyACM{0}'.format(i), baudrate=self.baud, timeout=0.5)
                    if self.port.isOpen():
                        time.sleep(2) # Wait for Arduino to initialize
                        print "Connected"
                        return True
                except:
                    # Some debugging prints
                    print "Arduino not connected on ACM{0}".format(i)
            print "Failed to connect"
            return False

    def write(self, message):
        self.port.write(message + '\n')

    def read(self):
        message = self.port.readline()
        if message:
            return (message[0:3], message[4:])

    def sendCommand (self, code, parameter = None)
        message = code + ":"
        if parameter != None:
            message += int(parameter)

        print self.port.readline()
        
    def checkBoolean(self, code)
        self.write(code)
        return True if self.read()[1] == '1' else False

    def turnRight(self, speed = turn_speed)
        self.command(cRightSpeed, -1*turn_speed)
        self.command(cLeftSpeed, turn_speed)

    def turnLeft(self, speed = turn_speed)
        self.command(cRightSpeed, turn_speed)
        self.command(cLeftSpeed, -1*turn_speed)

    def ballDemoState(self, input)
        if input[kRightLimit] or input[kLeftLimit]:
            pass
            sendCommand(kmoveDistance, -12);

        if input[balls]:
            diff = input[balls] - x
            if diff < ball_proximity_th:
                sendCommand(cForwardSpeed, forward_speed)

            else if diff > 0:
                self.turnRight()

            else:
                self.turnLeft()

        else:
            self.turnRight()

        return self.state

    def startState(self input)
        return self.state

    def nextState(self, input)
        state = self.state
        if state == sStart: state = self.startState(input)
        if state == sBallDemo: state = self.BallDemoState(input)
        else state = self.state

        if state != self.state:
            print "New State: " + state

        return state

    def run(self):
        self.connect()
        #run CV thread
        while true:
            input = {}

            # Check Busy Status
            input[kBusy] = checkBoolean(cStatus)
            
            # Check Limit Switches
            input[kRightLimit] = checkBoolean(cRightLimit)
            input[kLeftLimit] = checkBoolean(cLeftLimit)

            # Check IR Sensors

            # Check Image Vision
            input[kBalls] = self.vision.areBalls()

            self.state = self.nextState(input) 
