MAINTAINERCLEANFILES    = Makefile.in

if HAVE_SNDFILE
JACKREC = jackrec
dist-check-sndfile:
else
JACKREC =
dist-check-sndfile:
	@echo
	@echo ' ******' You need sndfile installed to make dist.' ******'
	@echo
	@false
endif

bin_PROGRAMS = jack_simple_client \
	       jack_transport_client \
	       jack_impulse_grabber \
	       jack_metro \
	       jack_showtime \
	       jack_midisine \
	       jack_midiseq \
	       $(JACKREC)

AM_CFLAGS = -I.. $(JACK_CFLAGS)
AM_CXXFLAGS = -I.. $(JACK_CFLAGS)

jack_simple_client_SOURCES = simple_client.c
jack_simple_client_LDFLAGS = @OS_LDFLAGS@
jack_simple_client_LDADD = $(top_builddir)/libjack/libjack.la

jack_transport_client_SOURCES = transport_client.c
jack_transport_client_LDFLAGS = @OS_LDFLAGS@
jack_transport_client_LDADD = $(top_builddir)/libjack/libjack.la

jack_metro_SOURCES = metro.c
jack_metro_LDFLAGS = @OS_LDFLAGS@
jack_metro_LDADD = $(top_builddir)/libjack/libjack.la

jack_showtime_SOURCES = showtime.c
jack_showtime_LDFLAGS = @OS_LDFLAGS@
jack_showtime_LDADD = $(top_builddir)/libjack/libjack.la

jack_impulse_grabber_SOURCES = impulse_grabber.c
jack_impulse_grabber_LDFLAGS = @OS_LDFLAGS@
jack_impulse_grabber_LDADD = $(top_builddir)/libjack/libjack.la

jack_midiseq_SOURCES = midiseq.c
jack_midiseq_LDFLAGS = @OS_LDFLAGS@
jack_midiseq_LDADD = $(top_builddir)/libjack/libjack.la

jack_midisine_SOURCES = midisine.c
jack_midisine_LDFLAGS = @OS_LDFLAGS@
jack_midisine_LDADD = $(top_builddir)/libjack/libjack.la

if HAVE_SNDFILE
jackrec_SOURCES = capture_client.c
jackrec_LDFLAGS = @SNDFILE_LIBS@ @OS_LDFLAGS@
jackrec_LDADD = $(top_builddir)/libjack/libjack.la
endif

#
# sample in-process client(s)
#
ip_clientdir = $(ADDON_DIR)

ip_client_LTLIBRARIES = inprocess.la intime.la

inprocess_la_LDFLAGS = -module -avoid-version @OS_LDFLAGS@
inprocess_la_SOURCES = inprocess.c

intime_la_LDFLAGS = -module -avoid-version @OS_LDFLAGS@
intime_la_SOURCES = intime.c
