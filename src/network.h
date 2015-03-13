/******************************************************************************
 * Locke's   __                      __         NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5     Version 5 (ALPHA)             *
 * |  /   \  __|  \__/  |  | |  |      |        unreleased+revamped 2004      *
 * |       ||  |        |  \_|  | ()   |                                      *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************/

/*
 * OS-dependent declarations.
 */





/*
 * IBM's AIX
 */
#if defined(_AIX)
#include <sys/select.h>
/*
int  accept       args( ( int s, struct sockaddr *addr, int *addrlen ) );
int  bind         args( ( int s, struct sockaddr *name, int namelen ) );
*/
void bzero        args( ( char *b, int length ) );
int  getpeername  args( ( int s, struct sockaddr *name, int *namelen ) );
int  getsockname  args( ( int s, struct sockaddr *name, int *namelen ) );
int  gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
int  listen       args( ( int s, int backlog ) );
int  setsockopt   args( ( int s, int level, int optname, void *optval,
                          int optlen ) );
int  socket       args( ( int domain, int type, int protocol ) );
#endif

/*
 * Apollo workstations.
 */
#if	defined(apollo)
#include <unistd.h>
void bzero        args( ( char *b, int length ) );
#endif

/*
 * HP-UX machines.
 */
#if	defined(__hpux)
/*
int  accept       args( ( int s, void *addr, int *addrlen ) );
int  bind         args( ( int s, const void *addr, int addrlen ) );
*/
void bzero        args( ( char *b, int length ) );
int  getpeername  args( ( int s, void *addr, int *addrlen ) );
int  getsockname  args( ( int s, void *name, int *addrlen ) );
int  gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
int  listen       args( ( int s, int backlog ) );
int  setsockopt   args( ( int s, int level, int optname, const void *optval,
                          int optlen ) );
int  socket       args( ( int domain, int type, int protocol ) );
#endif

/*
 * Interactive option.
 */
#if	defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif


/*
 * Linux users.
 */
#if	defined(linux)
/*
int  accept       args( ( int s, struct sockaddr *addr, int *addrlen ) );
int  bind         args( ( int s, struct sockaddr *name, int namelen ) );
*/

int  close        args( ( int fd ) );

/*
int  getpeername  args( ( int s, struct sockaddr *name, int *namelen ) );
int  getsockname  args( ( int s, struct sockaddr *name, int *namelen ) );
*/

int  gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );

/*
int  listen       args( ( int s, int backlog ) );
*/

//int  read         args( ( int fd, char *buf, int nbyte ) );
int  select       args( ( int width, fd_set *readfds, fd_set *writefds,
                          fd_set *exceptfds, struct timeval *timeout ) );
int  socket       args( ( int domain, int type, int protocol ) );
//int  write        args( ( int fd, char *buf, int nbyte ) );
#endif


/*
 * Mac Users.
 */
#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static  long            theKeys [4];
int  gettimeofday        args( ( struct timeval *tp, void *tzp ) );
#endif

/*
 * M.I.P.S. OS
 */
#if	defined(MIPS_OS)
extern	int		errno;
#endif

/*
 * Intels.
 */
#if defined(MSDOS)
int  gettimeofday args( ( struct timeval *tp, void *tzp ) );
int  kbhit        args( ( void ) );
#endif

/*
 * NeXT machines.
 */
#if	defined(NeXT)
int  close        args( ( int fd ) );
int  fcntl        args( ( int fd, int cmd, int arg ) );
#if  !defined(htons)
u_short htons     args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long  ntohl     args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * Sequent machines.
 */
#if	defined(sequent)
/*
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
*/
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif


/*
 * SunOS or Solaris SYSV
 */
#if defined(sun)
#undef SO_DONTLINGER
/* Commented out these defines to compile on Solaris 8
  int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
  int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
  int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
*/
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int	setsockopt	args( ( int s, int level, int optname, 
                            const char *optval, int optlen ) );
#else 
/*int     setsockopt      args( ( int s, int level, int optname, void *optval,
                            int optlen ) );*/
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif


/*
 * Ultrix machines.
 */
#if defined(ultrix)
/*
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
 */
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif


