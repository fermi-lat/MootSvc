# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/MootSvc/MootSvcLib.py,v 1.2 2009/08/27 16:39:00 jrb Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['MootSvc'])

    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('facilitiesLib')
    env.Tool('xmlBaseLib')
    env.Tool('EventLib')
    env.Tool('LdfEventLib')
    env.Tool('mootCoreLib')
    env.Tool('CalibDataLib')
def exists(env):
    return 1;
