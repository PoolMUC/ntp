/* 
 * ports/winnt/include/config.h - static Windows config.h
 *
 * On most systems config.h is generated by the configure script.
 * For the Windows port, it's hand-maintained.  Compilers earlier
 * than Visual C++ 2005 are no longer supported, enabling
 * portable use of "long long" and "%lld".
 */

#ifndef CONFIG_H
#define CONFIG_H

 /*
  * Known predifined MS C compiler _MSC_VER values:
  *
  *  1930  MSVC++ 14.3 (Visual Studio 2022)
  *  1920  MSVC++ 14.2 (Visual Studio 2019)
  *  1910  MSVC++ 14.1 (Visual Studio 2017)
  *  1900  MSVC++ 14.0 (Visual Studio 2015)
  *  1800: MSVC++ 12.0 (Visual Studio 2013)
  *  1700: MSVC++ 11.0 (Visual Studio 2012)
  *  1600: MSVC++ 10.0 (Visual Studio 2010)
  *  1500: MSVC++ 9.0  (Visual Studio 2008)
  *  1400: MSVC++ 8.0  (Visual Studio 2005) (minimum supported)
  *
  * Note comparisons should be made using <, >, <=, or >= as there are
  * other revisions released between major versions.
  */

#if defined(_MSC_VER) && _MSC_VER < 1400
#error Minimum supported Microsoft compiler is Visual C++ 2005.
#endif

/*
 * We want structures and prototypes added after Windows NT 4.0 exposed
 * by Windows header files, so we define _WIN32_WINNT to target Windows
 * XP (version 5.1).  By default, _WIN32_WINNT also controls the minimum
 * required Windows version to launch the .exe.  As we want a single
 * binary to work on all supported Windows versions, we runtime link
 * newer functions, and use the linker /version:0x0400 option to
 * override the .EXE header minimum Windows version.
 *
 * When using the VC++ 2008 and later compilers, the resulting binaries
 * will not work on versions earlier than Windows XP, due to runtime
 * library dependencies.  That is, Visual C++ 2005 is the last version
 * capable of producing binaries usable with Windows NT 4 and 2000.
 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define __attribute__(x) /* empty */
#define _CRT_SECURE_NO_DEPRECATE 1

/*
 * ANSI C compliance enabled
 */
#define __STDC__ 1

/*
 * Enable the debug build of MS C runtime to dump leaks
 * at exit time (currently only if run under a debugger).
 */
#if defined(_MSC_VER) && defined(_DEBUG)
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <malloc.h>
# include <crtdbg.h>
/* # define MALLOC_LINT */	/* defers free() */
# define EREALLOC_IMPL(ptr, newsz, filenm, loc) \
	 _realloc_dbg(ptr, newsz, _NORMAL_BLOCK, filenm, loc)
#endif

/*
 * We need to include stdio.h first before we #define snprintf
 * otherwise we can get errors during the build
 */
#include <stdio.h>

/* Prevent inclusion of winsock.h in windows.h */
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#ifndef __RPCASYNC_H__
#define __RPCASYNC_H__
#endif

/*
 * On Unix struct sock_timeval is equivalent to struct timeval.
 * On Windows built with 64-bit time_t, sock_timeval.tv_sec is a long
 * as required by Windows' socket() interface timeout argument, while
 * timeval.tv_sec is time_t for the more common use as a UTC time 
 * within NTP.
 *
 * winsock.h unconditionally defines struct timeval with long tv_sec
 * instead of time_t tv_sec.  We redirect its declaration to struct 
 * sock_timeval instead of struct timeval with a #define.
 */
#define	timeval sock_timeval

/* Include Windows headers */
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#undef timeval	/* see sock_timeval #define and comment above */

/*
 * Some definitions we are using are missing in the headers
 * shipping with VC6. However, if the SDK is installed then the 
 * SDK's headers may declare the missing types. This is at least 
 * the case in the Oct 2001 SDK. That SDK and all subsequent 
 * versions also define the symbol _W64, so we can use that one
 * to determine whether some types need to be defined, or not.
 */
