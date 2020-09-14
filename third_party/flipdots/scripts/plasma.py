#!/usr/bin/env python2 
import socket, time, math

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

cnt = 0
switch = 0
invert = 0
def saw():
    global cnt
    global switch
    cnt = cnt + 1
    if (cnt%20 == 0):
        switch = 1 - switch
    return -2.0+4.0*(cnt%20/20.0)

def pattern(x,y,i,border):
    global switch
    global invert

    # some variables/effects to play with
    dist = math.sqrt((x-SIZE_X/2)*(x-SIZE_X/2) + (y-SIZE_Y/2)*(y-SIZE_Y/2))
    #invert = int(math.floor((dist+i)/40.0))%2
    circle = math.sin(dist-i)
    plasma = math.sin(x/8.0)*math.sin(y/4.0)
    stripe = math.sin((x+y+50*math.sin((i%10)/10.0*2.0*3.14)-SIZE_Y/2-SIZE_X/2))
    #if ( circle < -0.5):

    # build effect with them
    if ( (plasma + stripe) < border*circle):
    #if ( stripe < -0.7):
    #if ( math.tan(x+i-SIZE_X/2) + math.tan(y+i-SIZE_Y/2) < 0.0):
        if invert:
            return switch
        else:
            return (1-switch)
    else:
        if invert:
            return (1-switch)
        else:
            return switch

def make_buffer(i):
    border = saw()
    return [([pattern(x,y,i,border) for x in range(SIZE_X)]) for y in range(SIZE_Y)]

def main():
    i = 0
    while True:
        THRES = 100*(0.5*math.sin(i/(SPEED*3.14))+0.5)
        send(make_buffer(i))
        time.sleep(1.0/FPS)

        i += 1

if __name__=="__main__":
        main()
