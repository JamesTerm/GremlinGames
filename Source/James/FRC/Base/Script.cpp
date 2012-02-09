#if 0
#include "Base_Includes.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>


#define LUA_API
#define LUALIB_API
#define LUAI_FUNC
#define ASSERT(x)


//#include "luaconf.h"---------------------------------------------------------------------------------------------------
/*
@@ LUA_ANSI controls the use of non-ansi features.
** CHANGE it (define it) if you want Lua to avoid the use of any
** non-ansi feature or library.
*/
#if defined(__STRICT_ANSI__)
#define LUA_ANSI
#endif


#if !defined(LUA_ANSI) && defined(_WIN32)
#define LUA_WIN
#endif

#if defined(LUA_USE_LINUX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#define LUA_USE_READLINE	/* needs some extra libraries */
#endif

#if defined(LUA_USE_MACOSX)
#define LUA_USE_POSIX
#define LUA_DL_DYLD		/* does not need extra library */
#endif



/*
@@ LUA_USE_POSIX includes all functionallity listed as X/Open System
@* Interfaces Extension (XSI).
** CHANGE it (define it) if your system is XSI compatible.
*/
#if defined(LUA_USE_POSIX)
#define LUA_USE_MKSTEMP
#define LUA_USE_ISATTY
#define LUA_USE_POPEN
#define LUA_USE_ULONGJMP
#endif


/*
@@ LUA_PATH and LUA_CPATH are the names of the environment variables that
@* Lua check to set its paths.
@@ LUA_INIT is the name of the environment variable that Lua
@* checks for initialization code.
** CHANGE them if you want different names.
*/
#define LUA_PATH        "LUA_PATH"
#define LUA_CPATH       "LUA_CPATH"
#define LUA_INIT	"LUA_INIT"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
@* Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
@* C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/
#if defined(_WIN32)
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LUA_LDIR	"!\\lua\\"
#define LUA_CDIR	"!\\"
#define LUA_PATH_DEFAULT  \
	".\\?.lua;"  LUA_LDIR"?.lua;"  LUA_LDIR"?\\init.lua;" \
	LUA_CDIR"?.lua;"  LUA_CDIR"?\\init.lua"
#define LUA_CPATH_DEFAULT \
	".\\?.dll;"  LUA_CDIR"?.dll;" LUA_CDIR"loadall.dll"

#else
#define LUA_ROOT	"/usr/local/"
#define LUA_LDIR	LUA_ROOT "share/lua/5.1/"
#define LUA_CDIR	LUA_ROOT "lib/lua/5.1/"
#define LUA_PATH_DEFAULT  \
	"./?.lua;"  LUA_LDIR"?.lua;"  LUA_LDIR"?/init.lua;" \
	LUA_CDIR"?.lua;"  LUA_CDIR"?/init.lua"
#define LUA_CPATH_DEFAULT \
	"./?.so;"  LUA_CDIR"?.so;" LUA_CDIR"loadall.so"
#endif


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#if defined(_WIN32)
#define LUA_DIRSEP	"\\"
#else
#define LUA_DIRSEP	"/"
#endif


/*
@@ LUA_PATHSEP is the character that separates templates in a path.
@@ LUA_PATH_MARK is the string that marks the substitution points in a
@* template.
@@ LUA_EXECDIR in a Windows path is replaced by the executable's
@* directory.
@@ LUA_IGMARK is a mark to ignore all before it when bulding the
@* luaopen_ function name.
** CHANGE them if for some reason your system cannot use those
** characters. (E.g., if one of those characters is a common character
** in file/directory names.) Probably you do not need to change them.
*/
#define LUA_PATHSEP	";"
#define LUA_PATH_MARK	"?"
#define LUA_EXECDIR	"!"
#define LUA_IGMARK	"-"


/*
@@ LUA_INTEGER is the integral type used by lua_pushinteger/lua_tointeger.
** CHANGE that if ptrdiff_t is not adequate on your machine. (On most
** machines, ptrdiff_t gives a good choice between int or long.)
*/
#define LUA_INTEGER	ptrdiff_t


/*
@@ LUA_API is a mark for all core API functions.
@@ LUALIB_API is a mark for all standard library functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** LUA_BUILD_AS_DLL to get it).
*/
#if defined(LUA_BUILD_AS_DLL)

#if defined(LUA_CORE) || defined(LUA_LIB)
#define LUA_API __declspec(dllexport)
#else
#define LUA_API __declspec(dllimport)
#endif

#else

//#define LUA_API		extern

#endif

/* more often than not the libs go together with the core */
//#define LUALIB_API	LUA_API


/*
@@ LUAI_FUNC is a mark for all extern functions that are not to be
@* exported to outside modules.
@@ LUAI_DATA is a mark for all extern (const) variables that are not to
@* be exported to outside modules.
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when Lua is compiled as a shared library.
*/
#if defined(luaall_c)
#define LUAI_FUNC	static
#define LUAI_DATA	/* empty */

#elif defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
	defined(__ELF__)
//#define LUAI_FUNC	__attribute__((visibility("hidden"))) extern
#define LUAI_DATA	LUAI_FUNC

#else
#define LUAI_FUNC	extern
#define LUAI_DATA	extern
#endif



/*
@@ LUA_QL describes how error messages quote program elements.
** CHANGE it if you want a different appearance.
*/
#define LUA_QL(x)	"'" x "'"
#define LUA_QS		LUA_QL("%s")


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
@* of a function in debug information.
** CHANGE it if you want a different size.
*/
#define LUA_IDSIZE	60


/*
** {==================================================================
** Stand-alone configuration
** ===================================================================
*/

#if defined(lua_c) || defined(luaall_c)

/*
@@ lua_stdin_is_tty detects whether the standard input is a 'tty' (that
@* is, whether we're running lua interactively).
** CHANGE it if you have a better definition for non-POSIX/non-Windows
** systems.
*/
#if defined(LUA_USE_ISATTY)
#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)
#elif defined(LUA_WIN)
#include <io.h>
#include <stdio.h>
#define lua_stdin_is_tty()	_isatty(_fileno(stdin))
#else
#define lua_stdin_is_tty()	1  /* assume stdin is a tty */
#endif


/*
@@ LUA_PROMPT is the default prompt used by stand-alone Lua.
@@ LUA_PROMPT2 is the default continuation prompt used by stand-alone Lua.
** CHANGE them if you want different prompts. (You can also change the
** prompts dynamically, assigning to globals _PROMPT/_PROMPT2.)
*/
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "


/*
@@ LUA_PROGNAME is the default name for the stand-alone Lua program.
** CHANGE it if your stand-alone interpreter has a different name and
** your system is not able to detect that name automatically.
*/
#define LUA_PROGNAME		"lua"


/*
@@ LUA_MAXINPUT is the maximum length for an input line in the
@* stand-alone interpreter.
** CHANGE it if you need longer lines.
*/
#define LUA_MAXINPUT	512


/*
@@ lua_readline defines how to show a prompt and then read a line from
@* the standard input.
@@ lua_saveline defines how to "save" a read line in a "history".
@@ lua_freeline defines how to free a line read by lua_readline.
** CHANGE them if you want to improve this functionality (e.g., by using
** GNU readline and history facilities).
*/
#if defined(LUA_USE_READLINE)
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#define lua_readline(L,b,p)	((void)L, ((b)=readline(p)) != NULL)
#define lua_saveline(L,idx) \
	if (lua_strlen(L,idx) > 0)  /* non-empty line? */ \
	add_history(lua_tostring(L, idx));  /* add it to history */
#define lua_freeline(L,b)	((void)L, free(b))
#else
#define lua_readline(L,b,p)	\
	((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
	fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#define lua_saveline(L,idx)	{ (void)L; (void)idx; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }
#endif

#endif

/* }================================================================== */


/*
@@ LUAI_GCPAUSE defines the default pause between garbage-collector cycles
@* as a percentage.
** CHANGE it if you want the GC to run faster or slower (higher values
** mean larger pauses which mean slower collection.) You can also change
** this value dynamically.
*/
#define LUAI_GCPAUSE	200  /* 200% (wait memory to double before next GC) */


/*
@@ LUAI_GCMUL defines the default speed of garbage collection relative to
@* memory allocation as a percentage.
** CHANGE it if you want to change the granularity of the garbage
** collection. (Higher values mean coarser collections. 0 represents
** infinity, where each step performs a full collection.) You can also
** change this value dynamically.
*/
#define LUAI_GCMUL	200 /* GC runs 'twice the speed' of memory allocation */



/*
@@ LUA_COMPAT_GETN controls compatibility with old getn behavior.
** CHANGE it (define it) if you want exact compatibility with the
** behavior of setn/getn in Lua 5.0.
*/
#undef LUA_COMPAT_GETN

/*
@@ LUA_COMPAT_LOADLIB controls compatibility about global loadlib.
** CHANGE it to undefined as soon as you do not need a global 'loadlib'
** function (the function is still available as 'package.loadlib').
*/
#undef LUA_COMPAT_LOADLIB

/*
@@ LUA_COMPAT_VARARG controls compatibility with old vararg feature.
** CHANGE it to undefined as soon as your programs use only '...' to
** access vararg parameters (instead of the old 'arg' table).
*/
#define LUA_COMPAT_VARARG

/*
@@ LUA_COMPAT_MOD controls compatibility with old math.mod function.
** CHANGE it to undefined as soon as your programs use 'math.fmod' or
** the new '%' operator instead of 'math.mod'.
*/
#define LUA_COMPAT_MOD

/*
@@ LUA_COMPAT_LSTR controls compatibility with old long string nesting
@* facility.
** CHANGE it to 2 if you want the old behaviour, or undefine it to turn
** off the advisory error when nesting [[...]].
*/
#define LUA_COMPAT_LSTR		1

/*
@@ LUA_COMPAT_GFIND controls compatibility with old 'string.gfind' name.
** CHANGE it to undefined as soon as you rename 'string.gfind' to
** 'string.gmatch'.
*/
#define LUA_COMPAT_GFIND

/*
@@ LUA_COMPAT_OPENLIB controls compatibility with old 'luaL_openlib'
@* behavior.
** CHANGE it to undefined as soon as you replace to 'luaL_register'
** your uses of 'luaL_openlib'
*/
#define LUA_COMPAT_OPENLIB



/*
@@ luai_apicheck is the assert macro used by the Lua-C API.
** CHANGE luai_apicheck if you want Lua to perform some checks in the
** parameters it gets from API calls. This may slow down the interpreter
** a bit, but may be quite useful when debugging C code that interfaces
** with Lua. A useful redefinition is to use assert.h.
*/
#if defined(LUA_USE_APICHECK)
#include <assert.h>
#define luai_apicheck(L,o)	{ (void)L; assert(o); }
#else
#define luai_apicheck(L,o)	{ (void)L; }
#endif


/*
@@ LUAI_BITSINT defines the number of bits in an int.
** CHANGE here if Lua cannot automatically detect the number of bits of
** your machine. Probably you do not need to change this.
*/
/* avoid overflows in comparison */
#if INT_MAX-20 < 32760
#define LUAI_BITSINT	16
#elif INT_MAX > 2147483640L
/* int has at least 32 bits */
#define LUAI_BITSINT	32
#else
#error "you must define LUA_BITSINT with number of bits in an integer"
#endif


/*
@@ LUAI_UINT32 is an unsigned integer with at least 32 bits.
@@ LUAI_INT32 is an signed integer with at least 32 bits.
@@ LUAI_UMEM is an unsigned integer big enough to count the total
@* memory used by Lua.
@@ LUAI_MEM is a signed integer big enough to count the total memory
@* used by Lua.
** CHANGE here if for some weird reason the default definitions are not
** good enough for your machine. (The definitions in the 'else'
** part always works, but may waste space on machines with 64-bit
** longs.) Probably you do not need to change this.
*/
#if LUAI_BITSINT >= 32
#define LUAI_UINT32	unsigned int
#define LUAI_INT32	int
#define LUAI_MAXINT32	INT_MAX
#define LUAI_UMEM	size_t
#define LUAI_MEM	ptrdiff_t
#else
/* 16-bit ints */
#define LUAI_UINT32	unsigned long
#define LUAI_INT32	long
#define LUAI_MAXINT32	LONG_MAX
#define LUAI_UMEM	unsigned long
#define LUAI_MEM	long
#endif


/*
@@ LUAI_MAXCALLS limits the number of nested calls.
** CHANGE it if you need really deep recursive calls. This limit is
** arbitrary; its only purpose is to stop infinite recursion before
** exhausting memory.
*/
#define LUAI_MAXCALLS	20000


/*
@@ LUAI_MAXCSTACK limits the number of Lua stack slots that a C function
@* can use.
** CHANGE it if you need lots of (Lua) stack space for your C
** functions. This limit is arbitrary; its only purpose is to stop C
** functions to consume unlimited stack space.
*/
#define LUAI_MAXCSTACK	2048



/*
** {==================================================================
** CHANGE (to smaller values) the following definitions if your system
** has a small C stack. (Or you may want to change them to larger
** values if your system has a large C stack and these limits are
** too rigid for you.) Some of these constants control the size of
** stack-allocated arrays used by the compiler or the interpreter, while
** others limit the maximum number of recursive calls that the compiler
** or the interpreter can perform. Values too large may cause a C stack
** overflow for some forms of deep constructs.
** ===================================================================
*/


/*
@@ LUAI_MAXCCALLS is the maximum depth for nested C calls (short) and
@* syntactical nested non-terminals in a program.
*/
#define LUAI_MAXCCALLS		200


/*
@@ LUAI_MAXVARS is the maximum number of local variables per function
@* (must be smaller than 250).
*/
#define LUAI_MAXVARS		200


/*
@@ LUAI_MAXUPVALUES is the maximum number of upvalues per function
@* (must be smaller than 250).
*/
#define LUAI_MAXUPVALUES	60


/*
@@ LUAL_BUFFERSIZE is the buffer size used by the lauxlib buffer system.
*/
#define LUAL_BUFFERSIZE		BUFSIZ

/* }================================================================== */




/*
** {==================================================================
@@ LUA_NUMBER is the type of numbers in Lua.
** CHANGE the following definitions only if you want to build Lua
** with a number type different from double. You may also need to
** change lua_number2int & lua_number2integer.
** ===================================================================
*/

#define LUA_NUMBER_DOUBLE
#define LUA_NUMBER	double

/*
@@ LUAI_UACNUMBER is the result of an 'usual argument conversion'
@* over a number.
*/
#define LUAI_UACNUMBER	double


/*
@@ LUA_NUMBER_SCAN is the format for reading numbers.
@@ LUA_NUMBER_FMT is the format for writing numbers.
@@ lua_number2str converts a number to a string.
@@ LUAI_MAXNUMBER2STR is maximum size of previous conversion.
@@ lua_str2number converts a string to a number.
*/
#define LUA_NUMBER_SCAN		"%lf"
#define LUA_NUMBER_FMT		"%.14g"
#define lua_number2str(s,n)	sprintf((s), LUA_NUMBER_FMT, (n))
#define LUAI_MAXNUMBER2STR	32 /* 16 digits, sign, point, and \0 */
#define lua_str2number(s,p)	strtod((s), (p))


/*
@@ The luai_num* macros define the primitive operations over numbers.
*/
#if defined(LUA_CORE)
#include <math.h>
#define luai_numadd(a,b)	((a)+(b))
#define luai_numsub(a,b)	((a)-(b))
#define luai_nummul(a,b)	((a)*(b))
#define luai_numdiv(a,b)	((a)/(b))
#define luai_nummod(a,b)	((a) - floor((a)/(b))*(b))
#define luai_numpow(a,b)	(pow(a,b))
#define luai_numunm(a)		(-(a))
#define luai_numeq(a,b)		((a)==(b))
#define luai_numlt(a,b)		((a)<(b))
#define luai_numle(a,b)		((a)<=(b))
#define luai_numisnan(a)	(!luai_numeq((a), (a)))
#endif


/*
@@ lua_number2int is a macro to convert lua_Number to int.
@@ lua_number2integer is a macro to convert lua_Number to lua_Integer.
** CHANGE them if you know a faster way to convert a lua_Number to
** int (with any rounding method and without throwing errors) in your
** system. In Pentium machines, a naive typecast from double to int
** in C is extremely slow, so any alternative is worth trying.
*/

/* On a Pentium, resort to a trick */
#if defined(LUA_NUMBER_DOUBLE) && !defined(LUA_ANSI) && !defined(__SSE2__) && \
	(defined(__i386) || defined (_M_IX86) || defined(__i386__))

/* On a Microsoft compiler, use assembler */
#if defined(_MSC_VER)

#define lua_number2int(i,d)   __asm fld d   __asm fistp i
#define lua_number2integer(i,n)		lua_number2int(i, n)

/* the next trick should work on any Pentium, but sometimes clashes
with a DirectX idiosyncrasy */
#else

union luai_Cast { double l_d; long l_l; };
#define lua_number2int(i,d) \
{ volatile union luai_Cast u; u.l_d = (d) + 6755399441055744.0; (i) = u.l_l; }
#define lua_number2integer(i,n)		lua_number2int(i, n)

#endif


/* this option always works, but may be slow */
#else
#define lua_number2int(i,d)	((i)=(int)(d))
#define lua_number2integer(i,d)	((i)=(lua_Integer)(d))

#endif

/* }================================================================== */


/*
@@ LUAI_USER_ALIGNMENT_T is a type that requires maximum alignment.
** CHANGE it if your system requires alignments larger than double. (For
** instance, if your system supports long doubles and they must be
** aligned in 16-byte boundaries, then you should add long double in the
** union.) Probably you do not need to change this.
*/
#define LUAI_USER_ALIGNMENT_T	union { double u; void *s; long l; }


/*
@@ LUAI_THROW/LUAI_TRY define how Lua does exception handling.
** CHANGE them if you prefer to use longjmp/setjmp even with C++
** or if want/don't to use _longjmp/_setjmp instead of regular
** longjmp/setjmp. By default, Lua handles errors with exceptions when
** compiling as C++ code, with _longjmp/_setjmp when asked to use them,
** and with longjmp/setjmp otherwise.
*/
#if defined(__cplusplus)
/* C++ exceptions */
#define LUAI_THROW(L,c)	throw(c)
#define LUAI_TRY(L,c,a)	try { a } catch(...) \
	{ if ((c)->status == 0) (c)->status = -1; }
#define luai_jmpbuf	int  /* dummy variable */

#elif defined(LUA_USE_ULONGJMP)
/* in Unix, try _longjmp/_setjmp (more efficient) */
#define LUAI_THROW(L,c)	_longjmp((c)->b, 1)
#define LUAI_TRY(L,c,a)	if (_setjmp((c)->b) == 0) { a }
#define luai_jmpbuf	jmp_buf

#else
/* default handling with long jumps */
#define LUAI_THROW(L,c)	longjmp((c)->b, 1)
#define LUAI_TRY(L,c,a)	if (setjmp((c)->b) == 0) { a }
#define luai_jmpbuf	jmp_buf

#endif


/*
@@ LUA_MAXCAPTURES is the maximum number of captures that a pattern
@* can do during pattern-matching.
** CHANGE it if you need more captures. This limit is arbitrary.
*/
#define LUA_MAXCAPTURES		32


/*
@@ lua_tmpnam is the function that the OS library uses to create a
@* temporary name.
@@ LUA_TMPNAMBUFSIZE is the maximum size of a name created by lua_tmpnam.
** CHANGE them if you have an alternative to tmpnam (which is considered
** insecure) or if you want the original tmpnam anyway.  By default, Lua
** uses tmpnam except when POSIX is available, where it uses mkstemp.
*/
#if defined(loslib_c) || defined(luaall_c)

#if defined(LUA_USE_MKSTEMP)
#include <unistd.h>
#define LUA_TMPNAMBUFSIZE	32
#define lua_tmpnam(b,e)	{ \
	strcpy(b, "/tmp/lua_XXXXXX"); \
	e = mkstemp(b); \
	if (e != -1) close(e); \
	e = (e == -1); }

#else
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == NULL); }
#endif

#endif


/*
@@ lua_popen spawns a new process connected to the current one through
@* the file streams.
** CHANGE it if you have a way to implement it in your system.
*/
#if defined(LUA_USE_POPEN)

#define lua_popen(L,c,m)	((void)L, popen(c,m))
#define lua_pclose(L,file)	((void)L, (pclose(file) != -1))

#elif defined(LUA_WIN)

#define lua_popen(L,c,m)	((void)L, _popen(c,m))
#define lua_pclose(L,file)	((void)L, (_pclose(file) != -1))

#else

#define lua_popen(L,c,m)	((void)((void)c, m),  \
	luaL_error(L, LUA_QL("popen") " not supported"), (FILE*)0)
#define lua_pclose(L,file)		((void)((void)L, file), 0)

#endif

/*
@@ LUA_DL_* define which dynamic-library system Lua should use.
** CHANGE here if Lua has problems choosing the appropriate
** dynamic-library system for your platform (either Windows' DLL, Mac's
** dyld, or Unix's dlopen). If your system is some kind of Unix, there
** is a good chance that it has dlopen, so LUA_DL_DLOPEN will work for
** it.  To use dlopen you also need to adapt the src/Makefile (probably
** adding -ldl to the linker options), so Lua does not select it
** automatically.  (When you change the makefile to add -ldl, you must
** also add -DLUA_USE_DLOPEN.)
** If you do not want any kind of dynamic library, undefine all these
** options.
** By default, _WIN32 gets LUA_DL_DLL and MAC OS X gets LUA_DL_DYLD.
*/
#if defined(LUA_USE_DLOPEN)
#define LUA_DL_DLOPEN
#endif

#if defined(LUA_WIN)
#define LUA_DL_DLL
#endif


/*
@@ LUAI_EXTRASPACE allows you to add user-specific data in a lua_State
@* (the data goes just *before* the lua_State pointer).
** CHANGE (define) this if you really need that. This value must be
** a multiple of the maximum alignment required for your machine.
*/
#define LUAI_EXTRASPACE		0


/*
@@ luai_userstate* allow user-specific actions on threads.
** CHANGE them if you defined LUAI_EXTRASPACE and need to do something
** extra when a thread is created/deleted/resumed/yielded.
*/
#define luai_userstateopen(L)		((void)L)
#define luai_userstateclose(L)		((void)L)
#define luai_userstatethread(L,L1)	((void)L)
#define luai_userstatefree(L)		((void)L)
#define luai_userstateresume(L,n)	((void)L)
#define luai_userstateyield(L,n)	((void)L)


/*
@@ LUA_INTFRMLEN is the length modifier for integer conversions
@* in 'string.format'.
@@ LUA_INTFRM_T is the integer type correspoding to the previous length
@* modifier.
** CHANGE them if your system supports long long or does not support long.
*/

#if defined(LUA_USELONGLONG)

#define LUA_INTFRMLEN		"ll"
#define LUA_INTFRM_T		long long

#else

#define LUA_INTFRMLEN		"l"
#define LUA_INTFRM_T		long

#endif



/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/






//#include "lua.h"---------------------------------------------------------------------------------------------------
#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.2"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2007 Lua.org, PUC-Rio"
#define LUA_AUTHORS 	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"

/* mark for precompiled code (`<esc>Lua') */
#define	LUA_SIGNATURE	"\033Lua"

/* option for multiple returns in `lua_pcall' and `lua_call' */
#define LUA_MULTRET	(-1)


/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* thread status; 0 is OK */
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


typedef struct lua_State lua_State;

typedef int (*lua_CFunction) (lua_State *L);


/*
** functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);


/*
** prototype for memory-allocation functions
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);

/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20

/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;


/* type for integer functions */
typedef LUA_INTEGER lua_Integer;

/* 
** ===============================================================
** some useful macros
** ===============================================================
*/

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)		lua_objlen(L, (i))

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)

/*
** compatibility macros and functions
*/

#define lua_open()	luaL_newstate()

#define lua_getregistry(L)	lua_pushvalue(L, LUA_REGISTRYINDEX)

#define lua_getgccount(L)	lua_gc(L, LUA_GCCOUNT, 0)

#define lua_Chunkreader		lua_Reader
#define lua_Chunkwriter		lua_Writer

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct lua_Debug lua_Debug;  /* activation record */


/* Functions to be called by the debuger in specific events */
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n);

LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook lua_gethook (lua_State *L);
LUA_API int lua_gethookmask (lua_State *L);
LUA_API int lua_gethookcount (lua_State *L);


struct lua_Debug {
	int event;
	const char *name;	/* (n) */
	const char *namewhat;	/* (n) `global', `local', `field', `method' */
	const char *what;	/* (S) `Lua', `C', `main', `tail' */
	const char *source;	/* (S) */
	int currentline;	/* (l) */
	int nups;		/* (u) number of upvalues */
	int linedefined;	/* (S) */
	int lastlinedefined;	/* (S) */
	char short_src[LUA_IDSIZE]; /* (S) */
	/* private part */
	int i_ci;  /* active function */
};


//#include "lapi.h"
//#include "ldebug.h"
//#include "lfunc.h"
//#include "lgc.h"


const char lua_ident[] =
"$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
"$Authors: " LUA_AUTHORS " $\n"
"$URL: www.lua.org $\n";


#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))
#define api_checkvalidindex(L, i)	api_check(L, (i) != luaO_nilobject)
#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}


//#include "lmem.h"---------------------------------------------------------------------------------------------------
#define MEMERRMSG	"not enough memory"


#define luaM_reallocv(L,b,on,n,e) \
	((cast(size_t, (n)+1) <= MAX_SIZET/(e)) ?  /* +1 to avoid warnings */ \
	luaM_realloc_(L, (b), (on)*(e), (n)*(e)) : \
	luaM_toobig(L))

#define luaM_freemem(L, b, s)	luaM_realloc_(L, (b), (s), 0)
#define luaM_free(L, b)		luaM_realloc_(L, (b), sizeof(*(b)), 0)
#define luaM_freearray(L, b, n, t)   luaM_reallocv(L, (b), n, 0, sizeof(t))

#define luaM_malloc(L,t)	luaM_realloc_(L, NULL, 0, (t))
#define luaM_new(L,t)		cast(t *, luaM_malloc(L, sizeof(t)))
#define luaM_newvector(L,n,t) \
	cast(t *, luaM_reallocv(L, NULL, 0, n, sizeof(t)))

#define luaM_growvector(L,v,nelems,size,t,limit,e) \
	if ((nelems)+1 > (size)) \
	((v)=cast(t *, luaM_growaux_(L,v,&(size),sizeof(t),limit,e)))

#define luaM_reallocvector(L, v,oldn,n,t) \
	((v)=cast(t *, luaM_reallocv(L, v, oldn, n, sizeof(t))))


LUAI_FUNC void *luaM_realloc_ (lua_State *L, void *block, size_t oldsize,
							   size_t size);
LUAI_FUNC void *luaM_toobig (lua_State *L);
LUAI_FUNC void *luaM_growaux_ (lua_State *L, void *block, int *size,
							   size_t size_elem, int limit,
							   const char *errormsg);

//#include "limits.h"---------------------------------------------------------------------------------------------------
typedef LUAI_UINT32 lu_int32;

typedef LUAI_UMEM lu_mem;

typedef LUAI_MEM l_mem;



/* chars used as small naturals (so that `char' is reserved for characters) */
typedef unsigned char lu_byte;


#define MAX_SIZET	((size_t)(~(size_t)0)-2)

#define MAX_LUMEM	((lu_mem)(~(lu_mem)0)-2)


#define MAX_INT (INT_MAX-2)  /* maximum value of an int (-2 for safety) */

/*
** conversion of pointer to integer
** this is for hashing only; there is no problem if the integer
** cannot hold the whole pointer value
*/
#define IntPoint(p)  ((unsigned int)(lu_mem)(p))



/* type to ensure maximum alignment */
typedef LUAI_USER_ALIGNMENT_T L_Umaxalign;


/* result of a `usual argument conversion' over lua_Number */
typedef LUAI_UACNUMBER l_uacNumber;


/* internal assertions for in-house debugging */
#ifdef lua_assert

#define check_exp(c,e)		(lua_assert(c), (e))
#define api_check(l,e)		lua_assert(e)

#else

#define lua_assert(c)		((void)0)
#define check_exp(c,e)		(e)
#define api_check		luai_apicheck

#endif


#ifndef UNUSED
#define UNUSED(x)	((void)(x))	/* to avoid warnings */
#endif


#ifndef cast
#define cast(t, exp)	((t)(exp))
#endif

#define cast_byte(i)	cast(lu_byte, (i))
#define cast_num(i)	cast(lua_Number, (i))
#define cast_int(i)	cast(int, (i))



/*
** type for virtual-machine instructions
** must be an unsigned with (at least) 4 bytes (see details in lopcodes.h)
*/
typedef lu_int32 Instruction;



/* maximum stack for a Lua function */
#define MAXSTACK	250



/* minimum size for the string table (must be power of 2) */
#ifndef MINSTRTABSIZE
#define MINSTRTABSIZE	32
#endif


/* minimum size for string buffer */
#ifndef LUA_MINBUFFER
#define LUA_MINBUFFER	32
#endif

#define lapi_c
#define LUA_CORE

#ifndef lua_lock
#define lua_lock(L)     ((void) 0) 
#define lua_unlock(L)   ((void) 0)
#endif

#ifndef luai_threadyield
#define luai_threadyield(L)     {lua_unlock(L); lua_lock(L);}
#endif


/*
** macro to control inclusion of some hard tests on stack reallocation
*/ 
#ifndef HARDSTACKTESTS
#define condhardstacktests(x)	((void)0)
#else
#define condhardstacktests(x)	x
#endif


//oops limits here?
#define luai_apicheck(L,o)	{ (void)L; }
#define api_check		luai_apicheck


#define CHAR_BIT      8         /* number of bits in a char */
#define SCHAR_MIN   (-128)      /* minimum signed char value */
#define SCHAR_MAX     127       /* maximum signed char value */
//#define UCHAR_MAX     0xff      /* maximum unsigned char value */

#ifndef _CHAR_UNSIGNED
//#define CHAR_MIN    SCHAR_MIN   /* mimimum char value */
//#define CHAR_MAX    SCHAR_MAX   /* maximum char value */
#else
#define CHAR_MIN      0
#define CHAR_MAX    UCHAR_MAX
#endif  /* _CHAR_UNSIGNED */

//#define MB_LEN_MAX    5             /* max. # bytes in multibyte char */
//#define SHRT_MIN    (-32768)        /* minimum (signed) short value */
//#define SHRT_MAX      32767         /* maximum (signed) short value */
//#define USHRT_MAX     0xffff        /* maximum unsigned short value */
//#define INT_MIN     (-2147483647 - 1) /* minimum (signed) int value */
//#define INT_MAX       2147483647    /* maximum (signed) int value */
//#define UINT_MAX      0xffffffff    /* maximum unsigned int value */
//#define LONG_MIN    (-2147483647L - 1) /* minimum (signed) long value */
//#define LONG_MAX      2147483647L   /* maximum (signed) long value */
//#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */
#define LLONG_MAX     9223372036854775807i64       /* maximum signed long long int value */
#define LLONG_MIN   (-9223372036854775807i64 - 1)  /* minimum signed long long int value */
#define ULLONG_MAX    0xffffffffffffffffui64       /* maximum unsigned long long int value */

#define _I8_MIN     (-127i8 - 1)    /* minimum signed 8 bit value */
#define _I8_MAX       127i8         /* maximum signed 8 bit value */
#define _UI8_MAX      0xffui8       /* maximum unsigned 8 bit value */

#define _I16_MIN    (-32767i16 - 1) /* minimum signed 16 bit value */
#define _I16_MAX      32767i16      /* maximum signed 16 bit value */
#define _UI16_MAX     0xffffui16    /* maximum unsigned 16 bit value */

#define _I32_MIN    (-2147483647i32 - 1) /* minimum signed 32 bit value */
#define _I32_MAX      2147483647i32 /* maximum signed 32 bit value */
#define _UI32_MAX     0xffffffffui32 /* maximum unsigned 32 bit value */

/* minimum signed 64 bit value */
#define _I64_MIN    (-9223372036854775807i64 - 1)
/* maximum signed 64 bit value */
#define _I64_MAX      9223372036854775807i64
/* maximum unsigned 64 bit value */
#define _UI64_MAX     0xffffffffffffffffui64

#if     _INTEGRAL_MAX_BITS >= 128
/* minimum signed 128 bit value */
#define _I128_MIN   (-170141183460469231731687303715884105727i128 - 1)
/* maximum signed 128 bit value */
#define _I128_MAX     170141183460469231731687303715884105727i128
/* maximum unsigned 128 bit value */
#define _UI128_MAX    0xffffffffffffffffffffffffffffffffui128
#endif

#ifndef SIZE_MAX
#ifdef _WIN64 
#define SIZE_MAX _UI64_MAX
#else
#define SIZE_MAX UINT_MAX
#endif
#endif

#if __STDC_WANT_SECURE_LIB__
/* While waiting to the C standard committee to finalize the decision on RSIZE_MAX and rsize_t,
* we define RSIZE_MAX as SIZE_MAX
*/
#ifndef RSIZE_MAX
#define RSIZE_MAX SIZE_MAX
#endif
#endif

//#include "lobject.h"---------------------------------------------------------------------------------------------------
/* tags for values visible from Lua */
#define LAST_TAG	LUA_TTHREAD

#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common header in struct form
*/
typedef struct GCheader {
	CommonHeader;
} GCheader;




/*
** Union of all Lua values
*/
typedef union {
	GCObject *gc;
	void *p;
	lua_Number n;
	int b;
} Value;


/*
** Tagged Values
*/

#define TValuefields	Value value; int tt

typedef struct lua_TValue {
	TValuefields;
} TValue;


/* Macros to test type */
#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)

/* Macros to access values */
#define ttype(o)	((o)->tt)
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define tsvalue(o)	(&rawtsvalue(o)->tsv)
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

/*
** for internal debug only
*/
#define checkconsistency(obj) \
	lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

#define checkliveness(g,obj) \
	lua_assert(!iscollectable(obj) || \
	((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))


/* Macros to set values */
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

#define setnvalue(obj,x) \
{ TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue(obj,x) \
{ TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
{ TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

#define setsvalue(L,obj,x) \
{ TValue *i_o=(obj); \
	i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
	checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
{ TValue *i_o=(obj); \
	i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
	checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
{ TValue *i_o=(obj); \
	i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
	checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
{ TValue *i_o=(obj); \
	i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
	checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
{ TValue *i_o=(obj); \
	i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
	checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
{ TValue *i_o=(obj); \
	i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
	checkliveness(G(L),i_o); }




#define setobj(L,obj1,obj2) \
{ const TValue *o2=(obj2); TValue *o1=(obj1); \
	o1->value = o2->value; o1->tt=o2->tt; \
	checkliveness(G(L),o1); }


/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))


#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table
*/
typedef union TString {
	L_Umaxalign dummy;  /* ensures maximum alignment for strings */
	struct {
		CommonHeader;
		lu_byte reserved;
		unsigned int hash;
		size_t len;
	} tsv;
} TString;


#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(tsvalue(o))



typedef union Udata {
	L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
	struct {
		CommonHeader;
		struct Table *metatable;
		struct Table *env;
		size_t len;
	} uv;
} Udata;




/*
** Function Prototypes
*/
typedef struct Proto {
	CommonHeader;
	TValue *k;  /* constants used by the function */
	Instruction *code;
	struct Proto **p;  /* functions defined inside the function */
	int *lineinfo;  /* map from opcodes to source lines */
	struct LocVar *locvars;  /* information about local variables */
	TString **upvalues;  /* upvalue names */
	TString  *source;
	int sizeupvalues;
	int sizek;  /* size of `k' */
	int sizecode;
	int sizelineinfo;
	int sizep;  /* size of `p' */
	int sizelocvars;
	int linedefined;
	int lastlinedefined;
	GCObject *gclist;
	lu_byte nups;  /* number of upvalues */
	lu_byte numparams;
	lu_byte is_vararg;
	lu_byte maxstacksize;
} Proto;


/* masks for new-style vararg */
#define VARARG_HASARG		1
#define VARARG_ISVARARG		2
#define VARARG_NEEDSARG		4


typedef struct LocVar {
	TString *varname;
	int startpc;  /* first point where variable is active */
	int endpc;    /* first point where variable is dead */
} LocVar;



/*
** Upvalues
*/

typedef struct UpVal {
	CommonHeader;
	TValue *v;  /* points to stack or to its own value */
	union {
		TValue value;  /* the value (when closed) */
		struct {  /* double linked list (when open) */
			struct UpVal *prev;
			struct UpVal *next;
		} l;
	} u;
} UpVal;


/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
struct Table *env

typedef struct CClosure {
	ClosureHeader;
	lua_CFunction f;
	TValue upvalue[1];
} CClosure;


typedef struct LClosure {
	ClosureHeader;
	struct Proto *p;
	UpVal *upvals[1];
} LClosure;


typedef union Closure {
	CClosure c;
	LClosure l;
} Closure;


#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->c.isC)


/*
** Tables
*/

typedef union TKey {
	struct {
		TValuefields;
		struct Node *next;  /* for chaining */
	} nk;
	TValue tvk;
} TKey;


typedef struct Node {
	TValue i_val;
	TKey i_key;
} Node;


typedef struct Table {
	CommonHeader;
	lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
	lu_byte lsizenode;  /* log2 of size of `node' array */
	struct Table *metatable;
	TValue *array;  /* array part */
	Node *node;
	Node *lastfree;  /* any free position is before this position */
	GCObject *gclist;
	int sizearray;  /* size of `array' array */
} Table;



/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

class TValue_Init : public lua_TValue
{
	public:
	TValue_Init()
		{
			memset(this,0,sizeof(TValue_Init));
		}
};

LUAI_DATA const TValue luaO_nilobject_=TValue_Init();

#define ceillog2(x)	(luaO_log2((x)-1) + 1)


//#include "lgc.h"-------------------------------------------------------------------------------------------------------

/*
** Possible states of the Garbage Collector
*/
#define GCSpause	0
#define GCSpropagate	1
#define GCSsweepstring	2
#define GCSsweep	3
#define GCSfinalize	4


/*
** some userful bit tricks
*/
#define resetbits(x,m)	((x) &= cast(lu_byte, ~(m)))
#define setbits(x,m)	((x) |= (m))
#define testbits(x,m)	((x) & (m))
#define bitmask(b)	(1<<(b))
#define bit2mask(b1,b2)	(bitmask(b1) | bitmask(b2))
#define l_setbit(x,b)	setbits(x, bitmask(b))
#define resetbit(x,b)	resetbits(x, bitmask(b))
#define testbit(x,b)	testbits(x, bitmask(b))
#define set2bits(x,b1,b2)	setbits(x, (bit2mask(b1, b2)))
#define reset2bits(x,b1,b2)	resetbits(x, (bit2mask(b1, b2)))
#define test2bits(x,b1,b2)	testbits(x, (bit2mask(b1, b2)))



/*
** Layout for bit use in `marked' field:
** bit 0 - object is white (type 0)
** bit 1 - object is white (type 1)
** bit 2 - object is black
** bit 3 - for userdata: has been finalized
** bit 3 - for tables: has weak keys
** bit 4 - for tables: has weak values
** bit 5 - object is fixed (should not be collected)
** bit 6 - object is "super" fixed (only the main thread)
*/


#define WHITE0BIT	0
#define WHITE1BIT	1
#define BLACKBIT	2
#define FINALIZEDBIT	3
#define KEYWEAKBIT	3
#define VALUEWEAKBIT	4
#define FIXEDBIT	5
#define SFIXEDBIT	6
#define WHITEBITS	bit2mask(WHITE0BIT, WHITE1BIT)


#define iswhite(x)      test2bits((x)->gch.marked, WHITE0BIT, WHITE1BIT)
#define isblack(x)      testbit((x)->gch.marked, BLACKBIT)
#define isgray(x)	(!isblack(x) && !iswhite(x))

#define otherwhite(g)	(g->currentwhite ^ WHITEBITS)
#define isdead(g,v)	((v)->gch.marked & otherwhite(g) & WHITEBITS)

#define changewhite(x)	((x)->gch.marked ^= WHITEBITS)
#define gray2black(x)	l_setbit((x)->gch.marked, BLACKBIT)

#define valiswhite(x)	(iscollectable(x) && iswhite(gcvalue(x)))

#define luaC_white(g)	cast(lu_byte, (g)->currentwhite & WHITEBITS)


#define luaC_checkGC(L) { \
	condhardstacktests(luaD_reallocstack(L, L->stacksize - EXTRA_STACK - 1)); \
	if (G(L)->totalbytes >= G(L)->GCthreshold) \
	luaC_step(L); }


#define luaC_barrier(L,p,v) { if (valiswhite(v) && isblack(obj2gco(p)))  \
	luaC_barrierf(L,obj2gco(p),gcvalue(v)); }

#define luaC_barriert(L,t,v) { if (valiswhite(v) && isblack(obj2gco(t)))  \
	luaC_barrierback(L,t); }

#define luaC_objbarrier(L,p,o)  \
{ if (iswhite(obj2gco(o)) && isblack(obj2gco(p))) \
	luaC_barrierf(L,obj2gco(p),obj2gco(o)); }

#define luaC_objbarriert(L,t,o)  \
{ if (iswhite(obj2gco(o)) && isblack(obj2gco(t))) luaC_barrierback(L,t); }


//#include "ltm.h"-------------------------------------------------------------------------------------------------------
/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM"
*/
typedef enum {
	TM_INDEX,
	TM_NEWINDEX,
	TM_GC,
	TM_MODE,
	TM_EQ,  /* last tag method with `fast' access */
	TM_ADD,
	TM_SUB,
	TM_MUL,
	TM_DIV,
	TM_MOD,
	TM_POW,
	TM_UNM,
	TM_LEN,
	TM_LT,
	TM_LE,
	TM_CONCAT,
	TM_CALL,
	TM_N		/* number of elements in the enum */
} TMS;



#define gfasttm(g,et,e) ((et) == NULL ? NULL : \
	((et)->flags & (1u<<(e))) ? NULL : luaT_gettm(et, e, (g)->tmname[e]))

#define fasttm(l,et,e)	gfasttm(G(l), et, e)

const char *const luaT_typenames[] = {
  "nil", "boolean", "userdata", "number",
  "string", "table", "function", "userdata", "thread",
  "proto", "upval"
};


LUAI_FUNC const TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o,
										 TMS event);
LUAI_FUNC void luaT_init (lua_State *L);

//#include "lzio.h"------------------------------------------------------------------------------------------------------
#define EOZ	(-1)			/* end of stream */

typedef struct Zio ZIO;

#define char2int(c)	cast(int, cast(unsigned char, (c)))

#define zgetc(z)  (((z)->n--)>0 ?  char2int(*(z)->p++) : luaZ_fill(z))

typedef struct Mbuffer {
	char *buffer;
	size_t n;
	size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = NULL, (buff)->buffsize = 0)

#define luaZ_buffer(buff)	((buff)->buffer)
#define luaZ_sizebuffer(buff)	((buff)->buffsize)
#define luaZ_bufflen(buff)	((buff)->n)

#define luaZ_resetbuffer(buff) ((buff)->n = 0)


#define luaZ_resizebuffer(L, buff, size) \
	(luaM_reallocvector(L, (buff)->buffer, (buff)->buffsize, size, char), \
	(buff)->buffsize = size)

#define luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC char *luaZ_openspace (lua_State *L, Mbuffer *buff, size_t n);
LUAI_FUNC void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
						  void *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, void* b, size_t n);	/* read next n bytes */
LUAI_FUNC int luaZ_lookahead (ZIO *z);



/* --------- Private Part ------------------ */

struct Zio {
	size_t n;			/* bytes still unread */
	const char *p;		/* current position in buffer */
	lua_Reader reader;
	void* data;			/* additional data */
	lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC int luaZ_fill (ZIO *z);

//#include "lstate.h"----------------------------------------------------------------------------------------------------
struct lua_longjmp;  /* defined in ldo.c */


/* table of globals */
#define gt(L)	(&L->l_gt)

/* registry */
#define registry(L)	(&G(L)->l_registry)


/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


#define BASIC_CI_SIZE           8

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)



typedef struct stringtable {
  GCObject **hash;
  lu_int32 nuse;  /* number of elements */
  int size;
} stringtable;


/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for this function */
  StkId func;  /* function index in the stack */
  StkId	top;  /* top for this function */
  const Instruction *savedpc;
  int nresults;  /* expected number of results from this function */
  int tailcalls;  /* number of tail calls lost under this entry */
} CallInfo;



#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->c.isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
  stringtable strt;  /* hash table for strings */
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to `frealloc' */
  lu_byte currentwhite;
  lu_byte gcstate;  /* state of garbage collector */
  int sweepstrgc;  /* position of sweep in `strt' */
  GCObject *rootgc;  /* list of all collectable objects */
  GCObject **sweepgc;  /* position of sweep in `rootgc' */
  GCObject *gray;  /* list of gray objects */
  GCObject *grayagain;  /* list of objects to be traversed atomically */
  GCObject *weak;  /* list of weak tables (to be cleared) */
  GCObject *tmudata;  /* last element of list of userdata to be GC */
  Mbuffer buff;  /* temporary buffer for string concatentation */
  lu_mem GCthreshold;
  lu_mem totalbytes;  /* number of bytes currently allocated */
  lu_mem estimate;  /* an estimate of number of bytes actually in use */
  lu_mem gcdept;  /* how much GC is `behind schedule' */
  int gcpause;  /* size of pause between successive GCs */
  int gcstepmul;  /* GC `granularity' */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TValue l_registry;
  struct lua_State *mainthread;
  UpVal uvhead;  /* head of double-linked list of all open upvalues */
  struct Table *mt[NUM_TAGS];  /* metatables for basic types */
  TString *tmname[TM_N];  /* array with tag-method names */
} global_State;

/*
** `per thread' state
*/
struct lua_State {
	CommonHeader;
	lu_byte status;
	StkId top;  /* first free slot in the stack */
	StkId base;  /* base of current function */
	global_State *l_G;
	CallInfo *ci;  /* call info for current function */
	const Instruction *savedpc;  /* `savedpc' of current function */
	StkId stack_last;  /* last free slot in the stack */
	StkId stack;  /* stack base */
	CallInfo *end_ci;  /* points after end of ci array*/
	CallInfo *base_ci;  /* array of CallInfo's */
	int stacksize;
	int size_ci;  /* size of array `base_ci' */
	unsigned short nCcalls;  /* number of nested C calls */
	lu_byte hookmask;
	lu_byte allowhook;
	int basehookcount;
	int hookcount;
	lua_Hook hook;
	TValue l_gt;  /* table of globals */
	TValue env;  /* temporary place for environments */
	GCObject *openupval;  /* list of open upvalues in this stack */
	GCObject *gclist;
	struct lua_longjmp *errorJmp;  /* current error recover point */
	ptrdiff_t errfunc;  /* current error handling function (stack index) */
};


#define G(L)	(L->l_G)


/*
** Union of all collectable objects
*/
union GCObject {
	GCheader gch;
	union TString ts;
	union Udata u;
	union Closure cl;
	struct Table h;
	struct Proto p;
	struct UpVal uv;
	struct lua_State th;  /* thread */
};


/* macros to convert a GCObject into a specific value */
#define rawgco2ts(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gco2ts(o)	(&rawgco2ts(o)->tsv)
#define rawgco2u(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gco2u(o)	(&rawgco2u(o)->uv)
#define gco2cl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
#define gco2h(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
#define gco2p(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
#define gco2uv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define gco2th(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

/* macro to convert any Lua object into a GCObject */
#define obj2gco(v)	(cast(GCObject *, (v)))

//#include "ldo.h"----------------------------------------------------------------------------------------------------
#define luaD_checkstack(L,n)	\
	if ((char *)L->stack_last - (char *)L->top <= (n)*(int)sizeof(TValue)) \
	luaD_growstack(L, n); \
  else condhardstacktests(luaD_reallocstack(L, L->stacksize - EXTRA_STACK - 1));


#define incr_top(L) {luaD_checkstack(L,1); L->top++;}

#define savestack(L,p)		((char *)(p) - (char *)L->stack)
#define restorestack(L,n)	((TValue *)((char *)L->stack + (n)))

#define saveci(L,p)		((char *)(p) - (char *)L->base_ci)
#define restoreci(L,n)		((CallInfo *)((char *)L->base_ci + (n)))


/* results from luaD_precall */
#define PCRLUA		0	/* initiated a call to a Lua function */
#define PCRC		1	/* did a call to a C function */
#define PCRYIELD	2	/* C funtion yielded */


/* type of protected functions, to be ran by `runprotected' */
typedef void (*Pfunc) (lua_State *L, void *ud);

LUAI_FUNC int luaD_protectedparser (lua_State *L, ZIO *z, const char *name);
LUAI_FUNC void luaD_callhook (lua_State *L, int event, int line);
LUAI_FUNC int luaD_precall (lua_State *L, StkId func, int nresults);
LUAI_FUNC void luaD_call (lua_State *L, StkId func, int nResults);
LUAI_FUNC int luaD_pcall (lua_State *L, Pfunc func, void *u,
						  ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC int luaD_poscall (lua_State *L, StkId firstResult);
LUAI_FUNC void luaD_reallocCI (lua_State *L, int newsize);
LUAI_FUNC void luaD_reallocstack (lua_State *L, int newsize);
LUAI_FUNC void luaD_growstack (lua_State *L, int n);

LUAI_FUNC void luaD_throw (lua_State *L, int errcode);
LUAI_FUNC int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud);

LUAI_FUNC void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop);


//#include "lstring.h"
//#include "ltable.h"
//#include "ltm.h"
//#include "lundump.h"
//#include "lvm.h"



//-------------------------------------------------------------lgc.c---------------------------------------



#define GCSTEPSIZE	1024u
#define GCSWEEPMAX	40
#define GCSWEEPCOST	10
#define GCFINALIZECOST	100


#define maskmarks	cast_byte(~(bitmask(BLACKBIT)|WHITEBITS))

#define makewhite(g,x)	\
	((x)->gch.marked = cast_byte(((x)->gch.marked & maskmarks) | luaC_white(g)))

#define white2gray(x)	reset2bits((x)->gch.marked, WHITE0BIT, WHITE1BIT)
#define black2gray(x)	resetbit((x)->gch.marked, BLACKBIT)

#define stringmark(s)	reset2bits((s)->tsv.marked, WHITE0BIT, WHITE1BIT)


#define isfinalized(u)		testbit((u)->marked, FINALIZEDBIT)
#define markfinalized(u)	l_setbit((u)->marked, FINALIZEDBIT)


#define KEYWEAK         bitmask(KEYWEAKBIT)
#define VALUEWEAK       bitmask(VALUEWEAKBIT)



#define markvalue(g,o) { checkconsistency(o); \
	if (iscollectable(o) && iswhite(gcvalue(o))) reallymarkobject(g,gcvalue(o)); }

#define markobject(g,t) { if (iswhite(obj2gco(t))) \
	reallymarkobject(g, obj2gco(t)); }


#define setthreshold(g)  (g->GCthreshold = (g->estimate/100) * g->gcpause)


static void removeentry (Node *n) {
	lua_assert(ttisnil(gval(n)));
	if (iscollectable(gkey(n)))
		setttype(gkey(n), LUA_TDEADKEY);  /* dead key; remove it */
}


static void reallymarkobject (global_State *g, GCObject *o) {
	lua_assert(iswhite(o) && !isdead(g, o));
	white2gray(o);
	switch (o->gch.tt) {
	case LUA_TSTRING: {
		return;
					  }
	case LUA_TUSERDATA: {
		Table *mt = gco2u(o)->metatable;
		gray2black(o);  /* udata are never gray */
		if (mt) markobject(g, mt);
		markobject(g, gco2u(o)->env);
		return;
						}
	case LUA_TUPVAL: {
		UpVal *uv = gco2uv(o);
		markvalue(g, uv->v);
		if (uv->v == &uv->u.value)  /* closed? */
			gray2black(o);  /* open upvalues are never black */
		return;
					 }
	case LUA_TFUNCTION: {
		gco2cl(o)->c.gclist = g->gray;
		g->gray = o;
		break;
						}
	case LUA_TTABLE: {
		gco2h(o)->gclist = g->gray;
		g->gray = o;
		break;
					 }
	case LUA_TTHREAD: {
		gco2th(o)->gclist = g->gray;
		g->gray = o;
		break;
					  }
	case LUA_TPROTO: {
		gco2p(o)->gclist = g->gray;
		g->gray = o;
		break;
					 }
	default: lua_assert(0);
	}
}


static void marktmu (global_State *g) {
	GCObject *u = g->tmudata;
	if (u) {
		do {
			u = u->gch.next;
			makewhite(g, u);  /* may be marked, if left from previous GC */
			reallymarkobject(g, u);
		} while (u != g->tmudata);
	}
}


/* move `dead' udata that need finalization to list `tmudata' */
size_t luaC_separateudata (lua_State *L, int all) {
	global_State *g = G(L);
	size_t deadmem = 0;
	GCObject **p = &g->mainthread->next;
	GCObject *curr;
	while ((curr = *p) != NULL) {
		if (!(iswhite(curr) || all) || isfinalized(gco2u(curr)))
			p = &curr->gch.next;  /* don't bother with them */
		else if (fasttm(L, gco2u(curr)->metatable, TM_GC) == NULL) {
			markfinalized(gco2u(curr));  /* don't need finalization */
			p = &curr->gch.next;
		}
		else {  /* must call its gc method */
			deadmem += sizeudata(gco2u(curr));
			markfinalized(gco2u(curr));
			*p = curr->gch.next;
			/* link `curr' at the end of `tmudata' list */
			if (g->tmudata == NULL)  /* list is empty? */
				g->tmudata = curr->gch.next = curr;  /* creates a circular list */
			else {
				curr->gch.next = g->tmudata->gch.next;
				g->tmudata->gch.next = curr;
				g->tmudata = curr;
			}
		}
	}
	return deadmem;
}


static int traversetable (global_State *g, Table *h) {
	int i;
	int weakkey = 0;
	int weakvalue = 0;
	const TValue *mode;
	if (h->metatable)
		markobject(g, h->metatable);
	mode = gfasttm(g, h->metatable, TM_MODE);
	if (mode && ttisstring(mode)) {  /* is there a weak mode? */
		weakkey = (strchr(svalue(mode), 'k') != NULL);
		weakvalue = (strchr(svalue(mode), 'v') != NULL);
		if (weakkey || weakvalue) {  /* is really weak? */
			h->marked &= ~(KEYWEAK | VALUEWEAK);  /* clear bits */
			h->marked |= cast_byte((weakkey << KEYWEAKBIT) |
				(weakvalue << VALUEWEAKBIT));
			h->gclist = g->weak;  /* must be cleared after GC, ... */
			g->weak = obj2gco(h);  /* ... so put in the appropriate list */
		}
	}
	if (weakkey && weakvalue) return 1;
	if (!weakvalue) {
		i = h->sizearray;
		while (i--)
			markvalue(g, &h->array[i]);
	}
	i = sizenode(h);
	while (i--) {
		Node *n = gnode(h, i);
		lua_assert(ttype(gkey(n)) != LUA_TDEADKEY || ttisnil(gval(n)));
		if (ttisnil(gval(n)))
			removeentry(n);  /* remove empty entries */
		else {
			lua_assert(!ttisnil(gkey(n)));
			if (!weakkey) markvalue(g, gkey(n));
			if (!weakvalue) markvalue(g, gval(n));
		}
	}
	return weakkey || weakvalue;
}


/*
** All marks are conditional because a GC may happen while the
** prototype is still being created
*/
static void traverseproto (global_State *g, Proto *f) {
	int i;
	if (f->source) stringmark(f->source);
	for (i=0; i<f->sizek; i++)  /* mark literals */
		markvalue(g, &f->k[i]);
	for (i=0; i<f->sizeupvalues; i++) {  /* mark upvalue names */
		if (f->upvalues[i])
			stringmark(f->upvalues[i]);
	}
	for (i=0; i<f->sizep; i++) {  /* mark nested protos */
		if (f->p[i])
			markobject(g, f->p[i]);
	}
	for (i=0; i<f->sizelocvars; i++) {  /* mark local-variable names */
		if (f->locvars[i].varname)
			stringmark(f->locvars[i].varname);
	}
}



static void traverseclosure (global_State *g, Closure *cl) {
	markobject(g, cl->c.env);
	if (cl->c.isC) {
		int i;
		for (i=0; i<cl->c.nupvalues; i++)  /* mark its upvalues */
			markvalue(g, &cl->c.upvalue[i]);
	}
	else {
		int i;
		lua_assert(cl->l.nupvalues == cl->l.p->nups);
		markobject(g, cl->l.p);
		for (i=0; i<cl->l.nupvalues; i++)  /* mark its upvalues */
			markobject(g, cl->l.upvals[i]);
	}
}


static void checkstacksizes (lua_State *L, StkId max) {
	int ci_used = cast_int(L->ci - L->base_ci);  /* number of `ci' in use */
	int s_used = cast_int(max - L->stack);  /* part of stack in use */
	if (L->size_ci > LUAI_MAXCALLS)  /* handling overflow? */
		return;  /* do not touch the stacks */
	if (4*ci_used < L->size_ci && 2*BASIC_CI_SIZE < L->size_ci)
		luaD_reallocCI(L, L->size_ci/2);  /* still big enough... */
	condhardstacktests(luaD_reallocCI(L, ci_used + 1));
	if (4*s_used < L->stacksize &&
		2*(BASIC_STACK_SIZE+EXTRA_STACK) < L->stacksize)
		luaD_reallocstack(L, L->stacksize/2);  /* still big enough... */
	condhardstacktests(luaD_reallocstack(L, s_used));
}


static void traversestack (global_State *g, lua_State *l) {
	StkId o, lim;
	CallInfo *ci;
	markvalue(g, gt(l));
	lim = l->top;
	for (ci = l->base_ci; ci <= l->ci; ci++) {
		lua_assert(ci->top <= l->stack_last);
		if (lim < ci->top) lim = ci->top;
	}
	for (o = l->stack; o < l->top; o++)
		markvalue(g, o);
	for (; o <= lim; o++)
		setnilvalue(o);
	checkstacksizes(l, lim);
}


/*
** traverse one gray object, turning it to black.
** Returns `quantity' traversed.
*/
static l_mem propagatemark (global_State *g) {
	GCObject *o = g->gray;
	lua_assert(isgray(o));
	gray2black(o);
	switch (o->gch.tt) {
	case LUA_TTABLE: {
		Table *h = gco2h(o);
		g->gray = h->gclist;
		if (traversetable(g, h))  /* table is weak? */
			black2gray(o);  /* keep it gray */
		return sizeof(Table) + sizeof(TValue) * h->sizearray +
			sizeof(Node) * sizenode(h);
					 }
	case LUA_TFUNCTION: {
		Closure *cl = gco2cl(o);
		g->gray = cl->c.gclist;
		traverseclosure(g, cl);
		return (cl->c.isC) ? sizeCclosure(cl->c.nupvalues) :
			sizeLclosure(cl->l.nupvalues);
						}
	case LUA_TTHREAD: {
		lua_State *th = gco2th(o);
		g->gray = th->gclist;
		th->gclist = g->grayagain;
		g->grayagain = o;
		black2gray(o);
		traversestack(g, th);
		return sizeof(lua_State) + sizeof(TValue) * th->stacksize +
			sizeof(CallInfo) * th->size_ci;
					  }
	case LUA_TPROTO: {
		Proto *p = gco2p(o);
		g->gray = p->gclist;
		traverseproto(g, p);
		return sizeof(Proto) + sizeof(Instruction) * p->sizecode +
			sizeof(Proto *) * p->sizep +
			sizeof(TValue) * p->sizek + 
			sizeof(int) * p->sizelineinfo +
			sizeof(LocVar) * p->sizelocvars +
			sizeof(TString *) * p->sizeupvalues;
					 }
	default: lua_assert(0); return 0;
	}
}


static size_t propagateall (global_State *g) {
	size_t m = 0;
	while (g->gray) m += propagatemark(g);
	return m;
}


/*
** The next function tells whether a key or value can be cleared from
** a weak table. Non-collectable objects are never removed from weak
** tables. Strings behave as `values', so are never removed too. for
** other objects: if really collected, cannot keep them; for userdata
** being finalized, keep them in keys, but not in values
*/
static int iscleared (const TValue *o, int iskey) {
	if (!iscollectable(o)) return 0;
	if (ttisstring(o)) {
		stringmark(rawtsvalue(o));  /* strings are `values', so are never weak */
		return 0;
	}
	return iswhite(gcvalue(o)) ||
		(ttisuserdata(o) && (!iskey && isfinalized(uvalue(o))));
}


/*
** clear collected entries from weaktables
*/
static void cleartable (GCObject *l) {
	while (l) {
		Table *h = gco2h(l);
		int i = h->sizearray;
		lua_assert(testbit(h->marked, VALUEWEAKBIT) ||
			testbit(h->marked, KEYWEAKBIT));
		if (testbit(h->marked, VALUEWEAKBIT)) {
			while (i--) {
				TValue *o = &h->array[i];
				if (iscleared(o, 0))  /* value was collected? */
					setnilvalue(o);  /* remove value */
			}
		}
		i = sizenode(h);
		while (i--) {
			Node *n = gnode(h, i);
			if (!ttisnil(gval(n)) &&  /* non-empty entry? */
				(iscleared(key2tval(n), 1) || iscleared(gval(n), 0))) {
					setnilvalue(gval(n));  /* remove value ... */
					removeentry(n);  /* remove entry from table */
			}
		}
		l = h->gclist;
	}
}


static void freeobj (lua_State *L, GCObject *o) {
	switch (o->gch.tt) {
	case LUA_TPROTO: luaF_freeproto(L, gco2p(o)); break;
	case LUA_TFUNCTION: luaF_freeclosure(L, gco2cl(o)); break;
	case LUA_TUPVAL: luaF_freeupval(L, gco2uv(o)); break;
	case LUA_TTABLE: luaH_free(L, gco2h(o)); break;
	case LUA_TTHREAD: {
		lua_assert(gco2th(o) != L && gco2th(o) != G(L)->mainthread);
		luaE_freethread(L, gco2th(o));
		break;
					  }
	case LUA_TSTRING: {
		G(L)->strt.nuse--;
		luaM_freemem(L, o, sizestring(gco2ts(o)));
		break;
					  }
	case LUA_TUSERDATA: {
		luaM_freemem(L, o, sizeudata(gco2u(o)));
		break;
						}
	default: lua_assert(0);
	}
}



#define sweepwholelist(L,p)	sweeplist(L,p,MAX_LUMEM)


static GCObject **sweeplist (lua_State *L, GCObject **p, lu_mem count) {
	GCObject *curr;
	global_State *g = G(L);
	int deadmask = otherwhite(g);
	while ((curr = *p) != NULL && count-- > 0) {
		if (curr->gch.tt == LUA_TTHREAD)  /* sweep open upvalues of each thread */
			sweepwholelist(L, &gco2th(curr)->openupval);
		if ((curr->gch.marked ^ WHITEBITS) & deadmask) {  /* not dead? */
			lua_assert(!isdead(g, curr) || testbit(curr->gch.marked, FIXEDBIT));
			makewhite(g, curr);  /* make it white (for next cycle) */
			p = &curr->gch.next;
		}
		else {  /* must erase `curr' */
			lua_assert(isdead(g, curr) || deadmask == bitmask(SFIXEDBIT));
			*p = curr->gch.next;
			if (curr == g->rootgc)  /* is the first element of the list? */
				g->rootgc = curr->gch.next;  /* adjust first */
			freeobj(L, curr);
		}
	}
	return p;
}


static void checkSizes (lua_State *L) {
	global_State *g = G(L);
	/* check size of string hash */
	if (g->strt.nuse < cast(lu_int32, g->strt.size/4) &&
		g->strt.size > MINSTRTABSIZE*2)
		luaS_resize(L, g->strt.size/2);  /* table is too big */
	/* check size of buffer */
	if (luaZ_sizebuffer(&g->buff) > LUA_MINBUFFER*2) {  /* buffer too big? */
		size_t newsize = luaZ_sizebuffer(&g->buff) / 2;
		luaZ_resizebuffer(L, &g->buff, newsize);
	}
}


static void GCTM (lua_State *L) {
	global_State *g = G(L);
	GCObject *o = g->tmudata->gch.next;  /* get first element */
	Udata *udata = rawgco2u(o);
	const TValue *tm;
	/* remove udata from `tmudata' */
	if (o == g->tmudata)  /* last element? */
		g->tmudata = NULL;
	else
		g->tmudata->gch.next = udata->uv.next;
	udata->uv.next = g->mainthread->next;  /* return it to `root' list */
	g->mainthread->next = o;
	makewhite(g, o);
	tm = fasttm(L, udata->uv.metatable, TM_GC);
	if (tm != NULL) {
		lu_byte oldah = L->allowhook;
		lu_mem oldt = g->GCthreshold;
		L->allowhook = 0;  /* stop debug hooks during GC tag method */
		g->GCthreshold = 2*g->totalbytes;  /* avoid GC steps */
		setobj2s(L, L->top, tm);
		setuvalue(L, L->top+1, udata);
		L->top += 2;
		luaD_call(L, L->top - 2, 0);
		L->allowhook = oldah;  /* restore hooks */
		g->GCthreshold = oldt;  /* restore threshold */
	}
}


/*
** Call all GC tag methods
*/
void luaC_callGCTM (lua_State *L) {
	while (G(L)->tmudata)
		GCTM(L);
}


void luaC_freeall (lua_State *L) {
	global_State *g = G(L);
	int i;
	g->currentwhite = WHITEBITS | bitmask(SFIXEDBIT);  /* mask to collect all elements */
	sweepwholelist(L, &g->rootgc);
	for (i = 0; i < g->strt.size; i++)  /* free all string lists */
		sweepwholelist(L, &g->strt.hash[i]);
}


static void markmt (global_State *g) {
	int i;
	for (i=0; i<NUM_TAGS; i++)
		if (g->mt[i]) markobject(g, g->mt[i]);
}


/* mark root set */
static void markroot (lua_State *L) {
	global_State *g = G(L);
	g->gray = NULL;
	g->grayagain = NULL;
	g->weak = NULL;
	markobject(g, g->mainthread);
	/* make global table be traversed before main stack */
	markvalue(g, gt(g->mainthread));
	markvalue(g, registry(L));
	markmt(g);
	g->gcstate = GCSpropagate;
}


static void remarkupvals (global_State *g) {
	UpVal *uv;
	for (uv = g->uvhead.u.l.next; uv != &g->uvhead; uv = uv->u.l.next) {
		lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
		if (isgray(obj2gco(uv)))
			markvalue(g, uv->v);
	}
}


static void atomic (lua_State *L) {
	global_State *g = G(L);
	size_t udsize;  /* total size of userdata to be finalized */
	/* remark occasional upvalues of (maybe) dead threads */
	remarkupvals(g);
	/* traverse objects cautch by write barrier and by 'remarkupvals' */
	propagateall(g);
	/* remark weak tables */
	g->gray = g->weak;
	g->weak = NULL;
	lua_assert(!iswhite(obj2gco(g->mainthread)));
	markobject(g, L);  /* mark running thread */
	markmt(g);  /* mark basic metatables (again) */
	propagateall(g);
	/* remark gray again */
	g->gray = g->grayagain;
	g->grayagain = NULL;
	propagateall(g);
	udsize = luaC_separateudata(L, 0);  /* separate userdata to be finalized */
	marktmu(g);  /* mark `preserved' userdata */
	udsize += propagateall(g);  /* remark, to propagate `preserveness' */
	cleartable(g->weak);  /* remove collected objects from weak tables */
	/* flip current white */
	g->currentwhite = cast_byte(otherwhite(g));
	g->sweepstrgc = 0;
	g->sweepgc = &g->rootgc;
	g->gcstate = GCSsweepstring;
	g->estimate = g->totalbytes - udsize;  /* first estimate */
}


static l_mem singlestep (lua_State *L) {
	global_State *g = G(L);
	/*lua_checkmemory(L);*/
	switch (g->gcstate) {
	case GCSpause: {
		markroot(L);  /* start a new collection */
		return 0;
				   }
	case GCSpropagate: {
		if (g->gray)
			return propagatemark(g);
		else {  /* no more `gray' objects */
			atomic(L);  /* finish mark phase */
			return 0;
		}
					   }
	case GCSsweepstring: {
		lu_mem old = g->totalbytes;
		sweepwholelist(L, &g->strt.hash[g->sweepstrgc++]);
		if (g->sweepstrgc >= g->strt.size)  /* nothing more to sweep? */
			g->gcstate = GCSsweep;  /* end sweep-string phase */
		lua_assert(old >= g->totalbytes);
		g->estimate -= old - g->totalbytes;
		return GCSWEEPCOST;
						 }
	case GCSsweep: {
		lu_mem old = g->totalbytes;
		g->sweepgc = sweeplist(L, g->sweepgc, GCSWEEPMAX);
		if (*g->sweepgc == NULL) {  /* nothing more to sweep? */
			checkSizes(L);
			g->gcstate = GCSfinalize;  /* end sweep phase */
		}
		lua_assert(old >= g->totalbytes);
		g->estimate -= old - g->totalbytes;
		return GCSWEEPMAX*GCSWEEPCOST;
				   }
	case GCSfinalize: {
		if (g->tmudata) {
			GCTM(L);
			if (g->estimate > GCFINALIZECOST)
				g->estimate -= GCFINALIZECOST;
			return GCFINALIZECOST;
		}
		else {
			g->gcstate = GCSpause;  /* end collection */
			g->gcdept = 0;
			return 0;
		}
					  }
	default: lua_assert(0); return 0;
	}
}


void luaC_step (lua_State *L) {
	global_State *g = G(L);
	l_mem lim = (GCSTEPSIZE/100) * g->gcstepmul;
	if (lim == 0)
		lim = (MAX_LUMEM-1)/2;  /* no limit */
	g->gcdept += g->totalbytes - g->GCthreshold;
	do {
		lim -= singlestep(L);
		if (g->gcstate == GCSpause)
			break;
	} while (lim > 0);
	if (g->gcstate != GCSpause) {
		if (g->gcdept < GCSTEPSIZE)
			g->GCthreshold = g->totalbytes + GCSTEPSIZE;  /* - lim/g->gcstepmul;*/
		else {
			g->gcdept -= GCSTEPSIZE;
			g->GCthreshold = g->totalbytes;
		}
	}
	else {
		lua_assert(g->totalbytes >= g->estimate);
		setthreshold(g);
	}
}


void luaC_fullgc (lua_State *L) {
	global_State *g = G(L);
	if (g->gcstate <= GCSpropagate) {
		/* reset sweep marks to sweep all elements (returning them to white) */
		g->sweepstrgc = 0;
		g->sweepgc = &g->rootgc;
		/* reset other collector lists */
		g->gray = NULL;
		g->grayagain = NULL;
		g->weak = NULL;
		g->gcstate = GCSsweepstring;
	}
	lua_assert(g->gcstate != GCSpause && g->gcstate != GCSpropagate);
	/* finish any pending sweep phase */
	while (g->gcstate != GCSfinalize) {
		lua_assert(g->gcstate == GCSsweepstring || g->gcstate == GCSsweep);
		singlestep(L);
	}
	markroot(L);
	while (g->gcstate != GCSpause) {
		singlestep(L);
	}
	setthreshold(g);
}


void luaC_barrierf (lua_State *L, GCObject *o, GCObject *v) {
	global_State *g = G(L);
	lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
	lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
	lua_assert(ttype(&o->gch) != LUA_TTABLE);
	/* must keep invariant? */
	if (g->gcstate == GCSpropagate)
		reallymarkobject(g, v);  /* restore invariant */
	else  /* don't mind */
		makewhite(g, o);  /* mark as white just to avoid other barriers */
}


void luaC_barrierback (lua_State *L, Table *t) {
	global_State *g = G(L);
	GCObject *o = obj2gco(t);
	lua_assert(isblack(o) && !isdead(g, o));
	lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
	black2gray(o);  /* make table gray (again) */
	t->gclist = g->grayagain;
	g->grayagain = o;
}


void luaC_link (lua_State *L, GCObject *o, lu_byte tt) {
	global_State *g = G(L);
	o->gch.next = g->rootgc;
	g->rootgc = o;
	o->gch.marked = luaC_white(g);
	o->gch.tt = tt;
}


void luaC_linkupval (lua_State *L, UpVal *uv) {
	global_State *g = G(L);
	GCObject *o = obj2gco(uv);
	o->gch.next = g->rootgc;  /* link upvalue into `rootgc' list */
	g->rootgc = o;
	if (isgray(o)) { 
		if (g->gcstate == GCSpropagate) {
			gray2black(o);  /* closed upvalues need barrier */
			luaC_barrier(L, uv, uv->v);
		}
		else {  /* sweep phase: sweep it (turning it into white) */
			makewhite(g, o);
			lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
		}
	}
}



//-------------------------------------------------------------lapi.c---------------------------------------

/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;

/*
** Union of all Lua values
*/
//typedef union {
//  GCObject *gc;
//  void *p;
//  lua_Number n;
//  int b;
//} Value;

/*
** $Id: lapi.c,v 2.55 2006/06/07 12:37:17 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/


#define TValuefields	Value value; int tt

//typedef struct lua_TValue {
//	TValuefields;
//} TValue;

#ifndef cast
#define cast(t, exp)	((t)(exp))
#endif




/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked




static TValue *index2adr (lua_State *L, int idx) {
	if (idx > 0) {
		TValue *o = L->base + (idx - 1);
		api_check(L, idx <= L->ci->top - L->base);
		if (o >= L->top) return cast(TValue *, luaO_nilobject);
		else return o;
	}
	else if (idx > LUA_REGISTRYINDEX) {
		api_check(L, idx != 0 && -idx <= L->top - L->base);
		return L->top + idx;
	}
	else switch (idx) {  /* pseudo-indices */
	case LUA_REGISTRYINDEX: return registry(L);
	case LUA_ENVIRONINDEX: {
		Closure *func = curr_func(L);
		sethvalue(L, &L->env, func->c.env);
		return &L->env;
						   }
	case LUA_GLOBALSINDEX: return gt(L);
	default: {
		Closure *func = curr_func(L);
		idx = LUA_GLOBALSINDEX - idx;
		return (idx <= func->c.nupvalues)
			? &func->c.upvalue[idx-1]
		: cast(TValue *, luaO_nilobject);
			 }
	}
}


static Table *getcurrenv (lua_State *L) {
	if (L->ci == L->base_ci)  /* no enclosing function? */
		return hvalue(gt(L));  /* use global table as environment */
	else {
		Closure *func = curr_func(L);
		return func->c.env;
	}
}


void luaA_pushobject (lua_State *L, const TValue *o) {
	setobj2s(L, L->top, o);
	api_incr_top(L);
}


LUA_API int lua_checkstack (lua_State *L, int size) {
	int res;
	lua_lock(L);
	if ((L->top - L->base + size) > LUAI_MAXCSTACK)
		res = 0;  /* stack overflow */
	else {
		luaD_checkstack(L, size);
		if (L->ci->top < L->top + size)
			L->ci->top = L->top + size;
		res = 1;
	}
	lua_unlock(L);
	return res;
}


LUA_API void lua_xmove (lua_State *from, lua_State *to, int n) {
	int i;
	if (from == to) return;
	lua_lock(to);
	api_checknelems(from, n);
	api_check(from, G(from) == G(to));
	api_check(from, to->ci->top - to->top >= n);
	from->top -= n;
	for (i = 0; i < n; i++) {
		setobj2s(to, to->top++, from->top + i);
	}
	lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
	lua_CFunction old;
	lua_lock(L);
	old = G(L)->panic;
	G(L)->panic = panicf;
	lua_unlock(L);
	return old;
}


LUA_API lua_State *lua_newthread (lua_State *L) {
	lua_State *L1;
	lua_lock(L);
	luaC_checkGC(L);
	L1 = luaE_newthread(L);
	setthvalue(L, L->top, L1);
	api_incr_top(L);
	lua_unlock(L);
	luai_userstatethread(L, L1);
	return L1;
}



/*
** basic stack manipulation
*/


LUA_API int lua_gettop (lua_State *L) {
	return cast_int(L->top - L->base);
}


LUA_API void lua_settop (lua_State *L, int idx) {
	lua_lock(L);
	if (idx >= 0) {
		api_check(L, idx <= L->stack_last - L->base);
		while (L->top < L->base + idx)
			setnilvalue(L->top++);
		L->top = L->base + idx;
	}
	else {
		api_check(L, -(idx+1) <= (L->top - L->base));
		L->top += idx+1;  /* `subtract' index (index is negative) */
	}
	lua_unlock(L);
}


LUA_API void lua_remove (lua_State *L, int idx) {
	StkId p;
	lua_lock(L);
	p = index2adr(L, idx);
	api_checkvalidindex(L, p);
	while (++p < L->top) setobjs2s(L, p-1, p);
	L->top--;
	lua_unlock(L);
}


LUA_API void lua_insert (lua_State *L, int idx) {
	StkId p;
	StkId q;
	lua_lock(L);
	p = index2adr(L, idx);
	api_checkvalidindex(L, p);
	for (q = L->top; q>p; q--) setobjs2s(L, q, q-1);
	setobjs2s(L, p, L->top);
	lua_unlock(L);
}


LUA_API void lua_replace (lua_State *L, int idx) {
	StkId o;
	lua_lock(L);
	/* explicit test for incompatible code */
	if (idx == LUA_ENVIRONINDEX && L->ci == L->base_ci)
		luaG_runerror(L, "no calling environment");
	api_checknelems(L, 1);
	o = index2adr(L, idx);
	api_checkvalidindex(L, o);
	if (idx == LUA_ENVIRONINDEX) {
		Closure *func = curr_func(L);
		api_check(L, ttistable(L->top - 1)); 
		func->c.env = hvalue(L->top - 1);
		luaC_barrier(L, func, L->top - 1);
	}
	else {
		setobj(L, o, L->top - 1);
		if (idx < LUA_GLOBALSINDEX)  /* function upvalue? */
			luaC_barrier(L, curr_func(L), L->top - 1);
	}
	L->top--;
	lua_unlock(L);
}


LUA_API void lua_pushvalue (lua_State *L, int idx) {
	lua_lock(L);
	setobj2s(L, L->top, index2adr(L, idx));
	api_incr_top(L);
	lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API int lua_type (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	return (o == luaO_nilobject) ? LUA_TNONE : ttype(o);
}


LUA_API const char *lua_typename (lua_State *L, int t) {
	UNUSED(L);
	return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}


LUA_API int lua_iscfunction (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	return iscfunction(o);
}


LUA_API int lua_isnumber (lua_State *L, int idx) {
	TValue n;
	const TValue *o = index2adr(L, idx);
	return tonumber(o, &n);
}


LUA_API int lua_isstring (lua_State *L, int idx) {
	int t = lua_type(L, idx);
	return (t == LUA_TSTRING || t == LUA_TNUMBER);
}


LUA_API int lua_isuserdata (lua_State *L, int idx) {
	const TValue *o = index2adr(L, idx);
	return (ttisuserdata(o) || ttislightuserdata(o));
}


LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
	StkId o1 = index2adr(L, index1);
	StkId o2 = index2adr(L, index2);
	return (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
		: luaO_rawequalObj(o1, o2);
}


LUA_API int lua_equal (lua_State *L, int index1, int index2) {
	StkId o1, o2;
	int i;
	lua_lock(L);  /* may call tag method */
	o1 = index2adr(L, index1);
	o2 = index2adr(L, index2);
	i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
	lua_unlock(L);
	return i;
}


LUA_API int lua_lessthan (lua_State *L, int index1, int index2) {
	StkId o1, o2;
	int i;
	lua_lock(L);  /* may call tag method */
	o1 = index2adr(L, index1);
	o2 = index2adr(L, index2);
	i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
		: luaV_lessthan(L, o1, o2);
	lua_unlock(L);
	return i;
}



LUA_API lua_Number lua_tonumber (lua_State *L, int idx) {
	TValue n;
	const TValue *o = index2adr(L, idx);
	if (tonumber(o, &n))
		return nvalue(o);
	else
		return 0;
}


LUA_API lua_Integer lua_tointeger (lua_State *L, int idx) {
	TValue n;
	const TValue *o = index2adr(L, idx);
	if (tonumber(o, &n)) {
		lua_Integer res;
		lua_Number num = nvalue(o);
		lua_number2integer(res, num);
		return res;
	}
	else
		return 0;
}


LUA_API int lua_toboolean (lua_State *L, int idx) {
	const TValue *o = index2adr(L, idx);
	return !l_isfalse(o);
}


LUA_API const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
	StkId o = index2adr(L, idx);
	if (!ttisstring(o)) {
		lua_lock(L);  /* `luaV_tostring' may create a new string */
		if (!luaV_tostring(L, o)) {  /* conversion failed? */
			if (len != NULL) *len = 0;
			lua_unlock(L);
			return NULL;
		}
		luaC_checkGC(L);
		o = index2adr(L, idx);  /* previous call may reallocate the stack */
		lua_unlock(L);
	}
	if (len != NULL) *len = tsvalue(o)->len;
	return svalue(o);
}


LUA_API size_t lua_objlen (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	switch (ttype(o)) {
	case LUA_TSTRING: return tsvalue(o)->len;
	case LUA_TUSERDATA: return uvalue(o)->len;
	case LUA_TTABLE: return luaH_getn(hvalue(o));
	case LUA_TNUMBER: {
		size_t l;
		lua_lock(L);  /* `luaV_tostring' may create a new string */
		l = (luaV_tostring(L, o) ? tsvalue(o)->len : 0);
		lua_unlock(L);
		return l;
					  }
	default: return 0;
	}
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	return (!iscfunction(o)) ? NULL : clvalue(o)->c.f;
}


LUA_API void *lua_touserdata (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	switch (ttype(o)) {
	case LUA_TUSERDATA: return (rawuvalue(o) + 1);
	case LUA_TLIGHTUSERDATA: return pvalue(o);
	default: return NULL;
	}
}


LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	return (!ttisthread(o)) ? NULL : thvalue(o);
}


LUA_API const void *lua_topointer (lua_State *L, int idx) {
	StkId o = index2adr(L, idx);
	switch (ttype(o)) {
	case LUA_TTABLE: return hvalue(o);
	case LUA_TFUNCTION: return clvalue(o);
	case LUA_TTHREAD: return thvalue(o);
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		return lua_touserdata(L, idx);
	default: return NULL;
	}
}



/*
** push functions (C -> stack)
*/


LUA_API void lua_pushnil (lua_State *L) {
	lua_lock(L);
	setnilvalue(L->top);
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
	lua_lock(L);
	setnvalue(L->top, n);
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
	lua_lock(L);
	setnvalue(L->top, cast_num(n));
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_pushlstring (lua_State *L, const char *s, size_t len) {
	lua_lock(L);
	luaC_checkGC(L);
	setsvalue2s(L, L->top, luaS_newlstr(L, s, len));
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_pushstring (lua_State *L, const char *s) {
	if (s == NULL)
		lua_pushnil(L);
	else
		lua_pushlstring(L, s, strlen(s));
}


LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
									  va_list argp) {
										  const char *ret;
										  lua_lock(L);
										  luaC_checkGC(L);
										  ret = luaO_pushvfstring(L, fmt, argp);
										  lua_unlock(L);
										  return ret;
}


LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
	const char *ret;
	va_list argp;
	lua_lock(L);
	luaC_checkGC(L);
	va_start(argp, fmt);
	ret = luaO_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_unlock(L);
	return ret;
}


LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
	Closure *cl;
	lua_lock(L);
	luaC_checkGC(L);
	api_checknelems(L, n);
	cl = luaF_newCclosure(L, n, getcurrenv(L));
	cl->c.f = fn;
	L->top -= n;
	while (n--)
		setobj2n(L, &cl->c.upvalue[n], L->top+n);
	setclvalue(L, L->top, cl);
	lua_assert(iswhite(obj2gco(cl)));
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_pushboolean (lua_State *L, int b) {
	lua_lock(L);
	setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
	lua_lock(L);
	setpvalue(L->top, p);
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API int lua_pushthread (lua_State *L) {
	lua_lock(L);
	setthvalue(L, L->top, L);
	api_incr_top(L);
	lua_unlock(L);
	return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/


LUA_API void lua_gettable (lua_State *L, int idx) {
	StkId t;
	lua_lock(L);
	t = index2adr(L, idx);
	api_checkvalidindex(L, t);
	luaV_gettable(L, t, L->top - 1, L->top - 1);
	lua_unlock(L);
}


LUA_API void lua_getfield (lua_State *L, int idx, const char *k) {
	StkId t;
	TValue key;
	lua_lock(L);
	t = index2adr(L, idx);
	api_checkvalidindex(L, t);
	setsvalue(L, &key, luaS_new(L, k));
	luaV_gettable(L, t, &key, L->top);
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_rawget (lua_State *L, int idx) {
	StkId t;
	lua_lock(L);
	t = index2adr(L, idx);
	api_check(L, ttistable(t));
	setobj2s(L, L->top - 1, luaH_get(hvalue(t), L->top - 1));
	lua_unlock(L);
}


LUA_API void lua_rawgeti (lua_State *L, int idx, int n) {
	StkId o;
	lua_lock(L);
	o = index2adr(L, idx);
	api_check(L, ttistable(o));
	setobj2s(L, L->top, luaH_getnum(hvalue(o), n));
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API void lua_createtable (lua_State *L, int narray, int nrec) {
	lua_lock(L);
	luaC_checkGC(L);
	sethvalue(L, L->top, luaH_new(L, narray, nrec));
	api_incr_top(L);
	lua_unlock(L);
}


LUA_API int lua_getmetatable (lua_State *L, int objindex) {
	const TValue *obj;
	Table *mt = NULL;
	int res;
	lua_lock(L);
	obj = index2adr(L, objindex);
	switch (ttype(obj)) {
	case LUA_TTABLE:
		mt = hvalue(obj)->metatable;
		break;
	case LUA_TUSERDATA:
		mt = uvalue(obj)->metatable;
		break;
	default:
		mt = G(L)->mt[ttype(obj)];
		break;
	}
	if (mt == NULL)
		res = 0;
	else {
		sethvalue(L, L->top, mt);
		api_incr_top(L);
		res = 1;
	}
	lua_unlock(L);
	return res;
}


LUA_API void lua_getfenv (lua_State *L, int idx) {
	StkId o;
	lua_lock(L);
	o = index2adr(L, idx);
	api_checkvalidindex(L, o);
	switch (ttype(o)) {
	case LUA_TFUNCTION:
		sethvalue(L, L->top, clvalue(o)->c.env);
		break;
	case LUA_TUSERDATA:
		sethvalue(L, L->top, uvalue(o)->env);
		break;
	case LUA_TTHREAD:
		setobj2s(L, L->top,  gt(thvalue(o)));
		break;
	default:
		setnilvalue(L->top);
		break;
	}
	api_incr_top(L);
	lua_unlock(L);
}


/*
** set functions (stack -> Lua)
*/


LUA_API void lua_settable (lua_State *L, int idx) {
	StkId t;
	lua_lock(L);
	api_checknelems(L, 2);
	t = index2adr(L, idx);
	api_checkvalidindex(L, t);
	luaV_settable(L, t, L->top - 2, L->top - 1);
	L->top -= 2;  /* pop index and value */
	lua_unlock(L);
}


LUA_API void lua_setfield (lua_State *L, int idx, const char *k) {
	StkId t;
	TValue key;
	lua_lock(L);
	api_checknelems(L, 1);
	t = index2adr(L, idx);
	api_checkvalidindex(L, t);
	setsvalue(L, &key, luaS_new(L, k));
	luaV_settable(L, t, &key, L->top - 1);
	L->top--;  /* pop value */
	lua_unlock(L);
}


LUA_API void lua_rawset (lua_State *L, int idx) {
	StkId t;
	lua_lock(L);
	api_checknelems(L, 2);
	t = index2adr(L, idx);
	api_check(L, ttistable(t));
	setobj2t(L, luaH_set(L, hvalue(t), L->top-2), L->top-1);
	luaC_barriert(L, hvalue(t), L->top-1);
	L->top -= 2;
	lua_unlock(L);
}


LUA_API void lua_rawseti (lua_State *L, int idx, int n) {
	StkId o;
	lua_lock(L);
	api_checknelems(L, 1);
	o = index2adr(L, idx);
	api_check(L, ttistable(o));
	setobj2t(L, luaH_setnum(L, hvalue(o), n), L->top-1);
	luaC_barriert(L, hvalue(o), L->top-1);
	L->top--;
	lua_unlock(L);
}


LUA_API int lua_setmetatable (lua_State *L, int objindex) {
	TValue *obj;
	Table *mt;
	lua_lock(L);
	api_checknelems(L, 1);
	obj = index2adr(L, objindex);
	api_checkvalidindex(L, obj);
	if (ttisnil(L->top - 1))
		mt = NULL;
	else {
		api_check(L, ttistable(L->top - 1));
		mt = hvalue(L->top - 1);
	}
	switch (ttype(obj)) {
	case LUA_TTABLE: {
		hvalue(obj)->metatable = mt;
		if (mt)
			luaC_objbarriert(L, hvalue(obj), mt);
		break;
					 }
	case LUA_TUSERDATA: {
		uvalue(obj)->metatable = mt;
		if (mt)
			luaC_objbarrier(L, rawuvalue(obj), mt);
		break;
						}
	default: {
		G(L)->mt[ttype(obj)] = mt;
		break;
			 }
	}
	L->top--;
	lua_unlock(L);
	return 1;
}


LUA_API int lua_setfenv (lua_State *L, int idx) {
	StkId o;
	int res = 1;
	lua_lock(L);
	api_checknelems(L, 1);
	o = index2adr(L, idx);
	api_checkvalidindex(L, o);
	api_check(L, ttistable(L->top - 1));
	switch (ttype(o)) {
	case LUA_TFUNCTION:
		clvalue(o)->c.env = hvalue(L->top - 1);
		break;
	case LUA_TUSERDATA:
		uvalue(o)->env = hvalue(L->top - 1);
		break;
	case LUA_TTHREAD:
		sethvalue(L, gt(thvalue(o)), hvalue(L->top - 1));
		break;
	default:
		res = 0;
		break;
	}
	luaC_objbarrier(L, gcvalue(o), hvalue(L->top - 1));
	L->top--;
	lua_unlock(L);
	return res;
}


/*
** `load' and `call' functions (run Lua code)
*/


#define adjustresults(L,nres) \
{ if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }


#define checkresults(L,na,nr) \
	api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))


LUA_API void lua_call (lua_State *L, int nargs, int nresults) {
	StkId func;
	lua_lock(L);
	api_checknelems(L, nargs+1);
	checkresults(L, nargs, nresults);
	func = L->top - (nargs+1);
	luaD_call(L, func, nresults);
	adjustresults(L, nresults);
	lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
	StkId func;
	int nresults;
};


static void f_call (lua_State *L, void *ud) {
	struct CallS *c = cast(struct CallS *, ud);
	luaD_call(L, c->func, c->nresults);
}



LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc) {
	struct CallS c;
	int status;
	ptrdiff_t func;
	lua_lock(L);
	api_checknelems(L, nargs+1);
	checkresults(L, nargs, nresults);
	if (errfunc == 0)
		func = 0;
	else {
		StkId o = index2adr(L, errfunc);
		api_checkvalidindex(L, o);
		func = savestack(L, o);
	}
	c.func = L->top - (nargs+1);  /* function to be called */
	c.nresults = nresults;
	status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
	adjustresults(L, nresults);
	lua_unlock(L);
	return status;
}


/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
	lua_CFunction func;
	void *ud;
};