#ifdef _W64
/* VC6 can include wspiapi.h only if the SDK is installed */
#include <wspiapi.h>
#endif

#undef interface
#include <process.h>
#include <time.h>		/* time_t for timeval decl */
#include <io.h>
#include <isc/strerror.h>

/* ---------------------------------------------------------------------
 * Above this line are #include lines and the few #define lines
 * needed before including headers.
 */

struct timeval {
	time_t	tv_sec;
	long	tv_usec;
};

/*
 * ntohl and friends are actual functions on Windows, use our own
 * macros instead to save the function call overhead.  All releases
 * of Windows are little-endian.
 */
#ifdef ntohl
#error ntohl is already defined in ports/winnt/include/config.h
#else
#define ntohl(ul)	(((u_long)(ul) & 0xff) << 24 |		\
			 ((u_long)(ul) & 0xff00) << 8 |		\
			 ((u_long)(ul) & 0xff0000) >> 8 |	\
			 ((u_long)(ul) & 0xff000000) >> 24)
#define htonl(ul)	ntohl(ul)
#define ntohs(us)	((u_short)				\
			 (((u_short)(us) & 0xff) << 8 |		\
			  ((u_short)(us) & 0xff00) >> 8))
#define htons(us)	ntohs(us)
#endif

/*
 * On Unix open() works for tty (serial) devices just fine, while on
 * Windows refclock serial devices are opened using CreateFile, a lower
 * level than the CRT-provided descriptors, because the C runtime lacks
 * tty APIs.  For refclocks which wish to use open() as well as or 
 * instead of refclock_open(), tty_open() is equivalent to open() on
 * Unix and  implemented in the Windows port similarly to
 * refclock_open().
 */
extern int tty_open(const char *, int, int);

/*
 * disable use of __declspec(dllexport) by libisc routines
 */
#define ISC_STATIC_WIN	1

/*
 * ntp_rfc2553.h has cruft under #ifdef SYS_WINNT which is
 * appropriate for older Microsoft IPv6 definitions, such
 * as in_addr6 being the struct type.  We can differentiate
 * the RFC2553-compliant newer headers because they have
 *   #define in_addr6 in6_addr
 * for backward compatibility.  With the newer headers,
 * we define ISC_PLATFORM_HAVEIPV6 and disable the cruft.
 */
#ifdef in_addr6
#define WANT_IPV6
#define ISC_PLATFORM_HAVEIPV6
#define ISC_PLATFORM_HAVESCOPEID
#define HAVE_STRUCT_SOCKADDR_STORAGE
#define ISC_PLATFORM_HAVEIN6PKTINFO
#endif	/* in_addr6 / RFC2553-compliant IPv6 headers */

#define NO_OPTION_NAME_WARNINGS

#if !defined( _W64 )
  /*
   * if ULONG_PTR needs to be defined then the build environment
   * is pure 32 bit Windows. Since ULONG_PTR and DWORD have 
   * the same size in 32 bit Windows we can safely define
   * a replacement.
   */
typedef DWORD ULONG_PTR;
/* VC6 doesn't know about socklen_t, except if the SDK is installed */
typedef int socklen_t;
#endif	/* _W64 */

#define ISC_PLATFORM_NEEDIN6ADDRANY
#define HAVE_SOCKADDR_IN6

/*
 * The type of the socklen_t defined for getnameinfo() and getaddrinfo()
 * is int for VS compilers on Windows but the type is already declared 
 */
#define GETSOCKNAME_SOCKLEN_TYPE socklen_t

/*
 * Older SDKs do not define SO_EXCLUSIVEADDRUSE in winsock2.h
 */
#ifndef SO_EXCLUSIVEADDRUSE
#define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR))
#endif

#define SIZEOF_TIME_T 8


/*
 * An attempt to cut down the number of warnings generated during compilation.
 * All of these should be benign to disable.
 */

#pragma warning(disable: 4100) /* unreferenced formal parameter */
#pragma warning(disable: 4127) /* conditional expression is constant */
#pragma warning(disable: 4996) /* more secure replacement available */

