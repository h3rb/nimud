#!/usr/bin/perl
#########################################################################
# mudcheck.pl				Version 1.0			#
# Created: Jun 28, 1997			Jared Proudfoot			#
# Last Modified: Jun 28, 1997		jproudfo@footprints.net		#
#########################################################################
# README								#
#									#
# This is a pretty simple script that checks to see if a mud is running	#
# properly.  Be sure to change the 'important vars' section.  The best	#
# idea is to run this script from cron every couple of minutes... That	#
# way you can be sure that it stays running.				#
#########################################################################
# CHANGES AND HISTORY							#
#									#
# v 1.0, Jun 28, 1997	Created.  Seems to work fine now.		#
#########################################################################
#########################################################################
# Define the important vars						#
#									#
# Define the host and port number where the mud resides.		#

$server		=	"nimud.divineright.org";
$port		=	"5333";

# $string is the string of characters we will look for upon connecting.	#
# If we connect, but don't see this string, we will assume the mud	#
# isn't responding (locked up?) and we'll restart it.  The string	#
# *must* be on the first line after connect.	

# You may enter this as a regexp if you wish.				#

$replyString		=	"^\n";

# How long do we wait before we connect timeout (in seconds)?		#

$timeOut	=	"60";

# What to execute if we need to restart the mud.  Please include the	#
# FULL path.								#

$exec		=	"/home/locke/NiM5/area/nimud";

# Path where you want the mud logs to be kept.

$logdir		=	"/home/locke/NiM5/log";

# Path where we should start the mud from.				#
$startPath	=	"/home/locke/NiM5/area";

# That's it.  You shouldn't need to change anything after this line.	#
#########################################################################

# What do we need to use?
use Socket;
require 5.003;



#########################################################################
# Main									#
#########################################################################

if (&connect_server == 0) {
	# If we couldn't connect, try and restart.			#
	print ("Connection to $server on port $port failed or timed out after $timeOut seconds!\n");
	$time = (scalar localtime);
	print ("Attempting to restart the mud on $time...\n");
	# Restart the mud						#
	&restart_mud;
}
else {
	# We connected, but is it working properly?			#
		$readline = (&gl);
		if ($readline =~ /$replyString/) {
			# We found what we were looking for, so exit 	#
			# properly.					#
			&disconnect_server;
			exit 1;
		}
	# After all those searches, we didn't find anything.  The mud	#
	# must be locked up.  Lets kill and restart it.			#
	&disconnect_server;
	print ("The connection was successful, but it doesn't seem to be responding\n");
	$time = (scalar localtime);
	print ("Attempting to restart the mud on $time...\n");
	system("killall $exec");
	&restart_mud;
}
	
	


#########################################################################
# Subroutines								#
#########################################################################


sub connect_server {
	# Connect to the server						#
	my ($iaddr, $paddr, $proto);
	$iaddr = inet_aton ($server)
		or die ("ERROR: No host: $server!\n");
	$paddr = sockaddr_in ($port, $iaddr);
	$proto = getprotobyname('tcp');
	socket (SOCK, PF_INET, SOCK_STREAM, $proto)
		or die ("ERROR: Socket error $!\n");
	alarm ($timeOut);
	if (connect (SOCK, $paddr)) {;
		alarm (0);
		return 1;
	}
	else {
		return 0;
	}
}

sub disconnect_server {
	# Disconnect from the server					#
	close (SOCK);
	return;
}

sub sl {
	# Send a line							#
  	my ($line)=@_;
  	print SOCK ("$line")
		or die ("ERROR: Error writing to server: $!\n");
	select SOCK;
  	$|=1;
  	select STDOUT;
  	$|=1;
	return;
}
	
sub gl {
	# Get a line							#
	my ($buffer, @reply);
	$buffer=(<SOCK>);
#	(@reply) = split (/\s/, $buffer);
#	return (@reply);
	return ($buffer);
}

sub restart_mud {
		# Restart the mud					#
		$timet = time();
		chdir $startPath;
#		system ("$exec $port > $logdir/$timet.log 2>&1 &");
                system ("nohup /home/locke/NiM5/startup 5333 &");
		return;
}
