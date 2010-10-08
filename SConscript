# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/MootSvc/SConscript,v 1.5.20.1 2010/09/20 16:13:41 heather Exp $
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: MootSvc-01-01-08-gr01

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