static void f_Ccall (lua_State *L, void *ud) {
	struct CCallS *c = cast(struct CCallS *, ud);
	Closure *cl;
	cl = luaF_newCclosure(L, 0, getcurrenv(L));
	cl->c.f = c->func;
	setclvalue(L, L->top, cl);  /* push function */
	api_incr_top(L);
	setpvalue(L->top, c->ud);  /* push only argument */
	api_incr_top(L);
	luaD_call(L, L->top - 2, 0);
}


LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
	struct CCallS c;
	int status;
	lua_lock(L);
	c.func = func;
	c.ud = ud;
	status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0);
	lua_unlock(L);
	return status;
}


LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
					  const char *chunkname) {
						  ZIO z;
						  int status;
						  lua_lock(L);
						  if (!chunkname) chunkname = "?";
						  luaZ_init(L, &z, reader, data);
						  status = luaD_protectedparser(L, &z, chunkname);
						  lua_unlock(L);
						  return status;
}


LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data) {
	int status;
	TValue *o;
	lua_lock(L);
	api_checknelems(L, 1);
	o = L->top - 1;
	if (isLfunction(o))
		status = luaU_dump(L, clvalue(o)->l.p, writer, data, 0);
	else
		status = 1;
	lua_unlock(L);
	return status;
}


LUA_API int  lua_status (lua_State *L) {
	return L->status;
}


/*
** Garbage-collection function
*/

LUA_API int lua_gc (lua_State *L, int what, int data) {
	int res = 0;
	global_State *g;
	lua_lock(L);
	g = G(L);
	switch (what) {
	case LUA_GCSTOP: {
		g->GCthreshold = MAX_LUMEM;
		break;
					 }
	case LUA_GCRESTART: {
		g->GCthreshold = g->totalbytes;
		break;
						}
	case LUA_GCCOLLECT: {
		luaC_fullgc(L);
		break;
						}
	case LUA_GCCOUNT: {
		/* GC values are expressed in Kbytes: #bytes/2^10 */
		res = cast_int(g->totalbytes >> 10);
		break;
					  }
	case LUA_GCCOUNTB: {
		res = cast_int(g->totalbytes & 0x3ff);
		break;
					   }
	case LUA_GCSTEP: {
		lu_mem a = (cast(lu_mem, data) << 10);
		if (a <= g->totalbytes)
			g->GCthreshold = g->totalbytes - a;
		else
			g->GCthreshold = 0;
		while (g->GCthreshold <= g->totalbytes)
			luaC_step(L);
		if (g->gcstate == GCSpause)  /* end of cycle? */
			res = 1;  /* signal it */
		break;
					 }
	case LUA_GCSETPAUSE: {
		res = g->gcpause;
		g->gcpause = data;
		break;
						 }
	case LUA_GCSETSTEPMUL: {
		res = g->gcstepmul;
		g->gcstepmul = data;
		break;
						   }
	default: res = -1;  /* invalid option */
	}
	lua_unlock(L);
	return res;
}



/*
** miscellaneous functions
*/


LUA_API int lua_error (lua_State *L) {
	lua_lock(L);
	api_checknelems(L, 1);
	luaG_errormsg(L);
	lua_unlock(L);
	return 0;  /* to avoid warnings */
}


LUA_API int lua_next (lua_State *L, int idx) {
	StkId t;
	int more;
	lua_lock(L);
	t = index2adr(L, idx);
	api_check(L, ttistable(t));
	more = luaH_next(L, hvalue(t), L->top - 1);
	if (more) {
		api_incr_top(L);
	}
	else  /* no more elements */
		L->top -= 1;  /* remove key */
	lua_unlock(L);
	return more;
}


LUA_API void lua_concat (lua_State *L, int n) {
	lua_lock(L);
	api_checknelems(L, n);
	if (n >= 2) {
		luaC_checkGC(L);
		luaV_concat(L, n, cast_int(L->top - L->base) - 1);
		L->top -= (n-1);
	}
	else if (n == 0) {  /* push empty string */
		setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
		api_incr_top(L);
	}
	/* else n == 1; nothing to do */
	lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
	lua_Alloc f;
	lua_lock(L);
	if (ud) *ud = G(L)->ud;
	f = G(L)->frealloc;
	lua_unlock(L);
	return f;
}


LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
	lua_lock(L);
	G(L)->ud = ud;
	G(L)->frealloc = f;
	lua_unlock(L);
}


LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
	Udata *u;
	lua_lock(L);
	luaC_checkGC(L);
	u = luaS_newudata(L, size, getcurrenv(L));
	setuvalue(L, L->top, u);
	api_incr_top(L);
	lua_unlock(L);
	return u + 1;
}




static const char *aux_upvalue (StkId fi, int n, TValue **val) {
	Closure *f;
	if (!ttisfunction(fi)) return NULL;
	f = clvalue(fi);
	if (f->c.isC) {
		if (!(1 <= n && n <= f->c.nupvalues)) return NULL;
		*val = &f->c.upvalue[n-1];
		return "";
	}
	else {
		Proto *p = f->l.p;
		if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
		*val = f->l.upvals[n-1]->v;
		return getstr(p->upvalues[n-1]);
	}
}


LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
	const char *name;
	TValue *val;
	lua_lock(L);
	name = aux_upvalue(index2adr(L, funcindex), n, &val);
	if (name) {
		setobj2s(L, L->top, val);
		api_incr_top(L);
	}
	lua_unlock(L);
	return name;
}


LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
	const char *name;
	TValue *val;
	StkId fi;
	lua_lock(L);
	fi = index2adr(L, funcindex);
	api_checknelems(L, 1);
	name = aux_upvalue(fi, n, &val);
	if (name) {
		L->top--;
		setobj(L, val, L->top);
		luaC_barrier(L, clvalue(fi), L->top);
	}
	lua_unlock(L);
	return name;
}



//-------------------------------------------------------------lmem.c---------------------------------------



/*
** About the realloc function:
** void * frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** (`osize' is the old size, `nsize' is the new size)
**
** Lua ensures that (ptr == NULL) iff (osize == 0).
**
** * frealloc(ud, NULL, 0, x) creates a new block of size `x'
**
** * frealloc(ud, p, x, 0) frees the block `p'
** (in this specific case, frealloc must return NULL).
** particularly, frealloc(ud, NULL, 0, 0) does nothing
** (which is equivalent to free(NULL) in ANSI C)
**
** frealloc returns NULL if it cannot create or reallocate the area
** (any reallocation to an equal or smaller size cannot fail!)
*/



#define MINSIZEARRAY	4


void *luaM_growaux_ (lua_State *L, void *block, int *size, size_t size_elems,
					 int limit, const char *errormsg) {
						 void *newblock;
						 int newsize;
						 if (*size >= limit/2) {  /* cannot double it? */
							 if (*size >= limit)  /* cannot grow even a little? */
								 luaG_runerror(L, errormsg);
							 newsize = limit;  /* still have at least one free place */
						 }
						 else {
							 newsize = (*size)*2;
							 if (newsize < MINSIZEARRAY)
								 newsize = MINSIZEARRAY;  /* minimum size */
						 }
						 newblock = luaM_reallocv(L, block, *size, newsize, size_elems);
						 *size = newsize;  /* update only when everything else is OK */
						 return newblock;
}


void *luaM_toobig (lua_State *L) {
	luaG_runerror(L, "memory allocation error: block too big");
	return NULL;  /* to avoid warnings */
}



/*
** generic allocation routine.
*/
void *luaM_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize) {
	global_State *g = G(L);
	lua_assert((osize == 0) == (block == NULL));
	block = (*g->frealloc)(g->ud, block, osize, nsize);
	if (block == NULL && nsize > 0)
		luaD_throw(L, LUA_ERRMEM);
	lua_assert((nsize == 0) == (block == NULL));
	g->totalbytes = (g->totalbytes - osize) + nsize;
	return block;
}

//-------------------------------------------------------------lparser.c---------------------------------------


typedef struct LG {
	lua_State l;
	global_State g;
} LG;

/*
** $Id: lparser.c,v 2.42a 2006/06/05 15:57:59 roberto Exp $
** Lua Parser
** See Copyright Notice in lua.h
*/


//#include <string.h>

#define lparser_c
#define LUA_CORE

//#include "lua.h"

//#include "lcode.h"
//#include "ldebug.h"
//#include "ldo.h"
//#include "lfunc.h"
//#include "llex.h"
//#include "lmem.h"
//#include "lobject.h"
//#include "lopcodes.h"
//#include "lparser.h"
//#include "lstate.h"
//#include "lstring.h"
//#include "ltable.h"



#define hasmultret(k)		((k) == VCALL || (k) == VVARARG)

#define getlocvar(fs, i)	((fs)->f->locvars[(fs)->actvar[i]])

#define luaY_checklimit(fs,v,l,m)	if ((v)>(l)) errorlimit(fs,l,m)


/*
** nodes for block list (list of active blocks)
*/
typedef struct BlockCnt {
	struct BlockCnt *previous;  /* chain */
	int breaklist;  /* list of jumps out of this loop */
	lu_byte nactvar;  /* # active locals outside the breakable structure */
	lu_byte upval;  /* true if some variable in the block is an upvalue */
	lu_byte isbreakable;  /* true if `block' is a loop */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
static void chunk (LexState *ls);
static void expr (LexState *ls, expdesc *v);


static void anchor_token (LexState *ls) {
	if (ls->t.token == TK_NAME || ls->t.token == TK_STRING) {
		TString *ts = ls->t.seminfo.ts;
		luaX_newstring(ls, getstr(ts), ts->tsv.len);
	}
}


static void error_expected (LexState *ls, int token) {
	luaX_syntaxerror(ls,
		luaO_pushfstring(ls->L, LUA_QS " expected", luaX_token2str(ls, token)));
}


static void errorlimit (FuncState *fs, int limit, const char *what) {
	const char *msg = (fs->f->linedefined == 0) ?
		luaO_pushfstring(fs->L, "main function has more than %d %s", limit, what) :
	luaO_pushfstring(fs->L, "function at line %d has more than %d %s",
		fs->f->linedefined, limit, what);
	luaX_lexerror(fs->ls, msg, 0);
}


static int testnext (LexState *ls, int c) {
	if (ls->t.token == c) {
		luaX_next(ls);
		return 1;
	}
	else return 0;
}


static void check (LexState *ls, int c) {
	if (ls->t.token != c)
		error_expected(ls, c);
}

static void checknext (LexState *ls, int c) {
	check(ls, c);
	luaX_next(ls);
}


#define check_condition(ls,c,msg)	{ if (!(c)) luaX_syntaxerror(ls, msg); }



static void check_match (LexState *ls, int what, int who, int where) {
	if (!testnext(ls, what)) {
		if (where == ls->linenumber)
			error_expected(ls, what);
		else {
			luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
				LUA_QS " expected (to close " LUA_QS " at line %d)",
				luaX_token2str(ls, what), luaX_token2str(ls, who), where));
		}
	}
}


static TString *str_checkname (LexState *ls) {
	TString *ts;
	check(ls, TK_NAME);
	ts = ls->t.seminfo.ts;
	luaX_next(ls);
	return ts;
}


static void init_exp (expdesc *e, expkind k, int i) {
	e->f = e->t = NO_JUMP;
	e->k = k;
	e->u.s.info = i;
}


static void codestring (LexState *ls, expdesc *e, TString *s) {
	init_exp(e, VK, luaK_stringK(ls->fs, s));
}


static void checkname(LexState *ls, expdesc *e) {
	codestring(ls, e, str_checkname(ls));
}


static int registerlocalvar (LexState *ls, TString *varname) {
	FuncState *fs = ls->fs;
	Proto *f = fs->f;
	int oldsize = f->sizelocvars;
	luaM_growvector(ls->L, f->locvars, fs->nlocvars, f->sizelocvars,
		LocVar, SHRT_MAX, "too many local variables");
	while (oldsize < f->sizelocvars) f->locvars[oldsize++].varname = NULL;
	f->locvars[fs->nlocvars].varname = varname;
	luaC_objbarrier(ls->L, f, varname);
	return fs->nlocvars++;
}


#define new_localvarliteral(ls,v,n) \
	new_localvar(ls, luaX_newstring(ls, "" v, (sizeof(v)/sizeof(char))-1), n)


static void new_localvar (LexState *ls, TString *name, int n) {
	FuncState *fs = ls->fs;
	luaY_checklimit(fs, fs->nactvar+n+1, LUAI_MAXVARS, "local variables");
	fs->actvar[fs->nactvar+n] = cast(unsigned short, registerlocalvar(ls, name));
}


static void adjustlocalvars (LexState *ls, int nvars) {
	FuncState *fs = ls->fs;
	fs->nactvar = cast_byte(fs->nactvar + nvars);
	for (; nvars; nvars--) {
		getlocvar(fs, fs->nactvar - nvars).startpc = fs->pc;
	}
}


static void removevars (LexState *ls, int tolevel) {
	FuncState *fs = ls->fs;
	while (fs->nactvar > tolevel)
		getlocvar(fs, --fs->nactvar).endpc = fs->pc;
}


static int indexupvalue (FuncState *fs, TString *name, expdesc *v) {
	int i;
	Proto *f = fs->f;
	int oldsize = f->sizeupvalues;
	for (i=0; i<f->nups; i++) {
		if (fs->upvalues[i].k == v->k && fs->upvalues[i].info == v->u.s.info) {
			lua_assert(f->upvalues[i] == name);
			return i;
		}
	}
	/* new one */
	luaY_checklimit(fs, f->nups + 1, LUAI_MAXUPVALUES, "upvalues");
	luaM_growvector(fs->L, f->upvalues, f->nups, f->sizeupvalues,
		TString *, MAX_INT, "");
	while (oldsize < f->sizeupvalues) f->upvalues[oldsize++] = NULL;
	f->upvalues[f->nups] = name;
	luaC_objbarrier(fs->L, f, name);
	lua_assert(v->k == VLOCAL || v->k == VUPVAL);
	fs->upvalues[f->nups].k = cast_byte(v->k);
	fs->upvalues[f->nups].info = cast_byte(v->u.s.info);
	return f->nups++;
}


static int searchvar (FuncState *fs, TString *n) {
	int i;
	for (i=fs->nactvar-1; i >= 0; i--) {
		if (n == getlocvar(fs, i).varname)
			return i;
	}
	return -1;  /* not found */
}


static void markupval (FuncState *fs, int level) {
	BlockCnt *bl = fs->bl;
	while (bl && bl->nactvar > level) bl = bl->previous;
	if (bl) bl->upval = 1;
}


static int singlevaraux (FuncState *fs, TString *n, expdesc *var, int base) {
	if (fs == NULL) {  /* no more levels? */
		init_exp(var, VGLOBAL, NO_REG);  /* default is global variable */
		return VGLOBAL;
	}
	else {
		int v = searchvar(fs, n);  /* look up at current level */
		if (v >= 0) {
			init_exp(var, VLOCAL, v);
			if (!base)
				markupval(fs, v);  /* local will be used as an upval */
			return VLOCAL;
		}
		else {  /* not found at current level; try upper one */
			if (singlevaraux(fs->prev, n, var, 0) == VGLOBAL)
				return VGLOBAL;
			var->u.s.info = indexupvalue(fs, n, var);  /* else was LOCAL or UPVAL */
			var->k = VUPVAL;  /* upvalue in this level */
			return VUPVAL;
		}
	}
}


static void singlevar (LexState *ls, expdesc *var) {
	TString *varname = str_checkname(ls);
	FuncState *fs = ls->fs;
	if (singlevaraux(fs, varname, var, 1) == VGLOBAL)
		var->u.s.info = luaK_stringK(fs, varname);  /* info points to global name */
}


static void adjust_assign (LexState *ls, int nvars, int nexps, expdesc *e) {
	FuncState *fs = ls->fs;
	int extra = nvars - nexps;
	if (hasmultret(e->k)) {
		extra++;  /* includes call itself */
		if (extra < 0) extra = 0;
		luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
		if (extra > 1) luaK_reserveregs(fs, extra-1);
	}
	else {
		if (e->k != VVOID) luaK_exp2nextreg(fs, e);  /* close last expression */
		if (extra > 0) {
			int reg = fs->freereg;
			luaK_reserveregs(fs, extra);
			luaK_nil(fs, reg, extra);
		}
	}
}


static void enterlevel (LexState *ls) {
	if (++ls->L->nCcalls > LUAI_MAXCCALLS)
		luaX_lexerror(ls, "chunk has too many syntax levels", 0);
}


#define leavelevel(ls)	((ls)->L->nCcalls--)


static void enterblock (FuncState *fs, BlockCnt *bl, lu_byte isbreakable) {
	bl->breaklist = NO_JUMP;
	bl->isbreakable = isbreakable;
	bl->nactvar = fs->nactvar;
	bl->upval = 0;
	bl->previous = fs->bl;
	fs->bl = bl;
	lua_assert(fs->freereg == fs->nactvar);
}


static void leaveblock (FuncState *fs) {
	BlockCnt *bl = fs->bl;
	fs->bl = bl->previous;
	removevars(fs->ls, bl->nactvar);
	if (bl->upval)
		luaK_codeABC(fs, OP_CLOSE, bl->nactvar, 0, 0);
	/* a block either controls scope or breaks (never both) */
	lua_assert(!bl->isbreakable || !bl->upval);
	lua_assert(bl->nactvar == fs->nactvar);
	fs->freereg = fs->nactvar;  /* free registers */
	luaK_patchtohere(fs, bl->breaklist);
}


static void pushclosure (LexState *ls, FuncState *func, expdesc *v) {
	FuncState *fs = ls->fs;
	Proto *f = fs->f;
	int oldsize = f->sizep;
	int i;
	luaM_growvector(ls->L, f->p, fs->np, f->sizep, Proto *,
		MAXARG_Bx, "constant table overflow");
	while (oldsize < f->sizep) f->p[oldsize++] = NULL;
	f->p[fs->np++] = func->f;
	luaC_objbarrier(ls->L, f, func->f);
	init_exp(v, VRELOCABLE, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np-1));
	for (i=0; i<func->f->nups; i++) {
		OpCode o = (func->upvalues[i].k == VLOCAL) ? OP_MOVE : OP_GETUPVAL;
		luaK_codeABC(fs, o, 0, func->upvalues[i].info, 0);
	}
}


static void open_func (LexState *ls, FuncState *fs) {
	lua_State *L = ls->L;
	Proto *f = luaF_newproto(L);
	fs->f = f;
	fs->prev = ls->fs;  /* linked list of funcstates */
	fs->ls = ls;
	fs->L = L;
	ls->fs = fs;
	fs->pc = 0;
	fs->lasttarget = -1;
	fs->jpc = NO_JUMP;
	fs->freereg = 0;
	fs->nk = 0;
	fs->np = 0;
	fs->nlocvars = 0;
	fs->nactvar = 0;
	fs->bl = NULL;
	f->source = ls->source;
	f->maxstacksize = 2;  /* registers 0/1 are always valid */
	fs->h = luaH_new(L, 0, 0);
	/* anchor table of constants and prototype (to avoid being collected) */
	sethvalue2s(L, L->top, fs->h);
	incr_top(L);
	setptvalue2s(L, L->top, f);
	incr_top(L);
}


