AM_CPPFLAGS = @VMOD_INCLUDES@

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_synth.la

libvmod_synth_la_LDFLAGS = -module -export-dynamic -avoid-version

libvmod_synth_la_SOURCES = \
    vcc_if.c vcc_if.h \
    vmod_synth.c vmod_synth.h

vcc_if.c vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_synth.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_synth.vcc

VMOD_TESTS = tests/*.vtc
.PHONY: $(VMOD_TESTS)

tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_synth.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c $(builddir)/vcc_if.h \
	$(builddir)/vmod_synth.man.rst \
	$(builddir)/vmod_synth.rst
