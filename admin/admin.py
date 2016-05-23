import requests as r
import flask as f
import jinja2 as j

import os

URL = "http://flipdot.openlab-augsburg.de"
TOKEN = '7bf7303b847f359f32bff627519e4dd4f4bbc2d0638a758d81bbb156c5d30569'
HOST = "0.0.0.0"
PORT = 5000

print(os.path.dirname(os.path.abspath(__file__)))
jinja_env = j.Environment(loader=j.FileSystemLoader(
    os.path.dirname(os.path.abspath(__file__))
))
template = jinja_env.get_template("admin.html")

app = f.Flask(__name__)

@app.route("/", methods=["GET"])
def root():
    queue = r.get(URL + "/queue").json()
    return template.render(queue=queue[u"queue"])

@app.route("/<int:item_id>")
def delete(item_id):
    url = "{}/queue/del/{}".format(URL, item_id)
    resp = r.delete(url, data={ "token": TOKEN })
    if resp.status_code == r.codes.no_content:
        return f.redirect("/", code=r.codes.see_other)
    else:
        return "Flipdot API returned {}".format(resp.status_code), r.codes.server_error

if __name__ == "__main__":
    app.run(host=HOST, port=PORT)
