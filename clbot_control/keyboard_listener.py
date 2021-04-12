#! /usr/bin/env python

# File used to record keyboard events on a Raspberry Pi and
# publish accordingly to a ROS topic

import sys
import rospy
from std_msgs.msg import Float32
from geometry_msgs.msg import Twist
import pygame
from collections import defaultdict

vel = .3
theta = 2

class KeyboardListener(object):
    """Class representing a keyboard listener"""

    def __init__(self):
        pygame.init()
        pygame.display.set_caption(u'Keyboard events')
        pygame.display.set_mode((400, 400))
        self.keys = defaultdict(bool)

    def check_key(self, key, direction):
        """Sets the dict based on what key is currently pressed
        Ignores any non-ASCII keys (e.g. arrow keys)"""
        try:
            char = chr(key)
        except:
            return
        self.keys[char] = direction

    def build_message(self):
        msg = Twist()
        if self.keys['w']: # Forward
            msg.linear.x = vel
        if self.keys['s']: # Back up
            msg.linear.x = vel * -1
        if self.keys['d']: # Turn right
            msg.angular.z = theta * -1
        if self.keys['a']: # Turn left
            msg.angular.z = theta
        return msg

    def listen(self, callback):
        """Listen for events to happen and passes the pressed character
        + an indicator of key press vs key release into the callback function"""

        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                self.check_key(event.key, True)
            elif event.type == pygame.KEYUP:
                self.check_key(event.key, False)
            elif event.type == pygame.QUIT:
                pygame.display.quit()
                pygame.quit()
                sys.exit()

        msg = self.build_message()
        callback(msg)

def keyboard_event_publisher():
    """Listens for keyboard events and publishes them to the specified topic"""
    key_capture = KeyboardListener()

    geo_twist_pub = rospy.Publisher('cmd_vel', Twist, queue_size = 10)
    rospy.init_node('keyboard_event_publisher', anonymous=True)
    rate = rospy.Rate(5)

    while not rospy.is_shutdown():
       key_capture.listen(geo_twist_pub.publish)
       rate.sleep()

if __name__ == '__main__':
    try:
        keyboard_event_publisher()
    except rospy.ROSInterruptException:
        pass
