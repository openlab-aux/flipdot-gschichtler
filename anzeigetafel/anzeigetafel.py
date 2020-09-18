import os
import sys
import requests
from flipdots.scripts.scroll_text import scroll_text
from time import sleep

if len(sys.argv) == 2:
    TOKEN = sys.argv[1]
else:
    print("Usage: {} TOKEN".format(sys.argv[0]))
    exit(1)

BASEURL = 'https://flipdot.openlab-augsburg.de/api/v2'

def get_queue():
    r = requests.get(BASEURL + '/queue')
    return r.json()['queue']

def delete_queue_entry(id):
    r = requests.delete(BASEURL + '/queue/{}'.format(id), data={'token' : TOKEN })
    if r.status_code == 204:
        return (True, r.status_code)
    else:
        return (False, r.status_code)

while True:
    queue = get_queue()

    while len(queue) < 1:
        try:
            queue = get_queue()
        except:
            pass
        sleep(15)

    nextentry = queue[0]
    id = nextentry['id']
    text = nextentry['text']

    print("Drawing string \"{}\" with id {}".format(text.encode("utf-8"), id))

    scroll_text("localhost", 2323, "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", text)

    (sucess, status) = delete_queue_entry(id)

    if sucess:
        print("Deleted queue item {}".format(id))
    else:
        print("Could not delete item {}: HTTP Error {}".format(id, status))
