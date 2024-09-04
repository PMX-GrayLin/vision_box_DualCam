# =============================================================================
# User configuration section.
#
# These options control compilation on all systems apart from Windows and Mac
# OS X. Use CMake to compile on Windows and Mac.
#
# Largely, these are options that are designed to make mosquitto run more
# easily in restrictive environments by removing features.
#
# Modify the variable below to enable/disable features.
#
# Can also be overriden at the command line, e.g.:
#
# make WITH_TLS=no
# =============================================================================

CLIENT_SHARED=../client_shared.o
_MCUCTL_=../mcuCtl/mcuCtl.o
MQTT_LIB=/usr/local/lib/libmosquitto.so.1