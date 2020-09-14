#!/usr/bin/env python2
import socket, time, math
import numpy as np

UDPHOST="flipdot.lab"
UDPPORT=2323

SIZE_Y = 16
SIZE_X = 80
FPS = 13
SPEED = 0.8

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send(image):
    msg = '';
    pieces = '';
    for line in image:
        pieces += ''.join(str(x) for x in line)

    pieces = [pieces[i:i+8] for i in range(0, len(pieces), 8)]

    for i in pieces:
        if (len(i) < 8):
            i = i.ljust(8, '1')
        msg += chr(int(str(i), 2))

    sock.sendto(msg, (UDPHOST, UDPPORT))

# clear screen
# params: pixel (1|0)
# return: screen map
def blank(c=0):
    return [ [ c for x in range(SIZE_X) ] for y in range(SIZE_Y) ]

# projecting vec3 to 2d-viewplane
# params: vec3
# return: vec2
def projectz(v):
    return [ v[0], v[1], v[2] ]
    #return [ v[0]/(1+0.05*v[2]), v[1]/(1+0.05*v[2]), v[2] ]

# calc slope between points
# params: vec2, vec2
# return: slope, dx, dy
def slope(v1, v2):
    dx = v2[0] - v1[0]
    dy = v2[1] - v1[1]
    dz = v2[2] - v1[2]
    if dx==0:
        s = 0
    else:
        s = dy/dx
    return s, dx, dy, math.ceil(math.sqrt(dx*dx+dy*dy)), dz

# generate list of indices to draw to
# params: list of 2 vec3
# return: list of indices
def rasterize_line(line):
    v1 = projectz(line[0])
    v2 = projectz(line[1])
    s, dx, dy, l, dz = slope(v1, v2)
    if (l>0):
        dx = dx/l
        dy = dy/l
        dz = dz/l
    else:
        dx = 1
        dy = 1
        dz = 1
    return [ [ int(math.ceil(v1[0] + dx*step + SIZE_X/2)), int(math.ceil(v1[1] + dy*step + SIZE_Y/2)), v1[2] + dz*step ] for step in range(int(l)) ]

# return: vlist for triangle
def triangle(a=1.0):
    vertexlist = np.array([
        np.array([ a*np.array([ -0.5, -0.25, 0.0 ]), a*np.array([ 0.0, 0.75, 0.0 ]) ]),
        np.array([ a*np.array([ -0.5, -0.25, 0.0 ]), a*np.array([ +0.5, -0.25, 0.0 ]) ]),
        np.array([ a*np.array([ +0.5, -0.25, 0.0 ]), a*np.array([ 0.0, 0.75, 0.0 ]) ])
    ])
    return vertexlist.flatten().reshape(3,2,3)

# params: n (normal)
# return: vlist for square
def square(n, a = 1.0):
    if n == "u":
        vertexlist = np.array([
            np.array([ a*np.array([ +1.0, +1.0, +1.0 ]), a*np.array([ +1.0, +1.0, -1.0 ]) ]),
            np.array([ a*np.array([ +1.0, +1.0, -1.0 ]), a*np.array([ -1.0, +1.0, -1.0 ]) ]),
            np.array([ a*np.array([ -1.0, +1.0, -1.0 ]), a*np.array([ -1.0, +1.0, +1.0 ]) ]),
            np.array([ a*np.array([ -1.0, +1.0, +1.0 ]), a*np.array([ +1.0, +1.0, +1.0 ]) ])
        ])
    if n == "d":
        vertexlist = np.array([
            np.array([ a*np.array([ +1.0, -1.0, +1.0 ]), a*np.array([ +1.0, -1.0, -1.0 ]) ]),
            np.array([ a*np.array([ +1.0, -1.0, -1.0 ]), a*np.array([ -1.0, -1.0, -1.0 ]) ]),
            np.array([ a*np.array([ -1.0, -1.0, -1.0 ]), a*np.array([ -1.0, -1.0, +1.0 ]) ]),
            np.array([ a*np.array([ -1.0, -1.0, +1.0 ]), a*np.array([ +1.0, -1.0, +1.0 ]) ])
        ])
    return vertexlist.flatten().reshape(4,2,3)

# return: vlist for cube
def cube(a):
    l1, l2, l3, l4 = square("u", a)
    l5, l6, l7, l8 = square("d", a)
    vertexlist = np.array([
        l1, l2, l3, l4, l5, l6, l7, l8,
        np.array([ a*np.array([ +1.0, +1.0, +1.0]), a*np.array([ +1.0, -1.0, +1.0 ]) ]),
        np.array([ a*np.array([ +1.0, +1.0, -1.0]), a*np.array([ +1.0, -1.0, -1.0 ]) ]),
        np.array([ a*np.array([ -1.0, +1.0, -1.0]), a*np.array([ -1.0, -1.0, -1.0 ]) ]),
        np.array([ a*np.array([ -1.0, +1.0, +1.0]), a*np.array([ -1.0, -1.0, +1.0 ]) ])
    ])
    return vertexlist.flatten().reshape(12,2,3)

