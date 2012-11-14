
OPTIONALSAMPLES = @OPTIONALSAMPLES@
OPTIONALINCLUDES = @OPTIONALINCLUDES@
spooleslib = @spooleslib@
genmmdlib = @genmmdlib@
amdbarlib = @amdbarlib@

EXTRA_PROGRAMS = spoolesmmd liummd addamd 
noinst_PROGRAMS =  chachb2dobrian symbfact \
		sloan greedyfillredux constrainedWavefront $(OPTIONALSAMPLES)



#all2chaco 
# chaco2hb
# chaco2hb_SOURCES = chaco2hb.cc

chachb2dobrian_SOURCES = chachb2dobrian.cc

#all2chaco_SOURCES = all2chaco.cc
# qgraph 
# qgraph_SOURCES = qgraph.cc
# pseudodiam 
#pseudodiam_SOURCES = pseudodiam.cc

symbfact_SOURCES = symbfact.cc

sloan_SOURCES = sloan.cc

greedyfillredux_SOURCES = greedyfillredux.cc

constrainedWavefront_SOURCES = constrainedWavefront.cc

spoolesmmd_SOURCES = spoolesmmd.cc
spoolesmmd_LDADD = $(spooleslib)

liummd_SOURCES = liummd.cc
liummd_LDADD = $(genmmdlib)

addamd_SOURCES = addamd.cc
addamd_LDADD = $(amdbarlib)

INCLUDES =  -I$(top_srcdir)/include $(OPTIONALINCLUDES)
LIBS = ../libspindle/libspindle.a
