# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/MootSvc/SConscript,v 1.11 2012/08/17 19:00:11 jrb Exp $
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: MootSvc-01-03-01

Import('baseEnv')
Import('listFiles')
Import('packages')
#progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='MootSvc', toBuild='component')
MootSvc = libEnv.ComponentLibrary('MootSvc',
                                  listFiles(['src/*.cxx']))

#progEnv.Tool('MootSvcLib')
libEnv.Tool('registerTargets', package = 'MootSvc',
            libraryCxts = [[MootSvc, libEnv]],
            includes = listFiles(['MootSvc/*.h']),
            jo = ['src/defaultOptions.txt'])




