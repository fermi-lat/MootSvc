# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/MootSvc/MootSvcLib.py,v 1.1 2008/08/15 21:42:36 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['MootSvc'])

    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('facilitiesLib')
    env.Tool('xmlBaseLib')
    env.Tool('EventLib')
    env.Tool('LdfEventLib')
    env.Tool('mootCoreLib')
def exists(env):
    return 1;
