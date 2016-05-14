import os
import requests
import scrolltext
from time import sleep

TOKEN = '7bf7303b847f359f32bff627519e4dd4f4bbc2d0638a758d81bbb156c5d30569'
BASEURL = 'http://flipdot.openlab-augsburg.de/'

def get_queue():
    r = requests.get(BASEURL + '/queue')
    return r.json()['queue']

def delete_queue_entry(id):
    r = requests.delete(BASEURL + '/queue/del/{}'.format(id), data={'token' : TOKEN })
    if r.status_code == 204:
        return (True, r.status_code)
    else:
        return (False, r.status_code)

def draw_string(str):
    imgmap = scrolltext.str2array(str)
    scrolltext.scroll_text(imgmap)

while True:
    queue = get_queue()

    while len(queue) < 1:
        queue = get_queue()
        sleep(1)
    
    nextentry = queue[0]
    id = nextentry['id']
    text = nextentry['text']

    print("Drawing string \"{}\" with id {}".format(text.encode("utf-8"), id))

    draw_string(text)

    (sucess, status) = delete_queue_entry(id)
    if sucess:
        print("Deleted queue item {}".format(id))
    else:
        print("Could not delete item {}: HTTP Error {}".format(id, status))