# filter vertices from vlist via (bad) depthtest
# delete the 3 lines with highest z-value
# z increases with depth
# so highest z value is away furthest from viewplane
def depthtest(vlist):
    vlist = vlist.flatten().reshape(12,6)
    vlist.view('f8,f8,f8,f8,f8,f8').sort(order=['f2'],axis=0)
    if (vlist[11][2] == vlist[10][2]):
        vlist = np.resize(vlist, (10,6))
    else:
        vlist = np.resize(vlist, (11,6))
    vlist.view('f8,f8,f8,f8,f8,f8').sort(order=['f5'],axis=0)
    return np.resize(vlist, (9,2,3))

# params: alpha (around x), beta (around y), gamma (around z)
# return: rotated vlist
def rotate(r, vlist):
    Ra = np.matrix([
        [ 1.0, 0.0, 0.0 ],
        [ 0.0, math.cos(r[0]), -math.sin(r[0]) ],
        [ 0.0, math.sin(r[0]), math.cos(r[0]) ]
        ])
    Rb = np.matrix([
        [ math.cos(r[1]), 0.0, math.sin(r[1]) ],
        [ 0.0, 1.0, 0.0 ],
        [ -math.sin(r[1]), 0, math.cos(r[1]) ]
        ])
    Rc = np.matrix([
        [ math.cos(r[2]), -math.sin(r[2]), 0.0 ],
        [ math.sin(r[2]), math.cos(r[2]), 0.0 ],
        [ 0.0, 0.0, 1.0 ]
        ])
    return np.array([ np.dot(Rc, np.dot(Rb, np.dot(Ra, v).A1).A1).A1 for line in vlist for v in line ])

# return: translated vlist
def translate(t, vlist):
    return np.array([ t+v for line in vlist for v in line ])

# draws a single cube
# return: framebuffer with single cube
def drawcube(num, count, i, vlist, effectmode):
    # framebuffer
    map = np.array(blank())

    # define some rotation and translation vectors
    rot1 = [
        (i+num)*3.14/60.0,
        (i+num*5.0)*3.14/35.0,
        (i+num*15.0)*3.14/50.0
    ]
    rot2 = [ 10, 0, 0 ]
    trans1 = [ 0, 0, 5.0 ]
    if effectmode == 0:
        trans2 = [
            25.0*math.sin(float(num)/float(count)*6.28+6.28*math.sin((i/360.0)*6.28)),
            0.0,
            15.0*math.cos(float(num)/float(count)*6.28+6.28*math.sin((i/360.0)*6.28))
        ]
    if effectmode == 1:
        trans2 = [
            25.0*math.sin(float(num)/float(count)*6.28+6.28*math.sin((i/360.0)*6.28)),
            0.0,
            15.0*math.cos(float(num)/float(count)*6.28+6.28*math.cos((i/360.0)*6.28))
        ]

    # move them around
    vlist = rotate(rot1, vlist).reshape(12,2,3)
    vlist = translate(trans2, vlist).reshape(12,2,3)
    vlist = rotate(rot2, vlist).reshape(12,2,3)
    vlist = translate(trans1, vlist).reshape(12,2,3)

    # fake depthtest, so cubes seem solid (for them selves atleast)
    vlist = depthtest(vlist).reshape(9,2,3)

    # fake z projection (make them appear smaller in the back)
    vlist = vlist*(0.8+0.6*math.cos((trans2[2]-15.0)/15.0)*1.57)

    # rasterize every line and draw it
    for line in vlist:
        for x, y, z in rasterize_line(line):
            if (x < SIZE_X) and (x >= 0) and (y < SIZE_Y) and (y >= 0):
                map[y][x] = 1
    return map

def main():
    i = 0
    vlist = cube(6.0)
    count = 4
    effectmode = 1
    while True:
        if (i%180 == 0):
            effectmode = 1 - effectmode

        # clear screen every frame
        map = np.array(blank())

        # draw multiple cubes
        for num in range(count):
            tlist = np.copy(vlist)
            map = (map + drawcube(num, count, i, tlist, effectmode))>0

        # need to invert framebuffer, because we use 1 for a pixel (black)
        # but protocol uses 0 to indicate black
        map = np.asarray(np.invert(map), dtype=np.uint8)

        send(map)
        time.sleep(1.0/FPS)

        i += 1

if __name__=="__main__":
        main()
