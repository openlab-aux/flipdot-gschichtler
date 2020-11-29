from setuptools import setup

setup(
    name = 'anzeigetafel',
    version = '2.0.0',
    py_modules = [ 'anzeigetafel' ],
    entry_points = {
        'console_scripts' : {
            'anzeigetafel=anzeigetafel:main'
        }
    }
)
