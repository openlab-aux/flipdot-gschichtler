import sqlite3
from flask import Flask, render_template, g, request, url_for, abort, redirect, jsonify

DATABASE = './queue.db'
TOKEN = '7bf7303b847f359f32bff627519e4dd4f4bbc2d0638a758d81bbb156c5d30569'
DEBUG = True

app = Flask(__name__)
app.config.from_object(__name__)

# db
@app.before_request
def before_request():
    g.db = sqlite3.connect(DATABASE)

@app.teardown_request
def teardown_request(exception):
    db = getattr(g, 'db', None)
    if db is not None:
        db.close()

@app.route("/")
def root():
    return render_template('root.html')

# queue management
@app.route("/queue")
def list_queue():
    cursor = g.db.cursor()
    rows = [{ "id" : row[0], "text" : row[1] } for row in cursor.execute("select id, text from queue")]
    return jsonify(length=len(rows),queue=rows)

@app.route("/queue/add", methods=['POST'])
def add_to_queue():
    if request.form['text'] != '':
        cursor = g.db.cursor()
        cursor.execute('insert into queue (text) values (?)', [request.form['text']])
        g.db.commit()

        queuelength = cursor.execute('select count(id) from queue').fetchone()[0] - 1

        return render_template("success.html", queuelength=queuelength)
    else:
        return render_template("error.html", error="Du hast einen leeren Text abgesendet!")

@app.route("/queue/del/<id>", methods=['DELETE'])
def del_from_queue(id):
    if request.form['token'] != TOKEN:
        abort(401)
    else:
        cursor = g.db.cursor()
        rows = cursor.execute('delete from queue where id = ?', (id,)).rowcount
        g.db.commit()
        if rows == 1:
            return "Deleted", 204
        else:
            abort(404)

if __name__ == "__main__":
    app.run(debug=DEBUG)
