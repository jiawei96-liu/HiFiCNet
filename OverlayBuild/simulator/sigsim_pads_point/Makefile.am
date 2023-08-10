AUTOMAKE_OPTIONS = foreign -Wno-syntax subdir-objects
ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -I./src -I./vendor -L./vendor/loci -Wstrict-prototypes -Wall -Wextra  -fPIC -pthread -lcfluid_base -loci 

noinst_LIBRARIES =
noinst_LTLIBRARIES =

include vendor/patricia/automake.mk
include vendor/log/automake.mk
include vendor/json/automake.mk
include src/automake.mk