/*
 * Windows NT Configuration Values
 */
#if defined _DEBUG /* Use VC standard macro definitions */
# define DEBUG 1
#endif

#define __windows__ 1
/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

#define OPEN_BCAST_SOCKET		1	/* for ntp_io.c */
#define TYPEOF_IP_MULTICAST_LOOP	BOOL
#define SETSOCKOPT_ARG_CAST		(const char *)
#define HAVE_RANDOM 
#define SAVECONFIG			1

/*
 * Multimedia timer enable
 */
#define USE_MM_TIMER

/* check for OpenSSL */
#ifdef OPENSSL
# define USE_OPENSSL_CRYPTO_RAND 1
# define AUTOKEY
# define HAVE_OPENSSL_CMAC_H
# define ENABLE_CMAC
#endif
extern void arc4random_buf(void *buf, size_t nbytes);

/*
 * Keywords and functions that Microsoft maps
 * to other names
 */
#define inline		__inline
#define stricmp		_stricmp
#define strcasecmp	_stricmp
#define isascii		__isascii
#define finite		_finite
#define random		rand
#define srandom		srand
#define fdopen		_fdopen
#define read		_read
#define open		_open
#ifndef close
#define close		_close
#endif
#define write		_write
#define strdup		_strdup
#define alloca		_alloca
#define stat		_stat		/*struct stat from  <sys/stat.h> */
#define fstat		_fstat
#define unlink		_unlink
/*
 * punt on fchmod on Windows
 */
#define fchmod(x,y)	{}
#define lseek		_lseek
#define pipe		_pipe
#define dup2		_dup2
/*
 * scale, unix sleep is seconds, Windows Sleep is msec
 */
#define sleep(x)	Sleep((unsigned)(x) * 1000)
#define fileno		_fileno
#define isatty		_isatty
#define mktemp		_mktemp
#define getpid		_getpid
#define timegm		_mkgmtime
#define errno_to_str	isc__strerror
/*
 * symbol returning the name of the current function
 */
#define __func__	__FUNCTION__

typedef int	pid_t;	/* PID is an int */

/*
 * Map the stream to the file number
 */
#define STDOUT_FILENO	_fileno(stdout)
#define STDERR_FILENO	_fileno(stderr)

/*
 * To minimize Windows-specific changes to the rest of the NTP code,
 * particularly reference clocks, ntp_stdlib.h will
 *
 * #define strerror(e) ntp_strerror(e)
 *
 * to deal with our mixture of C runtime (open, write) and Windows
 * (sockets, serial ports) error codes.  This is an ugly hack because
 * both use the lowest values differently, but particularly for ntpd,
 * it's not a problem.
 */
#define NTP_REDEFINE_STRERROR

#define MCAST				/* Enable Multicast Support */
#define MULTICAST_NONEWSOCKET		/* Don't create a new socket for mcast address */

# define REFCLOCK			/* from ntpd.mak */

/* #define CLOCK_PARSE  */
#define CLOCK_ACTS
#define CLOCK_ARBITER
#define CLOCK_ARCRON_MSF
#define OWN_PPS_NTP_TIMESTAMP_FROM_COUNTER	/* timepps.h */
#define HAVE_TIMEPPS_H
#define HAVE_PPSAPI
#define CLOCK_ATOM
#define CLOCK_CHU
#define CLOCK_CHRONOLOG
#define CLOCK_DUMBCLOCK
#define CLOCK_HOPF_SERIAL	/* device 38, hopf DCF77/GPS serial line receiver  */
#define CLOCK_HOPF_PCI		/* device 39, hopf DCF77/GPS PCI-Bus receiver  */
#define CLOCK_JUPITER
#define CLOCK_LOCAL
#define CLOCK_NMEA
#define CLOCK_ONCORE
#define CLOCK_PALISADE		/* from ntpd.mak */
#define CLOCK_PARSE
/* parse component drivers */
#define CLOCK_COMPUTIME
#define CLOCK_DCF7000
#define CLOCK_HOPF6021
#define CLOCK_MEINBERG
#define CLOCK_RAWDCF
#define CLOCK_RCC8000
#define CLOCK_SCHMID
#define CLOCK_TRIMTAIP
#define CLOCK_TRIMTSIP
#define CLOCK_VARITEXT
#define CLOCK_WHARTON_400A
/* end parse component drivers */
/* # define CLOCK_SHM */
#define CLOCK_SPECTRACOM	/* refclock_wwvb.c */
#define CLOCK_TRIMBLEDC
#define CLOCK_TRUETIME

