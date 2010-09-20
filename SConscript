# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/MootSvc/SConscript,v 1.5 2009/09/11 01:23:59 jrb Exp $
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: MootSvc-01-01-07-gr01
Import('baseEnv')
Import('listFiles')
Import('packages')
#progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('MootSvcLib', depsOnly = 1)
MootSvc = libEnv.SharedLibrary('MootSvc', listFiles(['src/*.cxx','src/Dll/*.cxx']))

#progEnv.Tool('MootSvcLib')
libEnv.Tool('registerTargets', package = 'MootSvc',
            libraryCxts = [[MootSvc, libEnv]],
            includes = listFiles(['MootSvc/*.h']))