static void close_func (LexState *ls) {
	lua_State *L = ls->L;
	FuncState *fs = ls->fs;
	Proto *f = fs->f;
	removevars(ls, 0);
	luaK_ret(fs, 0, 0);  /* final return */
	luaM_reallocvector(L, f->code, f->sizecode, fs->pc, Instruction);
	f->sizecode = fs->pc;
	luaM_reallocvector(L, f->lineinfo, f->sizelineinfo, fs->pc, int);
	f->sizelineinfo = fs->pc;
	luaM_reallocvector(L, f->k, f->sizek, fs->nk, TValue);
	f->sizek = fs->nk;
	luaM_reallocvector(L, f->p, f->sizep, fs->np, Proto *);
	f->sizep = fs->np;
	luaM_reallocvector(L, f->locvars, f->sizelocvars, fs->nlocvars, LocVar);
	f->sizelocvars = fs->nlocvars;
	luaM_reallocvector(L, f->upvalues, f->sizeupvalues, f->nups, TString *);
	f->sizeupvalues = f->nups;
	lua_assert(luaG_checkcode(f));
	lua_assert(fs->bl == NULL);
	ls->fs = fs->prev;
	L->top -= 2;  /* remove table and prototype from the stack */
	/* last token read was anchored in defunct function; must reanchor it */
	if (fs) anchor_token(ls);
}


Proto *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff, const char *name) {
	struct LexState lexstate;
	struct FuncState funcstate;
	lexstate.buff = buff;
	luaX_setinput(L, &lexstate, z, luaS_new(L, name));
	open_func(&lexstate, &funcstate);
	funcstate.f->is_vararg = VARARG_ISVARARG;  /* main func. is always vararg */
	luaX_next(&lexstate);  /* read first token */
	chunk(&lexstate);
	check(&lexstate, TK_EOS);
	close_func(&lexstate);
	lua_assert(funcstate.prev == NULL);
	lua_assert(funcstate.f->nups == 0);
	lua_assert(lexstate.fs == NULL);
	return funcstate.f;
}



/*============================================================*/
/* GRAMMAR RULES */
/*============================================================*/


static void field (LexState *ls, expdesc *v) {
	/* field -> ['.' | ':'] NAME */
	FuncState *fs = ls->fs;
	expdesc key;
	luaK_exp2anyreg(fs, v);
	luaX_next(ls);  /* skip the dot or colon */
	checkname(ls, &key);
	luaK_indexed(fs, v, &key);
}


static void yindex (LexState *ls, expdesc *v) {
	/* index -> '[' expr ']' */
	luaX_next(ls);  /* skip the '[' */
	expr(ls, v);
	luaK_exp2val(ls->fs, v);
	checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/


struct ConsControl {
	expdesc v;  /* last list item read */
	expdesc *t;  /* table descriptor */
	int nh;  /* total number of `record' elements */
	int na;  /* total number of array elements */
	int tostore;  /* number of array elements pending to be stored */
};


static void recfield (LexState *ls, struct ConsControl *cc) {
	/* recfield -> (NAME | `['exp1`]') = exp1 */
	FuncState *fs = ls->fs;
	int reg = ls->fs->freereg;
	expdesc key, val;
	int rkkey;
	if (ls->t.token == TK_NAME) {
		luaY_checklimit(fs, cc->nh, MAX_INT, "items in a constructor");
		checkname(ls, &key);
	}
	else  /* ls->t.token == '[' */
		yindex(ls, &key);
	cc->nh++;
	checknext(ls, '=');
	rkkey = luaK_exp2RK(fs, &key);
	expr(ls, &val);
	luaK_codeABC(fs, OP_SETTABLE, cc->t->u.s.info, rkkey, luaK_exp2RK(fs, &val));
	fs->freereg = reg;  /* free registers */
}


static void closelistfield (FuncState *fs, struct ConsControl *cc) {
	if (cc->v.k == VVOID) return;  /* there is no list item */
	luaK_exp2nextreg(fs, &cc->v);
	cc->v.k = VVOID;
	if (cc->tostore == LFIELDS_PER_FLUSH) {
		luaK_setlist(fs, cc->t->u.s.info, cc->na, cc->tostore);  /* flush */
		cc->tostore = 0;  /* no more items pending */
	}
}


static void lastlistfield (FuncState *fs, struct ConsControl *cc) {
	if (cc->tostore == 0) return;
	if (hasmultret(cc->v.k)) {
		luaK_setmultret(fs, &cc->v);
		luaK_setlist(fs, cc->t->u.s.info, cc->na, LUA_MULTRET);
		cc->na--;  /* do not count last expression (unknown number of elements) */
	}
	else {
		if (cc->v.k != VVOID)
			luaK_exp2nextreg(fs, &cc->v);
		luaK_setlist(fs, cc->t->u.s.info, cc->na, cc->tostore);
	}
}


static void listfield (LexState *ls, struct ConsControl *cc) {
	expr(ls, &cc->v);
	luaY_checklimit(ls->fs, cc->na, MAX_INT, "items in a constructor");
	cc->na++;
	cc->tostore++;
}


static void constructor (LexState *ls, expdesc *t) {
	/* constructor -> ?? */
	FuncState *fs = ls->fs;
	int line = ls->linenumber;
	int pc = luaK_codeABC(fs, OP_NEWTABLE, 0, 0, 0);
	struct ConsControl cc;
	cc.na = cc.nh = cc.tostore = 0;
	cc.t = t;
	init_exp(t, VRELOCABLE, pc);
	init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
	luaK_exp2nextreg(ls->fs, t);  /* fix it at stack top (for gc) */
	checknext(ls, '{');
	do {
		lua_assert(cc.v.k == VVOID || cc.tostore > 0);
		if (ls->t.token == '}') break;
		closelistfield(fs, &cc);
		switch(ls->t.token) {
	  case TK_NAME: {  /* may be listfields or recfields */
		  luaX_lookahead(ls);
		  if (ls->lookahead.token != '=')  /* expression? */
			  listfield(ls, &cc);
		  else
			  recfield(ls, &cc);
		  break;
					}
	  case '[': {  /* constructor_item -> recfield */
		  recfield(ls, &cc);
		  break;
				}
	  default: {  /* constructor_part -> listfield */
		  listfield(ls, &cc);
		  break;
			   }
		}
	} while (testnext(ls, ',') || testnext(ls, ';'));
	check_match(ls, '}', '{', line);
	lastlistfield(fs, &cc);
	SETARG_B(fs->f->code[pc], luaO_int2fb(cc.na)); /* set initial array size */
	SETARG_C(fs->f->code[pc], luaO_int2fb(cc.nh));  /* set initial table size */
}

/* }====================================================================== */



static void parlist (LexState *ls) {
	/* parlist -> [ param { `,' param } ] */
	FuncState *fs = ls->fs;
	Proto *f = fs->f;
	int nparams = 0;
	f->is_vararg = 0;
	if (ls->t.token != ')') {  /* is `parlist' not empty? */
		do {
			switch (ls->t.token) {
		case TK_NAME: {  /* param -> NAME */
			new_localvar(ls, str_checkname(ls), nparams++);
			break;
					  }
		case TK_DOTS: {  /* param -> `...' */
			luaX_next(ls);
#if defined(LUA_COMPAT_VARARG)
			/* use `arg' as default name */
			new_localvarliteral(ls, "arg", nparams++);
			f->is_vararg = VARARG_HASARG | VARARG_NEEDSARG;
#endif
			f->is_vararg |= VARARG_ISVARARG;
			break;
					  }
		default: luaX_syntaxerror(ls, "<name> or " LUA_QL("...") " expected");
			}
		} while (!f->is_vararg && testnext(ls, ','));
	}
	adjustlocalvars(ls, nparams);
	f->numparams = cast_byte(fs->nactvar - (f->is_vararg & VARARG_HASARG));
	luaK_reserveregs(fs, fs->nactvar);  /* reserve register for parameters */
}


static void body (LexState *ls, expdesc *e, int needself, int line) {
	/* body ->  `(' parlist `)' chunk END */
	FuncState new_fs;
	open_func(ls, &new_fs);
	new_fs.f->linedefined = line;
	checknext(ls, '(');
	if (needself) {
		new_localvarliteral(ls, "self", 0);
		adjustlocalvars(ls, 1);
	}
	parlist(ls);
	checknext(ls, ')');
	chunk(ls);
	new_fs.f->lastlinedefined = ls->linenumber;
	check_match(ls, TK_END, TK_FUNCTION, line);
	close_func(ls);
	pushclosure(ls, &new_fs, e);
}


static int explist1 (LexState *ls, expdesc *v) {
	/* explist1 -> expr { `,' expr } */
	int n = 1;  /* at least one expression */
	expr(ls, v);
	while (testnext(ls, ',')) {
		luaK_exp2nextreg(ls->fs, v);
		expr(ls, v);
		n++;
	}
	return n;
}


static void funcargs (LexState *ls, expdesc *f) {
	FuncState *fs = ls->fs;
	expdesc args;
	int base, nparams;
	int line = ls->linenumber;
	switch (ls->t.token) {
	case '(': {  /* funcargs -> `(' [ explist1 ] `)' */
		if (line != ls->lastline)
			luaX_syntaxerror(ls,"ambiguous syntax (function call x new statement)");
		luaX_next(ls);
		if (ls->t.token == ')')  /* arg list is empty? */
			args.k = VVOID;
		else {
			explist1(ls, &args);
			luaK_setmultret(fs, &args);
		}
		check_match(ls, ')', '(', line);
		break;
			  }
	case '{': {  /* funcargs -> constructor */
		constructor(ls, &args);
		break;
			  }
	case TK_STRING: {  /* funcargs -> STRING */
		codestring(ls, &args, ls->t.seminfo.ts);
		luaX_next(ls);  /* must use `seminfo' before `next' */
		break;
					}
	default: {
		luaX_syntaxerror(ls, "function arguments expected");
		return;
			 }
	}
	lua_assert(f->k == VNONRELOC);
	base = f->u.s.info;  /* base register for call */
	if (hasmultret(args.k))
		nparams = LUA_MULTRET;  /* open call */
	else {
		if (args.k != VVOID)
			luaK_exp2nextreg(fs, &args);  /* close last argument */
		nparams = fs->freereg - (base+1);
	}
	init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
	luaK_fixline(fs, line);
	fs->freereg = base+1;  /* call remove function and arguments and leaves
						   (unless changed) one result */
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


static void prefixexp (LexState *ls, expdesc *v) {
	/* prefixexp -> NAME | '(' expr ')' */
	switch (ls->t.token) {
	case '(': {
		int line = ls->linenumber;
		luaX_next(ls);
		expr(ls, v);
		check_match(ls, ')', '(', line);
		luaK_dischargevars(ls->fs, v);
		return;
			  }
	case TK_NAME: {
		singlevar(ls, v);
		return;
				  }
	default: {
		luaX_syntaxerror(ls, "unexpected symbol");
		return;
			 }
	}
}


static void primaryexp (LexState *ls, expdesc *v) {
	/* primaryexp ->
	prefixexp { `.' NAME | `[' exp `]' | `:' NAME funcargs | funcargs } */
	FuncState *fs = ls->fs;
	prefixexp(ls, v);
	for (;;) {
		switch (ls->t.token) {
	  case '.': {  /* field */
		  field(ls, v);
		  break;
				}
	  case '[': {  /* `[' exp1 `]' */
		  expdesc key;
		  luaK_exp2anyreg(fs, v);
		  yindex(ls, &key);
		  luaK_indexed(fs, v, &key);
		  break;
				}
	  case ':': {  /* `:' NAME funcargs */
		  expdesc key;
		  luaX_next(ls);
		  checkname(ls, &key);
		  luaK_self(fs, v, &key);
		  funcargs(ls, v);
		  break;
				}
	  case '(': case TK_STRING: case '{': {  /* funcargs */
		  luaK_exp2nextreg(fs, v);
		  funcargs(ls, v);
		  break;
				}
	  default: return;
		}
	}
}


static void simpleexp (LexState *ls, expdesc *v) {
	/* simpleexp -> NUMBER | STRING | NIL | true | false | ... |
	constructor | FUNCTION body | primaryexp */
	switch (ls->t.token) {
	case TK_NUMBER: {
		init_exp(v, VKNUM, 0);
		v->u.nval = ls->t.seminfo.r;
		break;
					}
	case TK_STRING: {
		codestring(ls, v, ls->t.seminfo.ts);
		break;
					}
	case TK_NIL: {
		init_exp(v, VNIL, 0);
		break;
				 }
	case TK_TRUE: {
		init_exp(v, VTRUE, 0);
		break;
				  }
	case TK_FALSE: {
		init_exp(v, VFALSE, 0);
		break;
				   }
	case TK_DOTS: {  /* vararg */
		FuncState *fs = ls->fs;
		check_condition(ls, fs->f->is_vararg,
			"cannot use " LUA_QL("...") " outside a vararg function");
		fs->f->is_vararg &= ~VARARG_NEEDSARG;  /* don't need 'arg' */
		init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, 1, 0));
		break;
				  }
	case '{': {  /* constructor */
		constructor(ls, v);
		return;
			  }
	case TK_FUNCTION: {
		luaX_next(ls);
		body(ls, v, 0, ls->linenumber);
		return;
					  }
	default: {
		primaryexp(ls, v);
		return;
			 }
	}
	luaX_next(ls);
}


static UnOpr getunopr (int op) {
	switch (op) {
	case TK_NOT: return OPR_NOT;
	case '-': return OPR_MINUS;
	case '#': return OPR_LEN;
	default: return OPR_NOUNOPR;
	}
}


static BinOpr getbinopr (int op) {
	switch (op) {
	case '+': return OPR_ADD;
	case '-': return OPR_SUB;
	case '*': return OPR_MUL;
	case '/': return OPR_DIV;
	case '%': return OPR_MOD;
	case '^': return OPR_POW;
	case TK_CONCAT: return OPR_CONCAT;
	case TK_NE: return OPR_NE;
	case TK_EQ: return OPR_EQ;
	case '<': return OPR_LT;
	case TK_LE: return OPR_LE;
	case '>': return OPR_GT;
	case TK_GE: return OPR_GE;
	case TK_AND: return OPR_AND;
	case TK_OR: return OPR_OR;
	default: return OPR_NOBINOPR;
	}
}


static const struct {
	lu_byte left;  /* left priority for each binary operator */
	lu_byte right; /* right priority */
} priority[] = {  /* ORDER OPR */
	{6, 6}, {6, 6}, {7, 7}, {7, 7}, {7, 7},  /* `+' `-' `/' `%' */
	{10, 9}, {5, 4},                 /* power and concat (right associative) */
	{3, 3}, {3, 3},                  /* equality and inequality */
	{3, 3}, {3, 3}, {3, 3}, {3, 3},  /* order */
	{2, 2}, {1, 1}                   /* logical (and/or) */
};

#define UNARY_PRIORITY	8  /* priority for unary operators */


/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where `binop' is any binary operator with a priority higher than `limit'
*/
static BinOpr subexpr (LexState *ls, expdesc *v, unsigned int limit) {
	BinOpr op;
	UnOpr uop;
	enterlevel(ls);
	uop = getunopr(ls->t.token);
	if (uop != OPR_NOUNOPR) {
		luaX_next(ls);
		subexpr(ls, v, UNARY_PRIORITY);
		luaK_prefix(ls->fs, uop, v);
	}
	else simpleexp(ls, v);
	/* expand while operators have priorities higher than `limit' */
	op = getbinopr(ls->t.token);
	while (op != OPR_NOBINOPR && priority[op].left > limit) {
		expdesc v2;
		BinOpr nextop;
		luaX_next(ls);
		luaK_infix(ls->fs, op, v);
		/* read sub-expression with higher priority */
		nextop = subexpr(ls, &v2, priority[op].right);
		luaK_posfix(ls->fs, op, v, &v2);
		op = nextop;
	}
	leavelevel(ls);
	return op;  /* return first untreated operator */
}


