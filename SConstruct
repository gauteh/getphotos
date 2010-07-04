import os

env = Environment ()

# env.ParseConfig ('pkg-config --cflags --libs sdl')

#env.Append (LIBS = 'SDL_ttf')
env.Append (CXXFLAGS = '-g') # debug

source_files = [
    'getphotos.cpp',
    ]

env.Program (target = 'getphotos', source = source_files)

