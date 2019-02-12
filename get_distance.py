import math

data = (135.001,35.001,135.0,35.0)

def get_distance_rad(goal_x,goal_y,present_x,present_y):
    LATI = 91141.288#north 35
    LONG = 111319.49
    delta_x = (goal_x - present_x)*LONG
    delta_y = (goal_y - present_y)*LATI
    return (delta_x**2 + delta_y**2)**0.5,math.atan2(delta_y,delta_x)

print get_distance_rad(*data)
