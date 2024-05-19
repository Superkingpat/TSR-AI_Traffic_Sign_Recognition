import sys
import numpy as np
from numba import njit

@njit
def fside(pointA, pointB, point):
    determinanta = (pointB[0] - pointA[0]) * (point[1] - pointA[1]) - (point[0] - pointA[0]) * (pointB[1] - pointA[1])
    if determinanta > 0:
        return 1 #left
    elif determinanta < 0:
        return -1 #right
    else:
        return 0

def grahm_algorithm(points):
    startPoint = points[np.argmin(points[:, 0])]
    hull_points = [startPoint]
    angle_dict = {}

    for point in points:
        delta_x = point[0] - startPoint[0]
        delta_y = point[1] - startPoint[1]
        angle_dict[tuple(point)] = np.arctan2(delta_y, delta_x)

    angle_dict = {k: v for k, v in sorted(angle_dict.items(), key=lambda item: item[1], reverse=True)}
    pointNext = None
    for k in angle_dict.keys():
        if tuple(hull_points[0]) == k:
            continue
        if len(hull_points) < 2:
            hull_points.append(np.array(k))
            continue
        pointNext = np.array(k)
        side = fside(hull_points[-2],  hull_points[-1], pointNext)
        if side == -1 or side == 0:
            hull_points.append(pointNext)
        elif side == 1:
            while fside(hull_points[-2],  hull_points[-1], pointNext) == 1:
                hull_points.pop()
            hull_points.append(pointNext)
    return np.array(hull_points)