# -*- coding: utf-8 -*-
import requests

class FlipdotGschichtlerError(Exception):
    """Error from the REST API"""
    def __init__(self, endpoint, status, message):
        self.endpoint = endpoint
        self.status = status
        self.message = message

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
            raise Exception('missing api token')

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
