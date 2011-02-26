# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/MootSvc/SConscript,v 1.8 2010/12/02 01:39:25 jrb Exp $
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: MootSvc-01-01-10
Import('baseEnv')
Import('listFiles')
Import('packages')
#progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='MootSvc', toBuild='component')
MootSvc = libEnv.SharedLibrary('MootSvc',
                               listFiles(['src/*.cxx','src/Dll/*.cxx']))

#progEnv.Tool('MootSvcLib')
libEnv.Tool('registerTargets', package = 'MootSvc',
            libraryCxts = [[MootSvc, libEnv]],
            includes = listFiles(['MootSvc/*.h']),
            jo = ['src/defaultOptions.txt'])