#define NTP_LITTLE_ENDIAN		/* from libntp.mak */
#define NTP_POSIX_SOURCE

#define SYSLOG_FILE			/* from libntp.mak */

#define HAVE_LONG_LONG_INT		1
#define HAVE_UNSIGNED_LONG_LONG_INT	1
#define HAVE_SIZE_T             1     
#define HAVE_PTRDIFF_T  		1

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define HAVE_WINT_T				1
#endif

# define SIZEOF_SIGNED_CHAR	1
# define SIZEOF_SHORT		2
# define SIZEOF_INT		4
# define SIZEOF_LONG		4
# define SIZEOF_LONG_LONG	8

/* libntp/snprintf.c doesn't know %I64d */
#define ISC_PLATFORM_QUADFORMAT "ll"

# define HAVE_ERRNO_H			1
# define HAVE_FCNTL_H			1
# define HAVE_LIMITS_H			1
# define HAVE_STDARG_H			1
# define HAVE_SYS_RESOURCE_H		1
# define HAVE_SYS_TIME_H		1
# define HAVE_TERMIOS_H			1

# define HAVE_ALLOCA			1
# define HAVE_GETCLOCK			1
# define HAVE_MEMMOVE			1
# define HAVE_MKTIME			1
# define HAVE_SETVBUF			1
# define HAVE_STRCHR			1	/* for libopts */
# define HAVE_STRDUP			1
# define HAVE_STRNLEN			1
# define HAVE_MEMCHR			1
# define HAVE_TIMEGM			1	/* actually _mkgmtime */

# define HAVE_STRUCT_TIMESPEC
# define HAVE_IO_COMPLETION_PORT
# define ISC_PLATFORM_NEEDNTOP
# define ISC_PLATFORM_NEEDPTON

#define HAVE_BSD_NICE			/* emulate BSD setpriority() */

#define HW_WANT_RPL_VSNPRINTF		1
#define vsnprintf			rpl_vsnprintf
#include <stdarg.h>
int rpl_vsnprintf(char *, size_t, const char *, va_list);
#define HW_WANT_RPL_SNPRINTF		1
#define snprintf			rpl_snprintf
int rpl_snprintf(char *, size_t, const char *, ...);
#define HAVE_VSNPRINTF			1
#define HAVE_SNPRINTF			1

typedef char *caddr_t;

#ifdef _WCTYPE_T_DEFINED	/* see vc/include/crtdefs.h */
#define HAVE_WINT_T			1
#endif

#ifndef _INTPTR_T_DEFINED
typedef long intptr_t;
#define _INTPTR_T_DEFINED
#endif
#define HAVE_INTPTR_T			1

#ifndef _UINTPTR_T_DEFINED
typedef unsigned long uintptr_t;
#define _UINTPTR_T_DEFINED
#endif
#define HAVE_UINTPTR_T			1

#if !defined( _W64 )
  /*
   * if DWORD_PTR needs to be defined then the build environment
   * is pure 32 bit Windows. Since DWORD_PTR and DWORD have 
   * the same size in 32 bit Windows we can safely define
   * a replacement.
   */
  typedef DWORD DWORD_PTR;
#endif

#define NEED_S_CHAR_TYPEDEF


