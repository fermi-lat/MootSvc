# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/MootSvc/SConscript,v 1.9 2011/02/26 16:54:07 jrb Exp $
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: MootSvc-01-02-00

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




