from distutils.core import setup
from distutils.core import Extension

ext = Extension(
    'wsp',
    sources=[
        'src/python/wsp.c',
        'src/python/WhisperException.c',
        'src/python/WhisperFile.c',
        'src/python/WhisperArchiveInfo.c',
    ],
    extra_compile_args=[
        '-I./src'
    ],
    extra_link_args=[
        'wsp.a'
    ]
)

setup(
    name='wsp',
    version='1.0',
    description='Native whisper library for python',
    ext_modules=[ext]
)
