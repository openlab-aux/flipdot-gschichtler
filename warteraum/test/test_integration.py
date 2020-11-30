import requests
from flipdot_gschichtler import FlipdotGschichtlerClient, FlipdotGschichtlerError
import pytest
import sys

BASE_URL = 'http://localhost:9000'
TOKEN = 'hannes'

# technically somebody could use these tokens,
# but tbh they deserve this test to fail then
WRONG_TOKENS = [ 'password', 'admin', '12345678' ]

MAX_TEXT_LEN = 512

api = FlipdotGschichtlerClient(BASE_URL, api_token = TOKEN)

# API response format

def test_queue_add_format():
    my_text = 'hello world'
    r = requests.post(BASE_URL + '/api/v2/queue/add', data = { 'text' : my_text })

    assert r.status_code == 200

    assert r.json()['text'] == my_text
    my_id = r.json()['id']

    assert my_id >= 0

def test_queue_format():
    r = requests.get(BASE_URL + '/api/v2/queue')

    assert r.status_code == 200
    assert r.json()['length'] == len(r.json()['queue'])

def test_queue_del_format():
    my_id = api.add('test_queue_del_format')

    r = requests.delete('{}/api/v2/queue/{}'.format(BASE_URL, my_id), data = { 'token' : TOKEN })

    # accept unauthorized (we can't know the token if a custom one was set
    # via the nix derivation arguments)
    if r.status_code == 401:
        pytest.xfail('invalid API token is configured')

    assert r.status_code == 204

def test_queue_404_format():
    r = requests.get(BASE_URL + '/api/v2/coffee')

    assert r.status_code == 404
    assert 'not found' in r.json()['error']

def test_announcement_formats():
    my_text = 'important news'

    r = requests.delete(BASE_URL + '/api/v2/announcement', data = { 'token' : TOKEN })
    assert r.status_code == 204

    r1 = requests.get(BASE_URL + '/api/v2/announcement')
    assert r1.status_code == 404
    assert r1.json()['announcement'] == None

    r2 = requests.put(BASE_URL + '/api/v2/announcement', data = { 'text' : my_text, 'token' : TOKEN })
    assert r2.status_code == 200
    assert r2.json()['announcement'] == my_text

    r3 = requests.get(BASE_URL + '/api/v2/announcement')
    assert r3.status_code == 200
    assert r3.json()['announcement'] == my_text

    # check that token is required
    r4 = requests.put(BASE_URL + '/api/v2/announcement', data = { 'text' : 'oops' })
    assert r4.status_code == 400

# /api/v2/queue/add input validation and normalization

def test_strip_whitespace():
    r = requests.post(BASE_URL + '/api/v2/queue/add', data = { 'text' : '   foo   ' })
    assert r.json()['text'] == 'foo'

def test_text_within_tolerance():
    long_string = '?' * MAX_TEXT_LEN
    my_id = api.add(long_string)

    found = False
    for q in api.queue():
        if q['id'] == my_id:
            assert q['text'] == long_string
            found = True

    assert found

def test_too_long_text():
    long_string = '!' * (MAX_TEXT_LEN + 1)

    r = requests.post(BASE_URL + '/api/v2/queue/add', data = { 'text' : long_string })

    assert r.status_code == 413

def test_whitespace_irrelevant_for_length():
    long_string = 'foo' + (MAX_TEXT_LEN * ' ')

    r = requests.post(BASE_URL + '/api/v2/queue/add', data = { 'text' : long_string })

    assert r.status_code == 200
    assert r.json()['text'] == 'foo'

# authentication

def test_expected_authentication_failures():
    for t in WRONG_TOKENS:
        # client with wrong token
        tmp_client = FlipdotGschichtlerClient(BASE_URL, api_token = t)

        # should be able to add text
        my_id = tmp_client.add(t)

        # but not delete them
        with pytest.raises(FlipdotGschichtlerError) as exc_info:
            tmp_client.delete(my_id)

        assert exc_info.value.status == 401

        # what our normal client can do
        api.delete(my_id)

def test_correct_failure_with_valid_token():
    q = api.queue()

    if len(q) > 0:
        highest_id = q[-1]['id']
    else:
        highest_id = 0

    with pytest.raises(FlipdotGschichtlerError) as err:
        api.delete(highest_id + 1)
        assert err.status == 404

def test_expected_authentication_failures_announcement():
    my_announcement = 'announcement works'

    for t in WRONG_TOKENS:
        tmp_client = FlipdotGschichtlerClient(BASE_URL, api_token = t)

        with pytest.raises(FlipdotGschichtlerError) as exc_info:
            tmp_client.delete_announcement()

        assert exc_info.value.status == 401

        api.delete_announcement()

        with pytest.raises(FlipdotGschichtlerError) as exc_info:
            tmp_client.set_announcement(my_announcement)

        assert exc_info.value.status == 401

        api.set_announcement(my_announcement)

        assert tmp_client.announcement() == my_announcement

# queue properties

def test_queue_ascending_ids():
    for x in range(15):
        api.add(str(x))

    last_id = -1
    for q in api.queue():
        assert q['id'] > last_id
        last_id = q['id']

def test_reassigning_only_after_emptying():
    for x in range(15):
        api.add(str(x))

    queue = api.queue()

    to_delete = [x['id'] for x in queue[:-1]]

    for d in to_delete:
        api.delete(d)

    assert len(api.queue()) == 1

    for x in range(15):
        api.add(str(x))

    ids_after = [x['id'] for x in api.queue()]

    for d in to_delete:
        assert not (d in ids_after)

    for q in api.queue():
        api.delete(q['id'])

    assert api.add('only text') == 0
    assert len(api.queue()) == 1
    assert api.queue()[0]['id'] == 0
