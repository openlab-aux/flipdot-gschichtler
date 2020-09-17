var token = null;

function saveApiToken(e) {
  e.preventDefault();

  var newToken = document.getElementById('api-token').value;

  if(newToken.length > 0) {
    localStorage.setItem('api-token', newToken);
    token = newToken;
  } else {
    localStorage.removeItem('api-token');
    token = null;
  }
}

const subjects = [
  'text',
  'shit post',
  'wisdom',
  '<code>char *</code>',
  'enc�ding err�r',
  'desparate longing for company',
  'banter',
  '👁️👄👁️',
  'SQL injection',
  'alert("XSS")',
  '<code>[Char]</code>',
  '<code>&str</code>',
  'love for <em>Zuckerwasser</em>',
  'take on systemd',
  'idle chatter',
  '(╯°□°）╯︵ ┻━┻',
  'questions for the lab',
  'ghost',
  'inspirational 👏 statement 👏'
];

function randomSubject() {
  // Profpatsch style gimmick
  // https://github.com/openlab-aux/vuizvui/blob/master/pkgs/profpatsch/profpatsch.de/talkies.js

  var span = document.getElementById('display-what');
  var subject = subjects[Math.floor(Math.random()*subjects.length)];
  span.innerHTML = subject;
}

function deleteHandler(e) {
  e.preventDefault();

  if(token != null) {
    var id = e.currentTarget.dataset.id;

    var body = new URLSearchParams();
    body.append('token', token);

    const settings = {
      method: 'DELETE',
      mode: 'same-origin',
      body: body
    };

    var req = new Request('/api/v2/queue/' + id, settings);

    fetch(req).then(res => {
      if(res.status == 204) {
        refreshQueue();
      } else {
        throw('API returned ' + res.status);
      }
    }).catch(e => alert(e));

    refreshQueue();
  } else {
    alert("Need API token!");
  }
}

function addHandler(e) {
  randomSubject();
  e.preventDefault();

  var input = document.getElementById('text');
  const text = input.value;

  var form = new URLSearchParams();
  form.append('text', text);

  const settings = {
    method: 'POST',
    mode: 'same-origin',
    body: form,
  };
  var req = new Request('/api/v2/queue/add', settings);

  fetch(req).then(res => {
    if(res.status != 200) {
      throw('API returned ' + res.status);
    } else {
      input.value = '';
      refreshQueue();
    }
  }).catch(e => alert(e));
}

function fetchQueue() {
  var settings = {
    method: 'GET',
    mode: 'same-origin',
  };

  var req = new Request('/api/v2/queue', settings);

  return fetch(req);
}

function refreshQueue() {
  var queue = document.getElementById('queue-container');

  fetchQueue().then(res => {
    if(res.status == 200) {
      res.json().then(ob => {
        queue.replaceChildren();
        for(var i in ob.queue) {
          // TODO fail on malformed
          var tr = document.createElement('tr');

          var idTd = document.createElement('td');
          idTd.innerText = ob.queue[i].id;

          var textTd = document.createElement('td');
          textTd.innerText = ob.queue[i].text;

          var actionTd = document.createElement('td');
          var actionA = document.createElement('a');
          actionA.href = '#';
          actionA.dataset.id = ob.queue[i].id;
          actionA.innerText = 'Delete';
          actionA.addEventListener('click', deleteHandler);
          actionTd.append(actionA);

          tr.append(idTd);
          tr.append(textTd);
          tr.append(actionTd);

          queue.append(tr);
        }
      });
    } else {
      throw("Response status isn't 200");
    }
  }).catch(e => {
    console.log(e);
    queue.innerHTML = '<tr><td>?</td><td>error refreshing the queue</td><td>none</td></tr>';
  });
}

function init() {
  var refreshA = document.getElementById('refresh-queue');
  refreshA.addEventListener('click', e => {
    e.preventDefault();
    refreshQueue();
  })

  var form = document.getElementById('send-form');
  form.removeAttribute('action');

  var submit = document.getElementById('submit');
  submit.addEventListener('click', addHandler);

  var subject = document.getElementById('display-what');
  subject.addEventListener('click', randomSubject);
  randomSubject();

  var apiTokenButton = document.getElementById('save-api-token');
  apiTokenButton.addEventListener('click', saveApiToken);

  var apiToken = localStorage.getItem('api-token');
  if(apiToken) {
    token = apiToken;
    var apiTokenInput = document.getElementById('api-token');
    apiTokenInput.value = token;
  }

  refreshQueue();
}

document.addEventListener('DOMContentLoaded', init);

// vim: set ft=javascript:
