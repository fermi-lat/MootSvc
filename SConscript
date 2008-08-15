# -*- python -*-
# $Header$
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: MootSvc-01-01-06
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('MootSvcLib', depsOnly = 1)
MootSvc = libEnv.SharedLibrary('MootSvc', listFiles(['src/*.cxx','src/Dll/*.cxx']))

progEnv.Tool('MootSvcLib')
progEnv.Tool('registerObjects', package = 'MootSvc', libraries = [MootSvc], includes = listFiles(['MootSvc/*.h']))
