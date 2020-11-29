#!/usr/bin/env python
import os
import sys
from flipdots.scripts.scroll_text import scroll_text
from flipdot_gschichtler import FlipdotGschichtlerClient
from time import sleep

if len(sys.argv) == 2:
    TOKEN = sys.argv[1]
else:
    print("Usage: {} TOKEN".format(sys.argv[0]))
    exit(1)

BASEURL = 'https://flipdot.openlab-augsburg.de'

FLIPDOT_HOST = 'localhost'
FLIPDOT_PORT = 2323

FONT = '/usr/share/fonts/truetype/unifont/unifont.ttf'

api = FlipdotGschichtlerClient(BASEURL, api_token = TOKEN)

while True:
    try:
        queue = api.queue()
    except:
        print("Retrieving queue failed")

    # select task
    if len(queue) > 0:
        target_text = queue[0]['text']
        target_id = queue[0]['id']

        print("Drawing queue string \"{}\" with id {}".format(target_text.encode("utf-8"), target_id))

        scroll_text(FLIPDOT_HOST, FLIPDOT_PORT, FONT, target_text)

        try:
            api.delete(target_id)
            print("Deleted queue item {}".format(target_id))
        except:
            print("Could not delete item {}".format(target_id))
    else:
        sleep(15)