static void expr (LexState *ls, expdesc *v) {
	subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


static int block_follow (int token) {
	switch (token) {
	case TK_ELSE: case TK_ELSEIF: case TK_END:
	case TK_UNTIL: case TK_EOS:
		return 1;
	default: return 0;
	}
}


static void block (LexState *ls) {
	/* block -> chunk */
	FuncState *fs = ls->fs;
	BlockCnt bl;
	enterblock(fs, &bl, 0);
	chunk(ls);
	lua_assert(bl.breaklist == NO_JUMP);
	leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** assignment
*/
struct LHS_assign {
	struct LHS_assign *prev;
	expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to a local variable, the local variable
** is needed in a previous assignment (to a table). If so, save original
** local value in a safe place and use this safe copy in the previous
** assignment.
*/
static void check_conflict (LexState *ls, struct LHS_assign *lh, expdesc *v) {
	FuncState *fs = ls->fs;
	int extra = fs->freereg;  /* eventual position to save local variable */
	int conflict = 0;
	for (; lh; lh = lh->prev) {
		if (lh->v.k == VINDEXED) {
			if (lh->v.u.s.info == v->u.s.info) {  /* conflict? */
				conflict = 1;
				lh->v.u.s.info = extra;  /* previous assignment will use safe copy */
			}
			if (lh->v.u.s.aux == v->u.s.info) {  /* conflict? */
				conflict = 1;
				lh->v.u.s.aux = extra;  /* previous assignment will use safe copy */
			}
		}
	}
	if (conflict) {
		luaK_codeABC(fs, OP_MOVE, fs->freereg, v->u.s.info, 0);  /* make copy */
		luaK_reserveregs(fs, 1);
	}
}


static void assignment (LexState *ls, struct LHS_assign *lh, int nvars) {
	expdesc e;
	check_condition(ls, VLOCAL <= lh->v.k && lh->v.k <= VINDEXED,
		"syntax error");
	if (testnext(ls, ',')) {  /* assignment -> `,' primaryexp assignment */
		struct LHS_assign nv;
		nv.prev = lh;
		primaryexp(ls, &nv.v);
		if (nv.v.k == VLOCAL)
			check_conflict(ls, lh, &nv.v);
		assignment(ls, &nv, nvars+1);
	}
	else {  /* assignment -> `=' explist1 */
		int nexps;
		checknext(ls, '=');
		nexps = explist1(ls, &e);
		if (nexps != nvars) {
			adjust_assign(ls, nvars, nexps, &e);
			if (nexps > nvars)
				ls->fs->freereg -= nexps - nvars;  /* remove extra values */
		}
		else {
			luaK_setoneret(ls->fs, &e);  /* close last expression */
			luaK_storevar(ls->fs, &lh->v, &e);
			return;  /* avoid default */
		}
	}
	init_exp(&e, VNONRELOC, ls->fs->freereg-1);  /* default assignment */
	luaK_storevar(ls->fs, &lh->v, &e);
}


static int cond (LexState *ls) {
	/* cond -> exp */
	expdesc v;
	expr(ls, &v);  /* read condition */
	if (v.k == VNIL) v.k = VFALSE;  /* `falses' are all equal here */
	luaK_goiftrue(ls->fs, &v);
	return v.f;
}


static void breakstat (LexState *ls) {
	FuncState *fs = ls->fs;
	BlockCnt *bl = fs->bl;
	int upval = 0;
	while (bl && !bl->isbreakable) {
		upval |= bl->upval;
		bl = bl->previous;
	}
	if (!bl)
		luaX_syntaxerror(ls, "no loop to break");
	if (upval)
		luaK_codeABC(fs, OP_CLOSE, bl->nactvar, 0, 0);
	luaK_concat(fs, &bl->breaklist, luaK_jump(fs));
}


static void whilestat (LexState *ls, int line) {
	/* whilestat -> WHILE cond DO block END */
	FuncState *fs = ls->fs;
	int whileinit;
	int condexit;
	BlockCnt bl;
	luaX_next(ls);  /* skip WHILE */
	whileinit = luaK_getlabel(fs);
	condexit = cond(ls);
	enterblock(fs, &bl, 1);
	checknext(ls, TK_DO);
	block(ls);
	luaK_patchlist(fs, luaK_jump(fs), whileinit);
	check_match(ls, TK_END, TK_WHILE, line);
	leaveblock(fs);
	luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


static void repeatstat (LexState *ls, int line) {
	/* repeatstat -> REPEAT block UNTIL cond */
	int condexit;
	FuncState *fs = ls->fs;
	int repeat_init = luaK_getlabel(fs);
	BlockCnt bl1, bl2;
	enterblock(fs, &bl1, 1);  /* loop block */
	enterblock(fs, &bl2, 0);  /* scope block */
	luaX_next(ls);  /* skip REPEAT */
	chunk(ls);
	check_match(ls, TK_UNTIL, TK_REPEAT, line);
	condexit = cond(ls);  /* read condition (inside scope block) */
	if (!bl2.upval) {  /* no upvalues? */
		leaveblock(fs);  /* finish scope */
		luaK_patchlist(ls->fs, condexit, repeat_init);  /* close the loop */
	}
	else {  /* complete semantics when there are upvalues */
		breakstat(ls);  /* if condition then break */
		luaK_patchtohere(ls->fs, condexit);  /* else... */
		leaveblock(fs);  /* finish scope... */
		luaK_patchlist(ls->fs, luaK_jump(fs), repeat_init);  /* and repeat */
	}
	leaveblock(fs);  /* finish loop */
}


static int exp1 (LexState *ls) {
	expdesc e;
	int k;
	expr(ls, &e);
	k = e.k;
	luaK_exp2nextreg(ls->fs, &e);
	return k;
}


static void forbody (LexState *ls, int base, int line, int nvars, int isnum) {
	/* forbody -> DO block */
	BlockCnt bl;
	FuncState *fs = ls->fs;
	int prep, endfor;
	adjustlocalvars(ls, 3);  /* control variables */
	checknext(ls, TK_DO);
	prep = isnum ? luaK_codeAsBx(fs, OP_FORPREP, base, NO_JUMP) : luaK_jump(fs);
	enterblock(fs, &bl, 0);  /* scope for declared variables */
	adjustlocalvars(ls, nvars);
	luaK_reserveregs(fs, nvars);
	block(ls);
	leaveblock(fs);  /* end of scope for declared variables */
	luaK_patchtohere(fs, prep);
	endfor = (isnum) ? luaK_codeAsBx(fs, OP_FORLOOP, base, NO_JUMP) :
		luaK_codeABC(fs, OP_TFORLOOP, base, 0, nvars);
	luaK_fixline(fs, line);  /* pretend that `OP_FOR' starts the loop */
	luaK_patchlist(fs, (isnum ? endfor : luaK_jump(fs)), prep + 1);
}


static void fornum (LexState *ls, TString *varname, int line) {
	/* fornum -> NAME = exp1,exp1[,exp1] forbody */
	FuncState *fs = ls->fs;
	int base = fs->freereg;
	new_localvarliteral(ls, "(for index)", 0);
	new_localvarliteral(ls, "(for limit)", 1);
	new_localvarliteral(ls, "(for step)", 2);
	new_localvar(ls, varname, 3);
	checknext(ls, '=');
	exp1(ls);  /* initial value */
	checknext(ls, ',');
	exp1(ls);  /* limit */
	if (testnext(ls, ','))
		exp1(ls);  /* optional step */
	else {  /* default step = 1 */
		luaK_codeABx(fs, OP_LOADK, fs->freereg, luaK_numberK(fs, 1));
		luaK_reserveregs(fs, 1);
	}
	forbody(ls, base, line, 1, 1);
}


static void forlist (LexState *ls, TString *indexname) {
	/* forlist -> NAME {,NAME} IN explist1 forbody */
	FuncState *fs = ls->fs;
	expdesc e;
	int nvars = 0;
	int line;
	int base = fs->freereg;
	/* create control variables */
	new_localvarliteral(ls, "(for generator)", nvars++);
	new_localvarliteral(ls, "(for state)", nvars++);
	new_localvarliteral(ls, "(for control)", nvars++);
	/* create declared variables */
	new_localvar(ls, indexname, nvars++);
	while (testnext(ls, ','))
		new_localvar(ls, str_checkname(ls), nvars++);
	checknext(ls, TK_IN);
	line = ls->linenumber;
	adjust_assign(ls, 3, explist1(ls, &e), &e);
	luaK_checkstack(fs, 3);  /* extra space to call generator */
	forbody(ls, base, line, nvars - 3, 0);
}


static void forstat (LexState *ls, int line) {
	/* forstat -> FOR (fornum | forlist) END */
	FuncState *fs = ls->fs;
	TString *varname;
	BlockCnt bl;
	enterblock(fs, &bl, 1);  /* scope for loop and control variables */
	luaX_next(ls);  /* skip `for' */
	varname = str_checkname(ls);  /* first variable name */
	switch (ls->t.token) {
	case '=': fornum(ls, varname, line); break;
	case ',': case TK_IN: forlist(ls, varname); break;
	default: luaX_syntaxerror(ls, LUA_QL("=") " or " LUA_QL("in") " expected");
	}
	check_match(ls, TK_END, TK_FOR, line);
	leaveblock(fs);  /* loop scope (`break' jumps to this point) */
}


static int test_then_block (LexState *ls) {
	/* test_then_block -> [IF | ELSEIF] cond THEN block */
	int condexit;
	luaX_next(ls);  /* skip IF or ELSEIF */
	condexit = cond(ls);
	checknext(ls, TK_THEN);
	block(ls);  /* `then' part */
	return condexit;
}


static void ifstat (LexState *ls, int line) {
	/* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
	FuncState *fs = ls->fs;
	int flist;
	int escapelist = NO_JUMP;
	flist = test_then_block(ls);  /* IF cond THEN block */
	while (ls->t.token == TK_ELSEIF) {
		luaK_concat(fs, &escapelist, luaK_jump(fs));
		luaK_patchtohere(fs, flist);
		flist = test_then_block(ls);  /* ELSEIF cond THEN block */
	}
	if (ls->t.token == TK_ELSE) {
		luaK_concat(fs, &escapelist, luaK_jump(fs));
		luaK_patchtohere(fs, flist);
		luaX_next(ls);  /* skip ELSE (after patch, for correct line info) */
		block(ls);  /* `else' part */
	}
	else
		luaK_concat(fs, &escapelist, flist);
	luaK_patchtohere(fs, escapelist);
	check_match(ls, TK_END, TK_IF, line);
}


static void localfunc (LexState *ls) {
	expdesc v, b;
	FuncState *fs = ls->fs;
	new_localvar(ls, str_checkname(ls), 0);
	init_exp(&v, VLOCAL, fs->freereg);
	luaK_reserveregs(fs, 1);
	adjustlocalvars(ls, 1);
	body(ls, &b, 0, ls->linenumber);
	luaK_storevar(fs, &v, &b);
	/* debug information will only see the variable after this point! */
	getlocvar(fs, fs->nactvar - 1).startpc = fs->pc;
}


static void localstat (LexState *ls) {
	/* stat -> LOCAL NAME {`,' NAME} [`=' explist1] */
	int nvars = 0;
	int nexps;
	expdesc e;
	do {
		new_localvar(ls, str_checkname(ls), nvars++);
	} while (testnext(ls, ','));
	if (testnext(ls, '='))
		nexps = explist1(ls, &e);
	else {
		e.k = VVOID;
		nexps = 0;
	}
	adjust_assign(ls, nvars, nexps, &e);
	adjustlocalvars(ls, nvars);
}


static int funcname (LexState *ls, expdesc *v) {
	/* funcname -> NAME {field} [`:' NAME] */
	int needself = 0;
	singlevar(ls, v);
	while (ls->t.token == '.')
		field(ls, v);
	if (ls->t.token == ':') {
		needself = 1;
		field(ls, v);
	}
	return needself;
}


static void funcstat (LexState *ls, int line) {
	/* funcstat -> FUNCTION funcname body */
	int needself;
	expdesc v, b;
	luaX_next(ls);  /* skip FUNCTION */
	needself = funcname(ls, &v);
	body(ls, &b, needself, line);
	luaK_storevar(ls->fs, &v, &b);
	luaK_fixline(ls->fs, line);  /* definition `happens' in the first line */
}


static void exprstat (LexState *ls) {
	/* stat -> func | assignment */
	FuncState *fs = ls->fs;
	struct LHS_assign v;
	primaryexp(ls, &v.v);
	if (v.v.k == VCALL)  /* stat -> func */
		SETARG_C(getcode(fs, &v.v), 1);  /* call statement uses no results */
	else {  /* stat -> assignment */
		v.prev = NULL;
		assignment(ls, &v, 1);
	}
}


static void retstat (LexState *ls) {
	/* stat -> RETURN explist */
	FuncState *fs = ls->fs;
	expdesc e;
	int first, nret;  /* registers with returned values */
	luaX_next(ls);  /* skip RETURN */
	if (block_follow(ls->t.token) || ls->t.token == ';')
		first = nret = 0;  /* return no values */
	else {
		nret = explist1(ls, &e);  /* optional return values */
		if (hasmultret(e.k)) {
			luaK_setmultret(fs, &e);
			if (e.k == VCALL && nret == 1) {  /* tail call? */
				SET_OPCODE(getcode(fs,&e), OP_TAILCALL);
				lua_assert(GETARG_A(getcode(fs,&e)) == fs->nactvar);
			}
			first = fs->nactvar;
			nret = LUA_MULTRET;  /* return all values */
		}
		else {
			if (nret == 1)  /* only one single value? */
				first = luaK_exp2anyreg(fs, &e);
			else {
				luaK_exp2nextreg(fs, &e);  /* values must go to the `stack' */
				first = fs->nactvar;  /* return all `active' values */
				lua_assert(nret == fs->freereg - first);
			}
		}
	}
	luaK_ret(fs, first, nret);
}


static int statement (LexState *ls) {
	int line = ls->linenumber;  /* may be needed for error messages */
	switch (ls->t.token) {
	case TK_IF: {  /* stat -> ifstat */
		ifstat(ls, line);
		return 0;
				}
	case TK_WHILE: {  /* stat -> whilestat */
		whilestat(ls, line);
		return 0;
				   }
	case TK_DO: {  /* stat -> DO block END */
		luaX_next(ls);  /* skip DO */
		block(ls);
		check_match(ls, TK_END, TK_DO, line);
		return 0;
				}
	case TK_FOR: {  /* stat -> forstat */
		forstat(ls, line);
		return 0;
				 }
	case TK_REPEAT: {  /* stat -> repeatstat */
		repeatstat(ls, line);
		return 0;
					}
	case TK_FUNCTION: {
		funcstat(ls, line);  /* stat -> funcstat */
		return 0;
					  }
	case TK_LOCAL: {  /* stat -> localstat */
		luaX_next(ls);  /* skip LOCAL */
		if (testnext(ls, TK_FUNCTION))  /* local function? */
			localfunc(ls);
		else
			localstat(ls);
		return 0;
				   }
	case TK_RETURN: {  /* stat -> retstat */
		retstat(ls);
		return 1;  /* must be last statement */
					}
	case TK_BREAK: {  /* stat -> breakstat */
		luaX_next(ls);  /* skip BREAK */
		breakstat(ls);
		return 1;  /* must be last statement */
				   }
	default: {
		exprstat(ls);
		return 0;  /* to avoid warnings */
			 }
	}
}


static void chunk (LexState *ls) {
	/* chunk -> { stat [`;'] } */
	int islast = 0;
	enterlevel(ls);
	while (!islast && !block_follow(ls->t.token)) {
		islast = statement(ls);
		testnext(ls, ';');
		lua_assert(ls->fs->f->maxstacksize >= ls->fs->freereg &&
			ls->fs->freereg >= ls->fs->nactvar);
		ls->fs->freereg = ls->fs->nactvar;  /* free registers */
	}
	leavelevel(ls);
}

/* }====================================================================== */

//-------------------------------------------------------------lstate.c---------------------------------------

static void stack_init (lua_State *L1, lua_State *L) {
	/* initialize CallInfo array */
	L1->base_ci = luaM_newvector(L, BASIC_CI_SIZE, CallInfo);
	L1->ci = L1->base_ci;
	L1->size_ci = BASIC_CI_SIZE;
	L1->end_ci = L1->base_ci + L1->size_ci - 1;
	/* initialize stack array */
	L1->stack = luaM_newvector(L, BASIC_STACK_SIZE + EXTRA_STACK, TValue);
	L1->stacksize = BASIC_STACK_SIZE + EXTRA_STACK;
	L1->top = L1->stack;
	L1->stack_last = L1->stack+(L1->stacksize - EXTRA_STACK)-1;
	/* initialize first ci */
	L1->ci->func = L1->top;
	setnilvalue(L1->top++);  /* `function' entry for this `ci' */
	L1->base = L1->ci->base = L1->top;
	L1->ci->top = L1->top + LUA_MINSTACK;
}


static void freestack (lua_State *L, lua_State *L1) {
	luaM_freearray(L, L1->base_ci, L1->size_ci, CallInfo);
	luaM_freearray(L, L1->stack, L1->stacksize, TValue);
}


/*
** open parts that may cause memory-allocation errors
*/
static void f_luaopen (lua_State *L, void *ud) {
	global_State *g = G(L);
	UNUSED(ud);
	stack_init(L, L);  /* init stack */
	sethvalue(L, gt(L), luaH_new(L, 0, 2));  /* table of globals */
	sethvalue(L, registry(L), luaH_new(L, 0, 2));  /* registry */
	luaS_resize(L, MINSTRTABSIZE);  /* initial size of string table */
	luaT_init(L);
	luaX_init(L);
	luaS_fix(luaS_newliteral(L, MEMERRMSG));
	g->GCthreshold = 4*g->totalbytes;
}


static void preinit_state (lua_State *L, global_State *g) {
	G(L) = g;
	L->stack = NULL;
	L->stacksize = 0;
	L->errorJmp = NULL;
	L->hook = NULL;
	L->hookmask = 0;
	L->basehookcount = 0;
	L->allowhook = 1;
	resethookcount(L);
	L->openupval = NULL;
	L->size_ci = 0;
	L->nCcalls = 0;
	L->status = 0;
	L->base_ci = L->ci = NULL;
	L->savedpc = NULL;
	L->errfunc = 0;
	setnilvalue(gt(L));
}


static void close_state (lua_State *L) {
	global_State *g = G(L);
	luaF_close(L, L->stack);  /* close all upvalues for this thread */
	luaC_freeall(L);  /* collect all objects */
	lua_assert(g->rootgc == obj2gco(L));
	lua_assert(g->strt.nuse == 0);
	luaM_freearray(L, G(L)->strt.hash, G(L)->strt.size, TString *);
	luaZ_freebuffer(L, &g->buff);
	freestack(L, L);
	lua_assert(g->totalbytes == sizeof(LG));
	(*g->frealloc)(g->ud, fromstate(L), state_size(LG), 0);
}


lua_State *luaE_newthread (lua_State *L) {
	lua_State *L1 = tostate(luaM_malloc(L, state_size(lua_State)));
	luaC_link(L, obj2gco(L1), LUA_TTHREAD);
	preinit_state(L1, G(L));
	stack_init(L1, L);  /* init stack */
	setobj2n(L, gt(L1), gt(L));  /* share table of globals */
	L1->hookmask = L->hookmask;
	L1->basehookcount = L->basehookcount;
	L1->hook = L->hook;
	resethookcount(L1);
	lua_assert(iswhite(obj2gco(L1)));
	return L1;
}


void luaE_freethread (lua_State *L, lua_State *L1) {
	luaF_close(L1, L1->stack);  /* close all upvalues for this thread */
	lua_assert(L1->openupval == NULL);
	luai_userstatefree(L1);
	freestack(L, L1);
	luaM_freemem(L, fromstate(L1), state_size(lua_State));
}


LUA_API lua_State *lua_newstate (lua_Alloc f, void *ud) {
	int i;
	lua_State *L;
	global_State *g;
	void *l = (*f)(ud, NULL, 0, state_size(LG));
	if (l == NULL) return NULL;
	L = tostate(l);
	g = &((LG *)L)->g;
	L->next = NULL;
	L->tt = LUA_TTHREAD;
	g->currentwhite = bit2mask(WHITE0BIT, FIXEDBIT);
	L->marked = luaC_white(g);
	set2bits(L->marked, FIXEDBIT, SFIXEDBIT);
	preinit_state(L, g);
	g->frealloc = f;
	g->ud = ud;
	g->mainthread = L;
	g->uvhead.u.l.prev = &g->uvhead;
	g->uvhead.u.l.next = &g->uvhead;
	g->GCthreshold = 0;  /* mark it as unfinished state */
	g->strt.size = 0;
	g->strt.nuse = 0;
	g->strt.hash = NULL;
	setnilvalue(registry(L));
	luaZ_initbuffer(L, &g->buff);
	g->panic = NULL;
	g->gcstate = GCSpause;
	g->rootgc = obj2gco(L);
	g->sweepstrgc = 0;
	g->sweepgc = &g->rootgc;
	g->gray = NULL;
	g->grayagain = NULL;
	g->weak = NULL;
	g->tmudata = NULL;
	g->totalbytes = sizeof(LG);
	g->gcpause = LUAI_GCPAUSE;
	g->gcstepmul = LUAI_GCMUL;
	g->gcdept = 0;
	for (i=0; i<NUM_TAGS; i++) g->mt[i] = NULL;
	if (luaD_rawrunprotected(L, f_luaopen, NULL) != 0) {
		/* memory allocation error: free partial state */
		close_state(L);
		L = NULL;
	}
	else
		luai_userstateopen(L);
	return L;
}


static void callallgcTM (lua_State *L, void *ud) {
	UNUSED(ud);
	luaC_callGCTM(L);  /* call GC metamethods for all udata */
}


LUA_API void lua_close (lua_State *L) {
	L = G(L)->mainthread;  /* only the main thread can be closed */
	lua_lock(L);
	luaF_close(L, L->stack);  /* close all upvalues for this thread */
	luaC_separateudata(L, 1);  /* separate udata that have GC metamethods */
	L->errfunc = 0;  /* no error function during GC metamethods */
	do {  /* repeat until no more errors */
		L->ci = L->base_ci;
		L->base = L->top = L->ci->base;
		L->nCcalls = 0;
	} while (luaD_rawrunprotected(L, callallgcTM, NULL) != 0);
	lua_assert(G(L)->tmudata == NULL);
	luai_userstateclose(L);
	close_state(L);
}


//-------------------------------------------------------------lauxlib.c---------------------------------------


/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
	lua_gettop(L) + (i) + 1)


/*
** {======================================================
** Error-report functions
** =======================================================
*/


LUALIB_API int luaL_argerror (lua_State *L, int narg, const char *extramsg) {
	lua_Debug ar;
	if (!lua_getstack(L, 0, &ar))  /* no stack frame? */
		return luaL_error(L, "bad argument #%d (%s)", narg, extramsg);
	lua_getinfo(L, "n", &ar);
	if (strcmp(ar.namewhat, "method") == 0) {
		narg--;  /* do not count `self' */
		if (narg == 0)  /* error is in the self argument itself? */
			return luaL_error(L, "calling " LUA_QS " on bad self (%s)",
			ar.name, extramsg);
	}
	if (ar.name == NULL)
		ar.name = "?";
	return luaL_error(L, "bad argument #%d to " LUA_QS " (%s)",
		narg, ar.name, extramsg);
}


LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname) {
	const char *msg = lua_pushfstring(L, "%s expected, got %s",
		tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}


static void tag_error (lua_State *L, int narg, int tag) {
	luaL_typerror(L, narg, lua_typename(L, tag));
}


LUALIB_API void luaL_where (lua_State *L, int level) {
	lua_Debug ar;
	if (lua_getstack(L, level, &ar)) {  /* check function at level */
		lua_getinfo(L, "Sl", &ar);  /* get info about it */
		if (ar.currentline > 0) {  /* is there info? */
			lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
			return;
		}
	}
	lua_pushliteral(L, "");  /* else, no information available... */
}


LUALIB_API int luaL_error (lua_State *L, const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	luaL_where(L, 1);
	lua_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_concat(L, 2);
	return lua_error(L);
}

/* }====================================================== */


LUALIB_API int luaL_checkoption (lua_State *L, int narg, const char *def,
								 const char *const lst[]) {
									 const char *name = (def) ? luaL_optstring(L, narg, def) :
										 luaL_checkstring(L, narg);
									 int i;
									 for (i=0; lst[i]; i++)
										 if (strcmp(lst[i], name) == 0)
											 return i;
									 return luaL_argerror(L, narg,
										 lua_pushfstring(L, "invalid option " LUA_QS, name));
}


LUALIB_API int luaL_newmetatable (lua_State *L, const char *tname) {
	lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get registry.name */
	if (!lua_isnil(L, -1))  /* name already in use? */
		return 0;  /* leave previous value on top, but return 0 */
	lua_pop(L, 1);
	lua_newtable(L);  /* create metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
	return 1;
}


LUALIB_API void *luaL_checkudata (lua_State *L, int ud, const char *tname) {
	void *p = lua_touserdata(L, ud);
	if (p != NULL) {  /* value is a userdata? */
		if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
				lua_pop(L, 2);  /* remove both metatables */
				return p;
			}
		}
	}
	luaL_typerror(L, ud, tname);  /* else error */
	return NULL;  /* to avoid warnings */
}


LUALIB_API void luaL_checkstack (lua_State *L, int space, const char *mes) {
	if (!lua_checkstack(L, space))
		luaL_error(L, "stack overflow (%s)", mes);
}


LUALIB_API void luaL_checktype (lua_State *L, int narg, int t) {
	if (lua_type(L, narg) != t)
		tag_error(L, narg, t);
}


LUALIB_API void luaL_checkany (lua_State *L, int narg) {
	if (lua_type(L, narg) == LUA_TNONE)
		luaL_argerror(L, narg, "value expected");
}


LUALIB_API const char *luaL_checklstring (lua_State *L, int narg, size_t *len) {
	const char *s = lua_tolstring(L, narg, len);
	if (!s) tag_error(L, narg, LUA_TSTRING);
	return s;
}


LUALIB_API const char *luaL_optlstring (lua_State *L, int narg,
										const char *def, size_t *len) {
											if (lua_isnoneornil(L, narg)) {
												if (len)
													*len = (def ? strlen(def) : 0);
												return def;
											}
											else return luaL_checklstring(L, narg, len);
}


LUALIB_API lua_Number luaL_checknumber (lua_State *L, int narg) {
	lua_Number d = lua_tonumber(L, narg);
	if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
		tag_error(L, narg, LUA_TNUMBER);
	return d;
}


LUALIB_API lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number def) {
	return luaL_opt(L, luaL_checknumber, narg, def);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, int narg) {
	lua_Integer d = lua_tointeger(L, narg);
	if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
		tag_error(L, narg, LUA_TNUMBER);
	return d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, int narg,
										lua_Integer def) {
											return luaL_opt(L, luaL_checkinteger, narg, def);
}


LUALIB_API int luaL_getmetafield (lua_State *L, int obj, const char *event) {
	if (!lua_getmetatable(L, obj))  /* no metatable? */
		return 0;
	lua_pushstring(L, event);
	lua_rawget(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 2);  /* remove metatable and metafield */
		return 0;
	}
	else {
		lua_remove(L, -2);  /* remove only metatable */
		return 1;
	}
}


LUALIB_API int luaL_callmeta (lua_State *L, int obj, const char *event) {
	obj = abs_index(L, obj);
	if (!luaL_getmetafield(L, obj, event))  /* no metafield? */
		return 0;
	lua_pushvalue(L, obj);
	lua_call(L, 1, 1);
	return 1;
}


LUALIB_API void (luaL_register) (lua_State *L, const char *libname,
								 const luaL_Reg *l) {
									 luaI_openlib(L, libname, l, 0);
}


static int libsize (const luaL_Reg *l) {
	int size = 0;
	for (; l->name; l++) size++;
	return size;
}


LUALIB_API void luaI_openlib (lua_State *L, const char *libname,
							  const luaL_Reg *l, int nup) {
								  if (libname) {
									  int size = libsize(l);
									  /* check whether lib already exists */
									  luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", size);
									  lua_getfield(L, -1, libname);  /* get _LOADED[libname] */
									  if (!lua_istable(L, -1)) {  /* not found? */
										  lua_pop(L, 1);  /* remove previous result */
										  /* try global variable (and create one if it does not exist) */
										  if (luaL_findtable(L, LUA_GLOBALSINDEX, libname, size) != NULL)
											  luaL_error(L, "name conflict for module " LUA_QS, libname);
										  lua_pushvalue(L, -1);
										  lua_setfield(L, -3, libname);  /* _LOADED[libname] = new table */
									  }
									  lua_remove(L, -2);  /* remove _LOADED table */
									  lua_insert(L, -(nup+1));  /* move library table to below upvalues */
								  }
								  for (; l->name; l++) {
									  int i;
									  for (i=0; i<nup; i++)  /* copy upvalues to the top */
										  lua_pushvalue(L, -nup);
									  lua_pushcclosure(L, l->func, nup);
									  lua_setfield(L, -(nup+2), l->name);
								  }
								  lua_pop(L, nup);  /* remove upvalues */
}



