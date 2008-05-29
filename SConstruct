# (c) 2008 Tod D. Romo
#
# Grossfield Lab
# Department fo Biochemistry & Biophysics
# University of Rochester Medical School
#
#

import sys

env = Environment(tools = ["default", "doxygen"], toolpath = '.')

# Some rudimentary autoconfish stuff...
if not env.GetOption('clean'):
   conf = Configure(env)
   if not conf.CheckLib('boost_regex'):
      print "***ERROR*** You must have the Boost regular expression libraries installed"
      Exit(1)

   if sys.platform == 'linux2':
      prior = env.get('LIBPATH')
      for dir in ['', '/usr/lib64/atlas', '/usr/lib/atlas', '/usr/local/atlas']:
      	  checks = 1
	  missing = []
	  if dir != "":
	     env.Replace(LIBPATH = [dir])
	     print "Checking for libraries in %s..." % dir
          else:
	     print "Checking for libraries..."

	  if not conf.CheckLib('lapack'):
	     checks = 0
	     missing += ['lapack']
	  if not conf.CheckLib('atlas'):
	     checks = 0
	     missing += ['atlas']
	  if checks:
	     break
      if not checks:
          print "***ERROR*** Missing libraries: ", missing
	  Exit(1)

      if prior != None:
          env['LIBPATH'] = prior + env['LIBPATH']

   env = conf.Finish()

### Compile-flags

debug_opts='-g -Wall'
release_opts='-O3'

# Setup the general environment...
env.Append(CPPPATH = ['#'])
env.Append(LIBPATH = ['#'])
env.Append(LIBS = ['loos', 'boost_regex'])


# Platform specific build options...
if sys.platform == 'darwin':
   env.Append(LINKFLAGS = ' -framework vecLib')

elif sys.platform == 'linux2':
   env.Append(LIBS = ['lapack', 'atlas'])


# Determine what kind of build...
release=ARGUMENTS.get('release', 0)
if int(release):
    env.Append(CCFLAGS=release_opts)
else:
    env.Append(CCFLAGS=debug_opts)

debug=ARGUMENTS.get('debug', 0)
if int(debug):
   if int(release):
      print "***ERROR*** You cannot have a release with debugging code included."
      Exit(1)
   env.Append(CCFLAGS=" -DDEBUG")


# Export for subsidiary SConscripts
Export('env')

# Build the LOOS library...
library_files = Split('dcd.cpp utils.cpp dcd_utils.cpp AtomicGroup.cpp pdb_remarks.cpp pdb.cpp psf.cpp KernelValue.cpp grammar.yy scanner.ll')
loos = env.Library('loos', library_files)

env.Default(loos)


docs = env.Doxygen('Doxyfile')
examples = SConscript('Examples/SConscript')
tests = SConscript('Tests/SConscript')
tools = SConscript('Tools/SConscript')


# build targets...

env.Alias('docs', docs)
env.Alias('examples', examples)
env.Alias('tests', tests)
env.Alias('tools', tools)

env.Alias('all', loos + examples + tools)
env.Alias('caboodle', loos + examples + tests + tools + docs)

