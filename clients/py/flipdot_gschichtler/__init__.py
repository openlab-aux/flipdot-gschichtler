# -*- coding: utf-8 -*-
import requests

class FlipdotGschichtlerError(Exception):
    """Error from the REST API"""
    def __init__(self, endpoint, status, message):
        # make this a proper exception
        super().__init__(message)

        self.endpoint = endpoint
        self.status = status
        self.message = message

    def __str__(self):
        return "Endpoint {} returned {} with message \"{}\"".format(
            self.endpoint, str(self.status), self.message)

class FlipdotGschichtlerClient():
    def __init__(self, base_url, api_token = None):
        self.base_url = base_url
        self.api_token = api_token

    def __get_error_message(self, r):
        try:
            return r.json()['error']
        except KeyError:
            return 'no message given'
        except ValueError:
            return 'no json response'

    def add(self, text):
        endpoint = '/api/v2/queue/add'
        r = requests.post(self.base_url + endpoint, data = { 'text' : text })

        if r.status_code == 200:
            return r.json()['id']
        else:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerError(endpoint, r.status_code, message)

    def delete(self, queue_id):
        if self.api_token is None:
            raise FlipdotGschichtlerNoToken

        endpoint = '/api/v2/queue/' + str(queue_id)
        r = requests.delete(self.base_url + endpoint, data = { 'token': self.api_token })

        if r.status_code != 204:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerError(endpoint, r.status_code, message)

    def queue(self):
        endpoint = '/api/v2/queue'
        r = requests.get(self.base_url + endpoint)

        if r.status_code == 200:
            return r.json()['queue']
        else:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerClient(endpoint, r.status_code, message)

    def queue_length(self):
        endpoint = '/api/v2/queue'
        r = requests.get(self.base_url + endpoint)

        if r.status_code == 200:
            return r.json()['length']
        else:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerClient(endpoint, r.status_code, message)

    def announcement(self, with_expiry = False):
        endpoint = '/api/v2/announcement'
        r = requests.get(self.base_url + endpoint)

        if r.status_code == 200:
            j = r.json()
            if with_expiry:
                return { 'text' : j['announcement'], 'expiry' : j['expiry_utc'] }
            else:
                return j['announcement']
        elif r.status_code == 404:
            # no / empty announcement
            assert 'announcement' in r.json()
            return None
        else:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerError(endpoint, r.status_code, message)

    def set_announcement(self, text, expiry = None):
        if self.api_token == None:
            raise FlipdotGschichtlerNoToken

        request = {
            'text': text,
            'token': self.api_token
        }

        if expiry != None:
            if type(expiry) is int:
                request['expiry_utc'] = expiry
            else:
                raise TypeError('expiry is expected to be an integer')

        endpoint = '/api/v2/announcement'
        r = requests.put(self.base_url + endpoint, data = request)

        if r.status_code != 200:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerError(endpoint, r.status_code, message)

    def delete_announcement(self):
        if self.api_token == None:
            raise FlipdotGschichtlerNoToken

        endpoint = '/api/v2/announcement'
        r = requests.delete(self.base_url + endpoint, data = { 'token': self.api_token })

        if r.status_code != 204:
            message = self.__get_error_message(r)
            raise FlipdotGschichtlerError(endpoint, r.status_code, message)
