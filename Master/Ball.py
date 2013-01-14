import json

class Ball:
    primary = 'red'
    green_balls = []
    red_balls = []
    closest_red = None
    closest_green = None
    
    @staticmethod
    def add(ball):
        try:
            if ball.color == 'red':
                Ball.red_balls.append(ball)
                if Ball.closest_red == None \
                    or Ball.closest_red.diameter < ball.diameter:
                        Ball.closest_red = ball

            else:
                Ball.green_balls.append(ball)
                if Ball.closest_green == None \
                    or Ball.closest_green.diameter < ball.diameter:
                        Ball.closest_green = ball

        except:
            print sys.exc_info()[1]
            return

    @staticmethod
    def closestPrimary():
        if Ball.primary == 'red':
            return Ball.closest_red

        else:
            return Ball.closest_green

    @staticmethod
    def clear():
        Ball.green_balls = []
        Ball.red_balls = []
        Ball.closest_green = None
        Ball.closest_red = None

    @staticmethod
    def setPrimary(color):
        Ball.primary = color

    @staticmethod
    def exist():
        if len(red_balls) or len(green_balls):
            return True

        else:
            return False
    
    def __init__(self, info = {}):
        try:
            self.color = info['color']
            self.x = info['x']
            self.y = info['y']
            self.radius = info['diameter']/2

        except:
            print sys.exc_info()[1]
            return