/*
** {======================================================
** getn-setn: size for arrays
** =======================================================
*/

#if defined(LUA_COMPAT_GETN)

static int checkint (lua_State *L, int topop) {
	int n = (lua_type(L, -1) == LUA_TNUMBER) ? lua_tointeger(L, -1) : -1;
	lua_pop(L, topop);
	return n;
}


static void getsizes (lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "LUA_SIZES");
	if (lua_isnil(L, -1)) {  /* no `size' table? */
		lua_pop(L, 1);  /* remove nil */
		lua_newtable(L);  /* create it */
		lua_pushvalue(L, -1);  /* `size' will be its own metatable */
		lua_setmetatable(L, -2);
		lua_pushliteral(L, "kv");
		lua_setfield(L, -2, "__mode");  /* metatable(N).__mode = "kv" */
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, "LUA_SIZES");  /* store in register */
	}
}


LUALIB_API void luaL_setn (lua_State *L, int t, int n) {
	t = abs_index(L, t);
	lua_pushliteral(L, "n");
	lua_rawget(L, t);
	if (checkint(L, 1) >= 0) {  /* is there a numeric field `n'? */
		lua_pushliteral(L, "n");  /* use it */
		lua_pushinteger(L, n);
		lua_rawset(L, t);
	}
	else {  /* use `sizes' */
		getsizes(L);
		lua_pushvalue(L, t);
		lua_pushinteger(L, n);
		lua_rawset(L, -3);  /* sizes[t] = n */
		lua_pop(L, 1);  /* remove `sizes' */
	}
}


LUALIB_API int luaL_getn (lua_State *L, int t) {
	int n;
	t = abs_index(L, t);
	lua_pushliteral(L, "n");  /* try t.n */
	lua_rawget(L, t);
	if ((n = checkint(L, 1)) >= 0) return n;
	getsizes(L);  /* else try sizes[t] */
	lua_pushvalue(L, t);
	lua_rawget(L, -2);
	if ((n = checkint(L, 2)) >= 0) return n;
	return (int)lua_objlen(L, t);
}

#endif

/* }====================================================== */



LUALIB_API const char *luaL_gsub (lua_State *L, const char *s, const char *p,
								  const char *r) {
									  const char *wild;
									  size_t l = strlen(p);
									  luaL_Buffer b;
									  luaL_buffinit(L, &b);
									  while ((wild = strstr(s, p)) != NULL) {
										  luaL_addlstring(&b, s, wild - s);  /* push prefix */
										  luaL_addstring(&b, r);  /* push replacement in place of pattern */
										  s = wild + l;  /* continue after `p' */
									  }
									  luaL_addstring(&b, s);  /* push last suffix */
									  luaL_pushresult(&b);
									  return lua_tostring(L, -1);
}


LUALIB_API const char *luaL_findtable (lua_State *L, int idx,
									   const char *fname, int szhint) {
										   const char *e;
										   lua_pushvalue(L, idx);
										   do {
											   e = strchr(fname, '.');
											   if (e == NULL) e = fname + strlen(fname);
											   lua_pushlstring(L, fname, e - fname);
											   lua_rawget(L, -2);
											   if (lua_isnil(L, -1)) {  /* no such field? */
												   lua_pop(L, 1);  /* remove this nil */
												   lua_createtable(L, 0, (*e == '.' ? 1 : szhint)); /* new table for field */
												   lua_pushlstring(L, fname, e - fname);
												   lua_pushvalue(L, -2);
												   lua_settable(L, -4);  /* set new table into field */
											   }
											   else if (!lua_istable(L, -1)) {  /* field has a non-table value? */
												   lua_pop(L, 2);  /* remove table and value */
												   return fname;  /* return problematic part of the name */
											   }
											   lua_remove(L, -2);  /* remove previous table */
											   fname = e + 1;
										   } while (*e == '.');
										   return NULL;
}



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/


#define bufflen(B)	((B)->p - (B)->buffer)
#define bufffree(B)	((size_t)(LUAL_BUFFERSIZE - bufflen(B)))

#define LIMIT	(LUA_MINSTACK/2)


static int emptybuffer (luaL_Buffer *B) {
	size_t l = bufflen(B);
	if (l == 0) return 0;  /* put nothing on stack */
	else {
		lua_pushlstring(B->L, B->buffer, l);
		B->p = B->buffer;
		B->lvl++;
		return 1;
	}
}


static void adjuststack (luaL_Buffer *B) {
	if (B->lvl > 1) {
		lua_State *L = B->L;
		int toget = 1;  /* number of levels to concat */
		size_t toplen = lua_strlen(L, -1);
		do {
			size_t l = lua_strlen(L, -(toget+1));
			if (B->lvl - toget + 1 >= LIMIT || toplen > l) {
				toplen += l;
				toget++;
			}
			else break;
		} while (toget < B->lvl);
		lua_concat(L, toget);
		B->lvl = B->lvl - toget + 1;
	}
}


LUALIB_API char *luaL_prepbuffer (luaL_Buffer *B) {
	if (emptybuffer(B))
		adjuststack(B);
	return B->buffer;
}


LUALIB_API void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) {
	while (l--)
		luaL_addchar(B, *s++);
}


LUALIB_API void luaL_addstring (luaL_Buffer *B, const char *s) {
	luaL_addlstring(B, s, strlen(s));
}


LUALIB_API void luaL_pushresult (luaL_Buffer *B) {
	emptybuffer(B);
	lua_concat(B->L, B->lvl);
	B->lvl = 1;
}


LUALIB_API void luaL_addvalue (luaL_Buffer *B) {
	lua_State *L = B->L;
	size_t vl;
	const char *s = lua_tolstring(L, -1, &vl);
	if (vl <= bufffree(B)) {  /* fit into buffer? */
		memcpy(B->p, s, vl);  /* put it there */
		B->p += vl;
		lua_pop(L, 1);  /* remove from stack */
	}
	else {
		if (emptybuffer(B))
			lua_insert(L, -2);  /* put buffer before new value */
		B->lvl++;  /* add new value into B stack */
		adjuststack(B);
	}
}


LUALIB_API void luaL_buffinit (lua_State *L, luaL_Buffer *B) {
	B->L = L;
	B->p = B->buffer;
	B->lvl = 0;
}

/* }====================================================== */


LUALIB_API int luaL_ref (lua_State *L, int t) {
	int ref;
	t = abs_index(L, t);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);  /* remove from stack */
		return LUA_REFNIL;  /* `nil' has a unique fixed reference */
	}
	lua_rawgeti(L, t, FREELIST_REF);  /* get first free element */
	ref = (int)lua_tointeger(L, -1);  /* ref = t[FREELIST_REF] */
	lua_pop(L, 1);  /* remove it from stack */
	if (ref != 0) {  /* any free element? */
		lua_rawgeti(L, t, ref);  /* remove it from list */
		lua_rawseti(L, t, FREELIST_REF);  /* (t[FREELIST_REF] = t[ref]) */
	}
	else {  /* no free elements */
		ref = (int)lua_objlen(L, t);
		ref++;  /* create new reference */
	}
	lua_rawseti(L, t, ref);
	return ref;
}


LUALIB_API void luaL_unref (lua_State *L, int t, int ref) {
	if (ref >= 0) {
		t = abs_index(L, t);
		lua_rawgeti(L, t, FREELIST_REF);
		lua_rawseti(L, t, ref);  /* t[ref] = t[FREELIST_REF] */
		lua_pushinteger(L, ref);
		lua_rawseti(L, t, FREELIST_REF);  /* t[FREELIST_REF] = ref */
	}
}



/*
** {======================================================
** Load functions
** =======================================================
*/

typedef struct LoadF {
	int extraline;
	FILE *f;
	char buff[LUAL_BUFFERSIZE];
} LoadF;


static const char *getF (lua_State *L, void *ud, size_t *size) {
	LoadF *lf = (LoadF *)ud;
	(void)L;
	if (lf->extraline) {
		lf->extraline = 0;
		*size = 1;
		return "\n";
	}
	if (feof(lf->f)) return NULL;
	*size = fread(lf->buff, 1, LUAL_BUFFERSIZE, lf->f);
	return (*size > 0) ? lf->buff : NULL;
}


static int errfile (lua_State *L, const char *what, int fnameindex) {
	const char *serr = strerror(errno);
	const char *filename = lua_tostring(L, fnameindex) + 1;
	lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
	lua_remove(L, fnameindex);
	return LUA_ERRFILE;
}


LUALIB_API int luaL_loadfile (lua_State *L, const char *filename) {
	LoadF lf;
	int status, readstatus;
	int c;
	int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
	lf.extraline = 0;
	if (filename == NULL) {
		lua_pushliteral(L, "=stdin");
		lf.f = stdin;
	}
	else {
		lua_pushfstring(L, "@%s", filename);
		lf.f = fopen(filename, "r");
		if (lf.f == NULL) return errfile(L, "open", fnameindex);
	}
	c = getc(lf.f);
	if (c == '#') {  /* Unix exec. file? */
		lf.extraline = 1;
		while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
		if (c == '\n') c = getc(lf.f);
	}
	if (c == LUA_SIGNATURE[0] && lf.f != stdin) {  /* binary file? */
		fclose(lf.f);
		lf.f = fopen(filename, "rb");  /* reopen in binary mode */
		if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
		/* skip eventual `#!...' */
		while ((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
		lf.extraline = 0;
	}
	ungetc(c, lf.f);
	status = lua_load(L, getF, &lf, lua_tostring(L, -1));
	readstatus = ferror(lf.f);
	if (lf.f != stdin) fclose(lf.f);  /* close file (even in case of errors) */
	if (readstatus) {
		lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
		return errfile(L, "read", fnameindex);
	}
	lua_remove(L, fnameindex);
	return status;
}


typedef struct LoadS {
	const char *s;
	size_t size;
} LoadS;


static const char *getS (lua_State *L, void *ud, size_t *size) {
	LoadS *ls = (LoadS *)ud;
	(void)L;
	if (ls->size == 0) return NULL;
	*size = ls->size;
	ls->size = 0;
	return ls->s;
}


LUALIB_API int luaL_loadbuffer (lua_State *L, const char *buff, size_t size,
								const char *name) {
									LoadS ls;
									ls.s = buff;
									ls.size = size;
									return lua_load(L, getS, &ls, name);
}


LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s) {
	return luaL_loadbuffer(L, s, strlen(s), s);
}



/* }====================================================== */


static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
	(void)ud;
	(void)osize;
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}


static int panic (lua_State *L) {
	(void)L;  /* to avoid warnings */
	fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
		lua_tostring(L, -1));
	return 0;
}


LUALIB_API lua_State *luaL_newstate (void) {
	lua_State *L = lua_newstate(l_alloc, NULL);
	if (L) lua_atpanic(L, &panic);
	return L;
}







#include "Script.h"


using namespace Framework::Scripting;

Script::Script() : m_lua_state(NULL), m_filename("UN-INITIALIZED") {}
Script::~Script()
{
	CloseScript();
	unsigned numErrs = (unsigned)m_errBuffs.size();
	for (unsigned i = 0; i < numErrs; ++i)
		delete m_errBuffs[i];
}
//////////////////////////////////////////////////////////////////////////

void Script::CloseScript()
{
	if (m_lua_state)
	{
		lua_close(m_lua_state);
		m_lua_state = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////


//! This function makes sure we are not using stuff off the LUA stack or our other buffers
const char* Script::CacheError(std::string err, int pop)
{
	std::string* errStr = new std::string(err);
	m_errBuffs.push_back(errStr);
	if (pop)
	{
		ASSERT(m_lua_state);
		lua_pop(m_lua_state, pop);
	}
	return errStr->c_str();
}
//////////////////////////////////////////////////////////////////////////



//! Returns an error string if there was a problem
//! ASSERTS that the script is not already valid
const char* Script::LoadScript(const char* fileName_or_buff, bool file)
{
	ASSERT(!IsValid());
	ASSERT(fileName_or_buff && fileName_or_buff[0]);

	m_filename = file ? fileName_or_buff : "BUFFER";

	const char* ret = NULL;
	m_lua_state = lua_open();
	//Skip this for now TODO see if it is needed
	//luaL_openlibs(m_lua_state);
	//I shouldn't need this for wind river
	//Register_C_Function::RegisterAllFunctions(m_lua_state);

	// Load the script file and execute the globals
	int errLoading = file ? luaL_loadfile(m_lua_state, fileName_or_buff) : 
		luaL_loadbuffer(m_lua_state, fileName_or_buff, strlen(fileName_or_buff), "buffer");
	if (errLoading != 0)
		ret = CacheError(lua_tostring(m_lua_state, -1), 1);
	else if (lua_pcall(m_lua_state, 0, 0, 0) != NULL)
		ret = CacheError(lua_tostring(m_lua_state, -1), 1);

	// If Something did not go well, shut it all down
	if (ret)
		CloseScript();
		
	return ret;
}


const char* Script::GetStackValue(std::string* t_string, bool* t_boolean, double* t_number)
{
	ASSERT(t_string || t_boolean || t_number);

	// These might hold the temporary values until we know we can return true.
	std::string l_string;
	bool l_boolean;
	double l_number;

	if (t_string)
	{
		if (lua_isstring(m_lua_state, -1))
			l_string = lua_tostring(m_lua_state, -1);
		else return "The value on the stack does not cast to a string";
	}

	if (t_boolean)
	{
		if (lua_isboolean(m_lua_state, -1))
			l_boolean = (lua_toboolean(m_lua_state, -1) != 0);
		else return "The value on the stack does not cast to a bool(int)";
	}

	if (t_number)
	{
		if (lua_isnumber(m_lua_state, -1))
			l_number = lua_tonumber(m_lua_state, -1);
		else return "The value on the stack does not cast to a number(double)";
	}

	// So far we know that we have gotten what we asked for
	if (t_string)
		*t_string = l_string;
	if (t_boolean)
		*t_boolean = l_boolean;
	if (t_number)
		*t_number = l_number;

	return NULL;
}

const char* Script::GetGlobal(std::string globalName,std::string* t_string, bool* t_boolean, double* t_number)
{
	ASSERT(IsValid());
	std::string mappedName = GetMappedName(globalName);
	lua_getglobal(m_lua_state, mappedName.c_str());
	const char* err = GetStackValue(t_string, t_boolean, t_number);
	if (err)
	{
		lua_getglobal(m_lua_state, globalName.c_str());
		err = GetStackValue(t_string, t_boolean, t_number);
	}
	
	if (err)
	{
		std::string errMsg(err);
		return CacheError(Framework::Base::BuildString("LUA Script[%s] failed to read %s, {%s}\n", 
			GetFilename(), mappedName.c_str(), errMsg.c_str()), false);
	}
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////////

const char* Script::GetGlobalTable(std::string globalTableName)
{
	ASSERT(IsValid());
	lua_getglobal(m_lua_state, GetMappedName(globalTableName).c_str());
	if (!lua_istable(m_lua_state, -1))
	{
		return CacheError(Framework::Base::BuildString(
			"LUA Script[%s] could Not load Table from the Global %s\n", 
			GetFilename(), globalTableName.c_str()), false);
	}
	else return NULL;	// All is ok
}
//////////////////////////////////////////////////////////////////////////

const char* Script::GetFieldTable(std::string fieldTableName)
{
	ASSERT(IsValid());
	ASSERT(lua_istable(m_lua_state, -1));	// We should already have a stack
	lua_pushstring(m_lua_state, GetMappedName(fieldTableName).c_str());
	lua_gettable(m_lua_state, -2);
	if (!lua_istable(m_lua_state, -1))
	{
		// This is not a good table name, Pop off the stack now
		lua_pop(m_lua_state, 1);  // Pop the name back off the stack
		return CacheError(Framework::Base::BuildString(
			"LUA Script[%s] could Not load Table from the field name %s\n", 
			GetFilename(), GetMappedName(fieldTableName).c_str()), false);
	}
	else
		return NULL;
}
//////////////////////////////////////////////////////////////////////////
const char* Script::GetField(std::string fieldName,std::string* t_string, bool* t_boolean, double* t_number)
{
	ASSERT(IsValid());
	ASSERT(lua_istable(m_lua_state, -1));	// We should already have a stack
	
	lua_pushstring(m_lua_state, GetMappedName(fieldName).c_str());
	lua_gettable(m_lua_state, -2);
	const char* err = GetStackValue(t_string, t_boolean, t_number);
	lua_pop(m_lua_state, 1);  // Pop the name back off the stack
	
	if (err)
	{
		std::string errMsg(err);
		return CacheError(Framework::Base::BuildString("LUA Script[%s] failed to read field name %s, {%s}\n", 
			GetFilename(), GetMappedName(fieldName).c_str(), errMsg.c_str()), false);
	}
	else
		return NULL;
}
//////////////////////////////////////////////////////////////////////////

void Script::Pop()
{
	lua_pop(m_lua_state, 1);
}
//////////////////////////////////////////////////////////////////////////

const char* Script::GetIndex(unsigned index, 
		std::string* t_string, bool* t_boolean, double* t_number)
{
	ASSERT(IsValid());
	ASSERT(lua_istable(m_lua_state, -1));	// We should already have a stack
	lua_rawgeti(m_lua_state, -1, index);
	const char* err = GetStackValue(t_string, t_boolean, t_number);
	lua_pop(m_lua_state, 1);  // Pop the name back off the stack
	
	if (err)
	{
		std::string errMsg(err);
		return CacheError(Framework::Base::BuildString("LUA Script[%s] failed to read field index %i, {%s}\n", 
			GetFilename(), index, errMsg.c_str()), false);
	}
	else
		return NULL;
}
//////////////////////////////////////////////////////////////////////////

const char* Script::GetIndexTable(unsigned index)
{
	ASSERT(IsValid());
	ASSERT(lua_istable(m_lua_state, -1));	// We should already have a stack
	lua_rawgeti(m_lua_state, -1, index);
	if (!lua_istable(m_lua_state, -1))
		return CacheError(Framework::Base::BuildString(
		"LUA Script[%s] could Not load Table from the index %i", GetFilename(), index), false);
	else
		return NULL;
}
//////////////////////////////////////////////////////////////////////////

//! e.g. CallFunction("f", "dd>d", x, y, &z);
//! Note that this was copied from the examples, and I do not necessarily like the style,
//! but it works and should get us going, and it is quite robust.
//! For params: d=double, i=int, s=char*
//! For returns: d=double*, i=int*, s=char**, t=(Event1<Script*>*
const char* Script::CallFunction(const char *func, const char *sig, ...) 
{
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(m_lua_state, GetMappedName(func).c_str());  /* get function */

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {

		  case 'd':  /* double argument */
			  lua_pushnumber(m_lua_state, va_arg(vl, double));
			  break;

		  case 'i':  /* int argument */
			  lua_pushnumber(m_lua_state, va_arg(vl, int));
			  break;

		  case 's':  /* string argument */
			  lua_pushstring(m_lua_state, va_arg(vl, char *));
			  break;

		  case '>':
			  goto endwhile;

		  default:
			  va_end(vl);
			  return CacheError(Framework::Base::BuildString("invalid option (%c)\n", *(sig - 1)), narg);
		}
		narg++;
		luaL_checkstack(m_lua_state, 1, "too many arguments");
	} endwhile:

	/* do the call */
	nres = (int)strlen(sig);  /* number of expected results */
	if (lua_pcall(m_lua_state, narg, nres, 0) != 0)  /* do the call */
	{
		va_end(vl);
		return CacheError(Framework::Base::BuildString("error running function `%s': %s\n",
			func, lua_tostring(m_lua_state, -1)), false);
	}

	/* retrieve results */
	int resIndex = -nres;  /* stack index of first result */
	while (*sig) {  /* get results */
		switch (*sig++) {

		  case 'd':  /* double result */
			  if (!lua_isnumber(m_lua_state, resIndex))
			  {
				  va_end(vl);
				  return CacheError(Framework::Base::BuildString("Invalid return type for %s at pos %i, expected number\n",
					  func, -resIndex), nres);
			  }
			  *va_arg(vl, double *) = lua_tonumber(m_lua_state, resIndex);
			  break;

		  case 'i':  /* int result */
			  if (!lua_isnumber(m_lua_state, resIndex))
			  {
				  va_end(vl);
				  return CacheError(Framework::Base::BuildString("Invalid return type for %s at pos %i, expected int\n",
					  func, -resIndex), nres);
			  }
			  *va_arg(vl, int *) = (int)lua_tonumber(m_lua_state, resIndex);
			  break;

		  case 'b':  /* bool result */
			  if (!lua_isboolean(m_lua_state, resIndex))
			  {
				  va_end(vl);
				  return CacheError(Framework::Base::BuildString("Invalid return type for %s at pos %i, expected bool(int)\n",
					  func, -resIndex), nres);
			  }
			  *va_arg(vl, bool *) = (int)lua_toboolean(m_lua_state, resIndex) != 0;
			  break;

		  case 's':  /* string result */
			  if (!lua_isstring(m_lua_state, resIndex))
			  {
				  va_end(vl);
				  return CacheError(Framework::Base::BuildString("Invalid return type for %s at pos %i, expected string\n",
					  func, -resIndex), nres);
			  }
			  *va_arg(vl, const char **) = lua_tostring(m_lua_state, resIndex);
			  break;

		  case 't':	/* Table result */
			  if (!lua_istable(m_lua_state, resIndex))
			  {
				  va_end(vl);
				  return CacheError(Framework::Base::BuildString("Invalid return type for %s at pos %i, expected table\n",
					  func, -resIndex), nres);
			  }
			  // Push this table to the TOP of the stack
			  lua_pushvalue(m_lua_state, resIndex);

				//TODO see if I need this

			  // OOH! Ugly void* cast, but it was the only way I could work out how to do it.
			  //((Event1<Script*>*)(va_arg(vl, void*)))->Fire(this);

			  // Pop the table back off to get the stack back the way it was
				lua_pop(m_lua_state, 1);
			  break;

		  default:
			  va_end(vl);
			  return CacheError(Framework::Base::BuildString("invalid option (%c)\n", *(sig - 1)), nres);
		}
		resIndex++;
	}
	va_end(vl);

	// Pop the results
	lua_pop(m_lua_state, nres);
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
#endif