/* C99 exact size integer support. */
#if defined(_MSC_VER) && _MSC_VER<1800
# define MISSING_INTTYPES_H         1  /* not provided by VS2012 and earlier */
# define MISSING_STDBOOL_H          1  /* not provided by VS2012 and earlier */
# define MISSING_C99_STRUCT_INIT    1  /* see [Bug 2728] */
#else
#if defined(_MSC_VER) && _MSC_VER>=1800
/* VS2013 and above support C99 types */
# define HAVE_INT8_T                1
# define HAVE_UINT8_T               1
# define HAVE_INT16_T               1
# define HAVE_UINT16_T              1
# define HAVE_INT32_T               1
# define HAVE_UINT32_T              1
#endif
#endif

#if !defined (MISSING_STDBOOL_H)
# define HAVE_STDBOOL_H
#endif
#if !defined(MISSING_INTTYPES_H)
# define HAVE_INTTYPES_H            1
#elif !defined(MISSING_STDINT_H)
# define HAVE_STDINT_H              1
#elif !defined(ADDED_EXACT_SIZE_INTEGERS)
# define ADDED_EXACT_SIZE_INTEGERS  1
  typedef __int8 int8_t;
  typedef unsigned __int8 uint8_t;

  typedef __int16 int16_t;
  typedef unsigned __int16 uint16_t;

  typedef __int32 int32_t;
  typedef unsigned __int32 uint32_t;

  typedef __int64 int64_t;
  typedef unsigned __int64 uint64_t;
#endif

#ifdef _WIN64		/* mirroring SIZE_MAX from limits.h */
  typedef SSIZE_T	ssize_t;
#define SSIZE_MAX	_I64_MAX
#else
  typedef int		ssize_t;
#define SSIZE_MAX	INT_MAX
#endif

/* Directory separator, usually / or \ */
#define	DIR_SEP	'\\'

#define	POSIX_SHELL	"/bin/sh"	/* libopts/makeshell.c */

#define ULONG_CONST(a) a ## UL

#define NOKMEM
#define RETSIGTYPE void

#ifndef STR_SYSTEM
#define STR_SYSTEM "Windows"
#endif

#ifndef STR_PROCESSOR

#define STRINGIZE(arg)	#arg

#ifdef _M_IX86
#ifndef _M_IX86_FP
#define STR_PROCESSOR "x86"
#else
#if !_M_IX86_FP 
#define STR_PROCESSOR "x86"
#else 
#if _M_IX86_FP > 2
#define STR_PROCESSOR "x86-FP-" STRINGIZE(_M_IX86_FP)
#else
#if _M_IX86_FP == 2
#define STR_PROCESSOR "x86-SSE2"
#else
#define STR_PROCESSOR "x86-SSE"
#endif /* _M_IX86 == 2 */
#endif /* _M_IX86_FP > 2 */
#endif /* !_M_IX86_FP */
#endif /* !defined(_M_IX86_FP) */
#endif /* !defined(_M_IX86) */

#ifdef _M_IA64
#define STR_PROCESSOR "Itanium"
#endif

#ifdef _M_X64
#define STR_PROCESSOR "x64"
#endif

#endif /* !defined(STR_PROCESSOR) */

#undef STRINGIZE

#define  SIOCGIFFLAGS SIO_GET_INTERFACE_LIST /* used in ntp_io.c */

/* Bug 2978 mitigation -- unless defined elsewhere, do it here */
#ifndef DYNAMIC_INTERLEAVE
# define DYNAMIC_INTERLEAVE 0
#endif

/*
 * Macro to use in otherwise-empty source files to comply with ANSI C
 * requirement that each translation unit (source file) contain some
 * declaration.  This has commonly been done by declaring an unused
 * global variable of type int or char.  An extern reference to abs()
 * serves the same purpose without bloat.  We once used exit() but
 * that can produce warnings on systems that declare exit() noreturn.
 */
#define	NONEMPTY_TRANSLATION_UNIT	extern int abs(int);

/*
 * Below this line are includes which must happen after the bulk of
 * config.h is processed.  If you need to add another #include to this
 * file the preferred location is near the top, above the similar
 * line of hyphens.
 * ---------------------------------------------------------------------
 */

/*
 * Include standard stat information
 */
#include <isc/stat.h>

#endif	/* CONFIG_H */
