from setuptools import setup

setup(
    name = 'flipdots',
    version = '0',
    packages = [ 'flipdots.scripts' ],
    package_dir = { 'flipdots' : '.' },
    install_requires = [ 'pillow', 'Flask', 'numpy' ]
)
