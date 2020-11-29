from setuptools import setup

setup(
    name = 'flipdots',
    version = 'unstable',
    packages = [ 'flipdots.scripts' ],
    package_dir = { 'flipdots' : '.' },
    install_requires = [ 'pillow', 'Flask', 'numpy' ]
)
