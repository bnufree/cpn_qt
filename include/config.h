#ifndef OCPN_CONFIG_H
#define OCPN_CONFIG_H

/* #undef LINUX_CRASHRPT */
/* #undef OCPN_USE_CRASHRPT */
/* #undef OCPN_HAVE_X11 */

// Garmin Host Mode support
#define USE_GARMINHOST

#define OCPN_USE_LZMA

#define OCPN_USE_CURL

/* #undef USE_LIBELF */

#define USE_GARMINHOST

#define OCPN_USE_NEWSERIAL

// Enable dark mode on modern MacOS.
/* #undef OCPN_USE_DARKMODE */

/* #undef ocpnUSE_GLES */


#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif


#define OPENGL_FOUND
/* #undef HAVE_PORTAUDIO */
#define HAVE_SYSTEM_CMD_SOUND
/* #undef HAVE_SNDFILE */

// Command line to play a sound in system(3), like "aplay %s".
#define SYSTEM_SOUND_CMD  "PowerShell (New-Object Media.SoundPlayer \\\"%s\\\").PlaySync();"

// Flag for ancient compilers without C++11 support
/* #undef COMPILER_SUPPORTS_CXX11 */


#define VERSION_MAJOR 5
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_DATE "2020 ?07-19"
#define VERSION_FULL "5.0.0+"

#endif  // OCPN_CONFIG_H
