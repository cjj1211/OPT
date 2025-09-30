en

	} elseif {$reusing} {
	    # Cf tests above - both are ready.
	    #Log re-use nonpipeline, GRANT r/w access to $token in geturl
	    set socketRdState($state(socketinfo)) $token
	    set socketWrState($state(socketinfo)) $token
	}

	# All (!$reusing) cases come here, and also some $reusing cases if the
	# connection is ready.
	#Log ---- $state(socketinfo) << conn to $token for HTTP request (a)
	# Connect does its own fconfigure.
	fileevent $sock writable \
		[list http::Connect $token $proto $phost $srvurl]
    }

    # Wait for the connection to complete.
    if {![info exists state(-command)]} {
	# geturl does EVERYTHING asynchronously, so if the user
	# calls it synchronously, we just do a wait here.
	http::wait $token

	if {![info exists state]} {
	    # If we timed out then Finish has been called and the users
	    # command callback may have cleaned up the token. If so we end up
	    # here with nothing left to do.
	    return $token
	} elseif {$state(status) eq "error"} {
	    # Something went wrong while trying to establish the connection.
	    # Clean up after events and such, but DON'T call the command
	    # callback (if available) because we're going to throw an
	    # exception from here instead.
	    set err [lindex $state(error) 0]
	    cleanup $token
	    return -code error $err
	}
    }
    ##Log Leaving http::geturl - token $token
    return $token
}

# http::Connected --
#
#	Callback used when the connection to the HTTP server is actually
#	established.
#
# Arguments:
#	token	State token.
#	proto	What protocol (http, https, etc.) was used to connect.
#	phost	Are we using keep-alive? Non-empty if yes.
#	srvurl	Service-local URL that we're requesting
# Results:
#	None.

proc http::Connected {token proto phost srvurl} {
    variable http
    variable urlTypes
    variable socketMapping
    variable socketRdState
    variable socketWrState
    variable socketRdQueue
    variable socketWrQueue
    variable socketClosing
    variable socketPlayCmd

    variable $token
    upvar 0 $token state
    set tk [namespace tail $token]

    if {$state(reusing) && (!$state(-pipeline)) && ($state(-timeout) > 0)} {
	set state(after) [after $state(-timeout) \
		[list http::reset $token timeout]]
    }

    # Set back the variables needed here.
    set sock $state(sock)
    set isQueryChannel [info exists state(-querychannel)]
    set isQuery [info exists state(-query)]
    set host [lindex [split $state(socketinfo) :] 0]
    set port [lindex [split $state(socketinfo) :] 1]

    set lower [string tolower $proto]
    set defport [lindex $urlTypes($lower) 0]

    # Send data in cr-lf format, but accept any line terminators.
    # Initialisation to {auto *} now done in geturl, KeepSocket and DoneRequest.
    # We are concerned here with the request (write) not the response (read).
    lassign [fconfigure $sock -translation] trRead trWrite
    fconfigure $sock -translation [list $trRead crlf] \
		     -buffersize $state(-blocksize)

    # The following is disallowed in safe interpreters, but the socket is
    # already in non-blocking mode in that case.

    catch {fconfigure $sock -blocking off}
    set how GET
    if {$isQuery} {
	set state(querylength) [string length $state(-query)]
	if {$state(querylength) > 0} {
	    set how POST
	    set contDone 0
	} else {
	    # There's no query data.
	    unset state(-query)
	    set isQuery 0
	}
    } elseif {$state(-validate)} {
	set how HEAD
    } elseif {$isQueryChannel} {
	set how POST
	# The query channel must be blocking for the async Write to
	# work properly.
	fconfigure $state(-querychannel) -blocking 1 -translation binary
	set contDone 0
    }
    if {[info exists state(-method)] && ($state(-method) ne "")} {
	set how $state(-method)
    }
    set accept_types_seen 0

    Log ^B$tk begin sending request - token $token

    if {[catch {
	set state(method) $how
	puts $sock "$how $srvurl HTTP/$state(-protocol)"
	if {[dict exists $state(-headers) Host]} {
	    # Allow Host spoofing. [Bug 928154]
	    puts $sock "Host: [dict get $state(-headers) Host]"
	} elseif {$port == $defport} {
	    # Don't add port in this case, to handle broken servers. [Bug
	    # #504508]
	    puts $sock "Host: $host"
	} else {
	    puts $sock "Host: $host:$port"
	}
	puts $sock "User-Agent: $http(-useragent)"
	if {($state(-protocol) > 1.0) && $state(-keepalive)} {
	    # Send this header, because a 1.1 server is not compelled to treat
	    # this as the default.
	    puts $sock "Connection: keep-alive"
	}
	if {($state(-protocol) > 1.0) && !$state(-keepalive)} {
	    puts $sock "Connection: close" ;# RFC2616 sec 8.1.2.1
	}
	if {($state(-protocol) < 1.1)} {
	    # RFC7230 A.1
	    # Some server implementations of HTTP/1.0 have a faulty
	    # implementation of RFC 2068 Keep-Alive.
	    # Don't leave this to chance.
	    # For HTTP/1.0 we have already "set state(connection) close"
	    # and "state(-keepalive) 0".
	    puts $sock "Connection: close"
	}
	# RFC7230 A.1 - "clients are encouraged not to send the
	# Proxy-Connection header field in any requests"
	set accept_encoding_seen 0
	set content_type_seen 0
	dict for {key value} $state(-headers) {
	    set value [string map [list \n "" \r ""] $value]
	    set key [string map {" " -} [string trim $key]]
	    if {[string equal -nocase $key "host"]} {
		continue
	    }
	    if {[string equal -nocase $key "accept-encoding"]} {
		set accept_encoding_seen 1
	    }
	    if {[string equal -nocase $key "accept"]} {
		set accept_types_seen 1
	    }
	    if {[string equal -nocase $key "content-type"]} {
		set content_type_seen 1
	    }
	    if {[string equal -nocase $key "content-length"]} {
		set contDone 1
		set state(querylength) $value
	    }
	    if {[string length $key]} {
		puts $sock "$key: $value"
	    }
	}
	# Allow overriding the Accept header on a per-connection basis. Useful
	# for working with REST services. [Bug c11a51c482]
	if {!$accept_types_seen} {
	    puts $sock "Accept: $state(accept-types)"
	}
	if {    (!$accept_encoding_seen)
	     && (![info exists state(-handler)])
	     && $http(-zip)
	} {
	    puts $sock "Accept-Encoding: gzip,deflate,compress"
	}
	if {$isQueryChannel && ($state(querylength) == 0)} {
	    # Try to determine size of data in channel. If we cannot seek, the
	    # surrounding catch will trap us

	    set start [tell $state(-querychannel)]
	    seek $state(-querychannel) 0 end
	    set state(querylength) \
		    [expr {[tell $state(-querychannel)] - $start}]
	    seek $state(-querychannel) $start
	}

	# Flush the request header and set up the fileevent that will either
	# push the POST data or read the response.
	#
	# fileevent note:
	#
	# It is possible to have both the read and write fileevents active at
	# this point. The only scenario it seems to affect is a server that
	# closes the connection without reading the POST data. (e.g., early
	# versions TclHttpd in various error cases). Depending on the
	# platform, the client may or may not be able to get the response from
	# the server because of the error it will get trying to write the post
	# data. Having both fileevents active changes the timing and the
	# behavior, but no two platforms (among Solaris, Linux, and NT) behave
	# the same, and none behave all that well in any case. Servers should
	# always read their POST data if they expect the client to read their
	# response.

	if {$isQuery || $isQueryChannel} {
	    # POST method.
	    if {!$content_type_seen} {
		puts $sock "Content-Type: $state(-type)"
	    }
	    if {!$contDone} {
		puts $sock "Content-Length: $state(querylength)"
	    }
	    puts $sock ""
	    flush $sock
	    # Flush flushes the error in the https case with a bad handshake:
	    # else the socket never becomes writable again, and hangs until
	    # timeout (if any).

	    lassign [fconfigure $sock -translation] trRead trWrite
	    fconfigure $sock -translation [list $trRead binary]
	    fileevent $sock writable [list http::Write $token]
	    # The http::Write command decides when to make the socket readable,
	    # using the same test as the GET/HEAD case below.
	} else {
	    # GET or HEAD method.
	    if {    (![catch {fileevent $sock readable} binding])
		 && ($binding eq [list http::CheckEof $sock])
	    } {
		# Remove the "fileevent readable" binding of an idle persistent
		# socket to http::CheckEof.  We can no longer treat bytes
		# received as junk. The server might still time out and
		# half-close the socket if it has not yet received the first
		# "puts".
		fileevent $sock readable {}
	    }
	    puts $sock ""
	    flush $sock
	    Log ^C$tk end sending request - token $token
	    # End of writing (GET/HEAD methods).  The request has been sent.

	    DoneRequest $token
	}

    } err]} {
	# The socket probably was never connected, OR the connection dropped
	# later, OR https handshake error, which may be discovered as late as
	# the "flush" command above...
	Log "WARNING - if testing, pay special attention to this\
		case (GI) which is seldom executed - token $token"
	if {[info exists state(reusing)] && $state(reusing)} {
	    # The socket was closed at the server end, and closed at
	    # this end by http::CheckEof.
    	    if {[TestForReplay $token write $err a]} {
		return
	    } else {
		Finish $token {failed to re-use socket}
	    }

	    # else:
	    # This is NOT a persistent socket that has been closed since its
	    # last use.
	    # If any other requests are in flight or pipelined/queued, they will
	    # be discarded.
	} elseif {$state(status) eq ""} {
	    # ...https handshake errors come here.
	    set msg [registerError $sock]
	    registerError $sock {}
	    if {$msg eq {}} {
		set msg {failed to use socket}
	    }
	    Finish $token $msg
	} elseif {$state(status) ne "error"} {
	    Finish $token $err
	}
    }
}

# http::registerError
#
#	Called (for example when processing TclTLS activity) to register
#	an error for a connection on a specific socket.  This helps
#	http::Connected to deliver meaningful error messages, e.g. when a TLS
#	certificate fails verification.
#
#	Usage: http::registerError socket ?newValue?
#
#	"set" semantics, except that a "get" (a call without a new value) for a
#	non-existent socket returns {}, not an error.

proc http::registerError {sock args} {
    variable registeredErrors

    if {    ([llength $args] == 0)
	 && (![info exists registeredErrors($sock)])
    } {
	return
    } elseif {    ([llength $args] == 1)
	       && ([lindex $args 0] eq {})
    } {
	unset -nocomplain registeredErrors($sock)
	return
    }
    set registeredErrors($sock) {*}$args
}

# http::DoneRequest --
#
#	Command called when a request has been sent.  It will arrange the
#	next request and/or response as appropriate.
#
#	If this command is called when $socketClosing(*), the request $token
#	that calls it must be pipelined and destined to fail.

proc http::DoneRequest {token} {
    variable http
    variable socketMapping
    variable socketRdState
    variable socketWrState
    variable socketRdQueue
    variable socketWrQueue
    variable socketClosing
    variable socketPlayCmd

    variable $token
    upvar 0 $token state
    set tk [namespace tail $token]
    set sock $state(sock)

    # If pipelined, connect the next HTTP request to the socket.
    if {$state(reusing) && $state(-pipeline)} {
	# Enable next token (if any) to write.
	# The value "Wready" is set only here, and
	# in http::Event after reading the response-headers of a
	# non-reusing transaction.
	# Previous value is $token. It cannot be pending.
	set socketWrState($state(socketinfo)) Wready

	# Now ready to write the next pipelined request (if any).
	http::NextPipelinedWrite $token
    } else {
	# If pipelined, this is the first transaction on this socket.  We wait
	# for the response headers to discover whether the connection is
	# persistent.  (If this is not done and the connection is not
	# persistent, we SHOULD retry and then MUST NOT pipeline before knowing
	# that we have a persistent connection
	# (rfc2616 8.1.2.2)).
    }

    # Connect to receive the response, unless the socket is pipelined
    # and another response is being sent.
    # This code block is separate from the code below because there are
    # cases where socketRdState already has the value $token.
    if {    $state(-keepalive)
	 && $state(-pipeline)
	 && [info exists socketRdState($state(socketinfo))]
	 && ($socketRdState($state(socketinfo)) eq "Rready")
    } {
	#Log pipelined, GRANT read access to $token in Connected
	set socketRdState($state(socketinfo)) $token
    }

    if {    $state(-keepalive)
	 && $state(-pipeline)
	 && [info exists socketRdState($state(socketinfo))]
	 && ($socketRdState($state(socketinfo)) ne $token)
    } {
	# Do not read from the socket until it is ready.
	##Log "HTTP response for token $token is queued for pipelined use"
	# If $socketClosing(*), then the caller will be a pipelined write and
	# execution will come here.
	# This token has already been recorded as "in flight" for writing.
	# When the socket is closed, the read queue will be cleared in
	# CloseQueuedQueries and so the "lappend" here has no effect.
	lappend socketRdQueue($state(socketinfo)) $token
    } else {
	# In the pipelined case, connection for reading depends on the
	# value of socketRdState.
	# In the nonpipeline case, connection for reading always occurs.
	ReceiveResponse $token
    }
}

# http::ReceiveResponse
#
#	Connects token to its socket for reading.

proc http::ReceiveResponse {token} {
    variable $token
    upvar 0 $token state
    set tk [namespace tail $token]
    set sock $state(sock)

    #Log ---- $state(socketinfo) >> conn to $token for HTTP response
    lassign [fconfigure $sock -translation] trRead trWrite
    fconfigure $sock -translation [list auto $trWrite] \
		     -buffersize $state(-blocksize)
    Log ^D$tk begin receiving response - token $token

    coroutine ${token}EventCoroutine http::Event $sock $token
    if {[info exists state(-handler)] || [info exists state(-progress)]} {
        fileevent $sock readable [list http::EventGateway $sock $token]
    } else {
        fileevent $sock readable ${token}EventCoroutine
    }
    return
}


# http::EventGateway
#
#	Bug [c2dc1da315].
#	- Recursive launch of the coroutine can occur if a -handler or -progress
#	  callback is used, and the callback command enters the event loop.
#	- To prevent this, the fileevent "binding" is disabled while the
#	  coroutine is in flight.
#	- If a recursive call occurs despite these precautions, it is not
#	  trapped and discarded here, because it is better to report it as a
#	  bug.
#	- Although this solution is believed to be sufficiently general, it is
#	  used only if -handler or -progress is specified.  In other cases,
#	  the coroutine is called directly.

proc http::EventGateway {sock token} {
    variable $token
    upvar 0 $token state
    fileevent $sock readable {}
    catch {${token}EventCoroutine} res opts
    if {[info commands ${token}EventCoroutine] ne {}} {
        # The coroutine can be deleted by completion (a non-yield return), by
        # http::Finish (when there is a premature end to the transaction), by
        # http::reset or http::cleanup, or if the caller set option -channel
        # but not option -handler: in the last case reading from the socket is
        # now managed by commands ::http::Copy*, http::ReceiveChunked, and
        # http::make-transformation-chunked.
        #
        # Catch in case the coroutine has closed the socket.
        catch {fileevent $sock readable [list http::EventGateway $sock $token]}
    }

    # If there was an error, re-throw it.
    return -options $opts $res
}


# http::NextPipelinedWrite
#
# - Connecting a socket to a token for writing is done by this command and by
#   command KeepSocket.
# - If another request has a pipelined write scheduled for $token's socket,
#   and if the socket is ready to accept it, connect the write and update
#   the queue accordingly.
# - This command is called from http::DoneRequest and http::Event,
#   IF $state(-pipeline) AND (the current transfer has reached the point at
#   which the socket is ready for the next request to be written).
# - This command is called when a token has write access and is pipelined and
#   keep-alive, and sets socketWrState to Wready.
# - The command need not consider the case where socketWrState is set to a token
#   that does not yet have write access.  Such a token is waiting for Rready,
#   and the assignment of the connection to the token will be done elsewhere (in
#   http::KeepSocket).
# - This command cannot be called after socketWrState has been set to a
#   "pending" token value (that is then overwritten by the caller), because that
#   value is set by this command when it is called by an earlier token when it
#   relinquishes its write access, and the pending token is always the next in
#   line to write.

proc http::NextPipelinedWrite {token} {
    variable http
    variable socketRdState
    variable socketWrState
    variable socketWrQueue
    variable socketClosing
    variable $token
    upvar 0 $token state
    set connId $state(socketinfo)

    if {    [info exists socketClosing($connId)]
	 && $socketClosing($connId)
    } {
	# socketClosing(*) is set because the server has sent a
	# "Connection: close" header.
	# Behave as if the queues are empty - so do nothing.
    } elseif {    $state(-pipeline)
	 && [info exists socketWrState($connId)]
	 && ($socketWrState($connId) eq "Wready")

	 && [info exists socketWrQueue($connId)]
	 && [llength $socketWrQueue($connId)]
	 && ([set token2 [lindex $socketWrQueue($connId) 0]
	      set ${token2}(-pipeline)
	     ]
	    )
    } {
	# - The usual case for a pipelined connection, ready for a new request.
	#Log pipelined, GRANT write access to $token2 in NextPipelinedWrite
	set conn [set ${token2}(tmpConnArgs)]
	set socketWrState($connId) $token2
	set socketWrQueue($connId) [lrange $socketWrQueue($connId) 1 end]
	# Connect does its own fconfigure.
	fileevent $state(sock) writable [list http::Connect $token2 {*}$conn]
	#Log ---- $connId << conn to $token2 for HTTP request (b)

	# In the tests below, the next request will be nonpipeline.
    } elseif {    $state(-pipeline)
	       && [info exists socketWrState($connId)]
	       && ($socketWrState($connId) eq "Wready")

	       && [info exists socketWrQueue($connId)]
	       && [llength $socketWrQueue($connId)]
	       && (![ set token3 [lindex $socketWrQueue($connId) 0]
		      set ${token3}(-pipeline)
		    ]
		  )

	       && [info exists socketRdState($connId)]
	       && ($socketRdState($connId) eq "Rready")
    } {
	# The case in which the next request will be non-pipelined, and the read
	# and write queues is ready: which is the condition for a non-pipelined
	# write.
	variable $token3
	upvar 0 $token3 state3
	set conn [set ${token3}(tmpConnArgs)]
	#Log nonpipeline, GRANT r/w access to $token3 in NextPipelinedWrite
	set socketRdState($connId) $token3
	set socketWrState($connId) $token3
	set socketWrQueue($connId) [lrange $socketWrQueue($connId) 1 end]
	# Connect does its own fconfigure.
	fileevent $state(sock) writable [list http::Connect $token3 {*}$conn]
	#Log ---- $state(sock) << conn to $token3 for HTTP request (c)

    } elseif {    $state(-pipeline)
	 && [info exists socketWrState($connId)]
	 && ($socketWrState($connId) eq "Wready")

	 && [info exists socketWrQueue($connId)]
	 && [llength $socketWrQueue($connId)]
	 && (![set token2 [lindex $socketWrQueue($connId) 0]
	      set ${token2}(-pipeline)
	     ]
	    )
    } {
	# - The case in which the next request will be non-pipelined, but the
	#   read queue is NOT ready.
	# - A read is queued or in progress, but not a write.  Cannot start the
	#   nonpipeline transaction, but must set socketWrState to prevent a new
	#   pipelined request (in http::geturl) jumping the queue.
	# - Because socketWrState($connId) is not set to Wready, the assignment
	#   of the connection to $token2 will be done elsewhere - by command
	#   http::KeepSocket when $socketRdState($connId) is set to "Rready".

	#Log re-use nonpipeline, GRANT delayed write access to $token in NextP..
	set socketWrState($connId) peNding
    }
}

# http::CancelReadPipeline
#
#	Cancel pipelined responses on a closing "Keep-Alive" socket.
#
#	- Called by a variable trace on "unset socketRdState($connId)".
#	- The variable relates to a Keep-Alive socket, which has been closed.
#	- Cancels all pipelined responses. The requests have been sent,
#	  the responses have not yet been received.
#	- This is a hard cancel that ends each transaction with error status,
#	  and closes the connection. Do not use it if you want to replay failed
#	  transactions.
#	- N.B. Always delete ::http::socketRdState($connId) before deleting
#	  ::http::socketRdQueue($connId), or this command will do nothing.
#
# Arguments
#	As for a trace command on a variable.

proc http::CancelReadPipeline {name1 connId op} {
    variable socketRdQueue
    ##Log CancelReadPipeline $name1 $connId $op
    if {[info exists socketRdQueue($connId)]} {
	set msg {the connection was closed by CancelReadPipeline}
	foreach token $socketRdQueue($connId) {
	    set tk [namespace tail $token]
	    Log ^X$tk end of response "($msg)" - token $token
	    set ${token}(status) eof
	    Finish $token ;#$msg
	}
	set socketRdQueue($connId) {}
    }
}

# http::CancelWritePipeline
#
#	Cancel queued events on a closing "Keep-Alive" socket.
#
#	- Called by a variable trace on "unset socketWrState($connId)".
#	- The variable relates to a Keep-Alive socket, which has been closed.
#	- In pipelined or nonpipeline case: cancels all queued requests.  The
#	  requests have not yet been sent, the responses are not due.
#	- This is a hard cancel that ends each transaction with error status,
#	  and closes the connection. Do not use it if you want to replay failed
#	  transactions.
#	- N.B. Always delete ::http::socketWrState($connId) before deleting
#	  ::http::socketWrQueue($connId), or this command will do nothing.
#
# Arguments
#	As for a trace command on a variable.

proc http::CancelWritePipeline {name1 connId op} {
    variable socketWrQueue

    ##Log CancelWritePipeline $name1 $connId $op
    if {[info exists socketWrQueue($connId)]} {
	set msg {the connection was closed by CancelWritePipeline}
	foreach token $socketWrQueue($connId) {
	    set tk [namespace tail $token]
	    Log ^X$tk end of response "($msg)" - token $token
	    set ${token}(status) eof
	    Finish $token ;#$msg
	}
	set socketWrQueue($connId) {}
    }
}

# http::ReplayIfDead --
#
# - A query on a re-used persistent socket failed at the earliest opportunity,
#   because the socket had been closed by the server.  Keep the token, tidy up,
#   and try to connect on a fresh socket.
# - The connection is monitored for eof by the command http::CheckEof.  Thus
#   http::ReplayIfDead is needed only when a server event (half-closing an
#   apparently idle connection), and a client event (sending a request) occur at
#   almost the same time, and neither client nor server detects the other's
#   action before performing its own (an "asynchronous close event").
# - To simplify testing of http::ReplayIfDead, set TEST_EOF 1 in
#   http::KeepSocket, and then http::ReplayIfDead will be called if http::geturl
#   is called at any time after the server timeout.
#
# Arguments:
#	token	Connection token.
#
# Side Effects:
#	Use the same token, but try to open a new socket.

proc http::ReplayIfDead {tokenArg doing} {
    variable socketMapping
    variable socketRdState
    variable socketWrState
    variable socketRdQueue
    variable socketWrQueue
    variable socketClosing
    variable socketPlayCmd

    variable $tokenArg
    upvar 0 $tokenArg stateArg

    Log running http::ReplayIfDead for $tokenArg $doing

    # 1. Merge the tokens for transactions in flight, the read (response) queue,
    #    and the write (request) queue.

    set InFlightR {}
    set InFlightW {}

    # Obtain the tokens for transactions in flight.
    if {$stateArg(-pipeline)} {
	# Two transactions may be in flight.  The "read" transaction was first.
	# It is unlikely that the server would close the socket if a response
	# was pending; however, an earlier request (as well as the present
	# request) may have been sent and ignored if the socket was half-closed
	# by the server.

	if {    [info exists socketRdState($stateArg(socketinfo))]
	     && ($socketRdState($stateArg(socketinfo)) ne "Rready")
	} {
	    lappend InFlightR $socketRdState($stateArg(socketinfo))
	} elseif {($doing eq "read")} {
	    lappend InFlightR $tokenArg
	}

	if {    [info exists socketWrState($stateArg(socketinfo))]
	     && $socketWrState($stateArg(socketinfo)) ni {Wready peNding}
	} {
	    lappend InFlightW $socketWrState($stateArg(socketinfo))
	} elseif {($doing eq "write")} {
	    lappend InFlightW $tokenArg
	}

	# Report any inconsistency of $tokenArg with socket*state.
	if {    ($doing eq "read")
	     && [info exists socketRdState($stateArg(socketinfo))]
	     && ($tokenArg ne $socketRdState($stateArg(socketinfo)))
	} {
	    Log WARNING - ReplayIfDead pipelined tokenArg $tokenArg $doing \
		    ne socketRdState($stateArg(socketinfo)) \
		      $socketRdState($stateArg(socketinfo))

	} elseif {
		($doing eq "write")
	     && [info exists socketWrState($stateArg(socketinfo))]
	     && ($tokenArg ne $socketWrState($stateArg(socketinfo)))
	} {
	    Log WARNING - ReplayIfDead pipelined tokenArg $tokenArg $doing \
		    ne socketWrState($stateArg(socketinfo)) \
		      $socketWrState($stateArg(socketinfo))
	}
    } else {
	# One transaction should be in flight.
	# socketRdState, socketWrQueue are used.
	# socketRdQueue should be empty.

	# Report any inconsistency of $tokenArg with socket*state.
	if {$tokenArg ne $socketRdState($stateArg(socketinfo))} {
	    Log WARNING - ReplayIfDead nonpipeline tokenArg $tokenArg $doing \
		    ne socketRdState($stateArg(socketinfo)) \
		      $socketRdState($stateArg(socketinfo))
	}

	# Report the inconsistency that socketRdQueue is non-empty.
	if {    [info exists socketRdQueue($stateArg(socketinfo))]
	     && ($socketRdQueue($stateArg(socketinfo)) ne {})
	} {
	    Log WARNING - ReplayIfDead nonpipeline tokenArg $tokenArg $doing \
		    has read queue socketRdQueue($stateArg(socketinfo)) \
		    $socketRdQueue($stateArg(socketinfo)) ne {}
	}

	lappend InFlightW $socketRdState($stateArg(socketinfo))
	set socketRdQueue($stateArg(socketinfo)) {}
    }

    set newQueue {}
    lappend newQueue {*}$InFlightR
    lappend newQueue {*}$socketRdQueue($stateArg(socketinfo))
    lappend newQueue {*}$InFlightW
    lappend newQueue {*}$socketWrQueue($stateArg(socketinfo))


    # 2. Tidy up tokenArg.  This is a cut-down form of Finish/CloseSocket.
    #    Do not change state(status).
    #    No need to after cancel stateArg(after) - either this is done in
    #    ReplayCore/ReInit, or Finish is called.

    catch {close $stateArg(sock)}

    # 2a. Tidy the tokens in the queues - this is done in ReplayCore/ReInit.
    # - Transactions, if any, that are awaiting responses cannot be completed.
    #   They are listed for re-sending in newQueue.
    # - All tokens are preserved for re-use by ReplayCore, and their variables
    #   will be re-initialised by calls to ReInit.
    # - The relevant element of socketMapping, socketRdState, socketWrState,
    #   socketRdQueue, socketWrQueue, socketClosing, socketPlayCmd will be set
    #   to new values in ReplayCore.

    ReplayCore $newQueue
}

# http::ReplayIfClose --
#
#	A request on a socket that was previously "Connection: keep-alive" has
#	received a "Connection: close" response header.  The server supplies
#	that response correctly, but any later requests already queued on this
#	connection will be lost when the socket closes.
#
#	This command takes arguments that represent the socketWrState,
#	socketRdQueue and socketWrQueue for this connection.  The socketRdState
#	is not needed because the server responds in full to the request that
#	received the "Connection: close" response header.
#
#	Existing request tokens $token (::http::$n) are preserved.  The caller
#	will be unaware that the request was processed this way.

proc http::ReplayIfClose {Wstate Rqueue Wqueue} {
    Log running http::ReplayIfClose for $Wstate $Rqueue $Wqueue

    if {$Wstate in $Rqueue || $Wstate in $Wqueue} {
	Log WARNING duplicate token in http::ReplayIfClose - token $Wstate
	set Wstate Wready
    }

    # 1. Create newQueue
    set InFlightW {}
    if {$Wstate ni {Wready peNding}} {
	lappend InFlightW $Wstate
    }

    set newQueue {}
    lappend newQueue {*}$Rqueue
    lappend newQueue {*}$InFlightW
    lappend newQueue {*}$Wqueue

    # 2. Cleanup - none needed, done by the caller.

    ReplayCore $newQueue
}

# http::ReInit --
#
#	Command to restore a token's state to a condition that
#	makes it ready to replay a request.
#
#	Command http::geturl stores extra state in state(tmp*) so
#	we don't need to do the argument processing again.
#
#	The caller must:
#	- Set state(reusing) and state(sock) to their new values after calling
#	  this command.
#	- Unset state(tmpState), state(tmpOpenCmd) if future calls to ReplayCore
#	  or ReInit are inappropriate for this token. Typically only one retry
#	  is allowed.
#	The caller may also unset state(tmpConnArgs) if this value (and the
#	token) will be used immediately.  The value is needed by tokens that
#	will be stored in a queue.
#
# Arguments:
#	token	Connection token.
#
# Return Value: (boolean) true iff the re-initialisation was successful.

proc http::ReInit {token} {
    variable $token
    upvar 0 $token state

    if {!(
	      [info exists state(tmpState)]
	   && [info exists state(tmpOpenCmd)]
	   && [info exists state(tmpConnArgs)]
	 )
    } {
	Log FAILED in http::ReInit via ReplayCore - NO tmp vars for $token
	return 0
    }

    if {[info exists state(after)]} {
	after cancel $state(after)
	unset state(after)
    }

    # Don't alter state(status) - this would trigger http::wait if it is in use.
    set tmpState    $state(tmpState)
    set tmpOpenCmd  $state(tmpOpenCmd)
    set tmpConnArgs $state(tmpConnArgs)
    foreach name [array names state] {
	if {$name ne "status"} {
	    unset state($name)
	}
    }

    # Don't alter state(status).
    # Restore state(tmp*) - the caller may decide to unset them.
    # Restore state(tmpConnArgs) which is needed for connection.
    # state(tmpState), state(tmpOpenCmd) are needed only for retries.

    dict unset tmpState status
    array set state $tmpState
    set state(tmpState)    $tmpState
    set state(tmpOpenCmd)  $tmpOpenCmd
    set state(tmpConnArgs) $tmpConnArgs

    return 1
}

# http::ReplayCore --
#
#	Command to replay a list of requests, using existing connection tokens.
#
#	Abstracted from http::geturl which stores extra state in state(tmp*) so
#	we don't need to do the argument processing again.
#
# Arguments:
#	newQueue	List of connection tokens.
#
# Side Effects:
#	Use existing tokens, but try to open a new socket.

proc http::ReplayCore {newQueue} {
    variable socketMapping
    variable socketRdState
    variable socketWrState
    variable socketRdQueue
    variable socketWrQueue
    variable socketClosing
    variable socketPlayCmd

    if {[llength $newQueue] == 0} {
	# Nothing to do.
	return
    }

    ##Log running ReplayCore for {*}$newQueue
    set newToken [lindex $newQueue 0]
    set newQueue [lrange $newQueue 1 end]

    # 3. Use newToken, and restore its values of state(*).  Do not restore
    #    elements tmp* - we try again only once.

    set token $newToken
    variable $token
    upvar 0 $token state

    if {![ReInit $token]} {
	Log FAILED in http::ReplayCore - NO tmp vars
	Finish $token {cannot send this request again}
	return
    }

    set tmpState    $state(tmpState)
    set tmpOpenCmd  $state(tmpOpenCmd)
    set tmpConnArgs $state(tmpConnArgs)
    unset state(tmpState)
    unset state(tmpOpenCmd)
    unset state(tmpConnArgs)

    set state(reusing) 0

    if {$state(-timeout) > 0} {
	set resetCmd [list http::reset $token timeout]
	set state(after) [after $state(-timeout) $resetCmd]
    }

    set pre [clock milliseconds]
    ##Log pre socket opened, - token $token
    ##Log $tmpOpenCmd - token $token
    # 4. Open a socket.
    if {[catch {eval $tmpOpenCmd} sock]} {
	# Something went wrong while trying to establish the connection.
	Log FAILED - $sock
	set state(sock) NONE
	Finish $token $sock
	return
    }
    ##Log post socket opened, - token $token
    set delay [expr {[clock milliseconds] - $pre}]
    if {$delay > 3000} {
	Log socket delay $delay - token $token
    }
    # Command [socket] is called with -async, but takes 5s to 5.1s to return,
    # with probability of order 1 in 10,000.  This may be a bizarre scheduling
    # issue with my (KJN's) system (Fedora Linux).
    # This does not cause a problem (unless the request times out when this
    # command returns).

    # 5. Configure the persistent socket data.
    if {$state(-keepalive)} {
	set socketMapping($state(socketinfo)) $sock

	if {![info exists socketRdState($state(socketinfo))]} {
	    set socketRdState($state(socketinfo)) {}
	    set varName ::http::socketRdState($state(socketinfo))
	    trace add variable $varName unset ::http::CancelReadPipeline
	}

	if {![info exists socketWrState($state(socketinfo))]} {
	    set socketWrState($state(socketinfo)) {}
	    set varName ::http::socketWrState($state(socketinfo))
	    trace add variable $varName unset ::http::CancelWritePipeline
	}

	if {$state(-pipeline)} {
	    #Log new, init for pipelined, GRANT write acc to $token ReplayCore
	    set socketRdState($state(socketinfo)) $token
	    set socketWrState($state(socketinfo)) $token
	} else {
	    #Log new, init for nonpipeline, GRANT r/w acc to $token ReplayCore
	    set socketRdState($state(socketinfo)) $token
	    set socketWrState($state(socketinfo)) $token
	}

	set socketRdQueue($state(socketinfo)) {}
	set socketWrQueue($state(socketinfo)) $newQueue
	set socketClosing($state(socketinfo)) 0
	set socketPlayCmd($state(socketinfo)) {ReplayIfClose Wready {} {}}
    }

    ##Log pre newQueue ReInit, - token $token
    # 6. Configure sockets in the queue.
    foreach tok $newQueue {
	if {[ReInit $tok]} {
	    set ${tok}(reusing) 1
	    set ${tok}(sock) $sock
	} else {
	    set ${tok}(reusing) 1
	    set ${tok}(sock) NONE
	    Finish $token {cannot send this request again}
	}
    }

    # 7. Configure the socket for newToken to send a request.
    set state(sock) $sock
    Log "Using $sock for $state(socketinfo) - token $token" \
	[expr {$state(-keepalive)?"keepalive":""}]

    # Initialisation of a new socket.
    ##Log socket opened, now fconfigure - token $token
    fconfigure $sock -translation {auto crlf} -buffersize $state(-blocksize)
    ##Log socket opened, DONE fconfigure - token $token

    # Connect does its own fconfigure.
    fileevent $sock writable [list http::Connect $token {*}$tmpConnArgs]
    #Log ---- $sock << conn to $token for HTTP request (e)
}

# Data access functions:
# Data - the URL data
# Status - the transaction status: ok, reset, eof, timeout, error
# Code - the HTTP transaction code, e.g., 200
# Size - the size of the URL data

proc http::data {token} {
    variable $token
    upvar 0 $token state
    return $state(body)
}
proc http::status {token} {
    if {![info exists $token]} {
	return "error"
    }
    variable $token
    upvar 0 $token state
    return $state(status)
}
proc http::code {token} {
    variable $token
    upvar 0 $token state
    return $state(http)
}
proc http::ncode {token} {
    variable $token
    upvar 0 $token state
    if {[regexp {[0-9]{3}} $state(http) numeric_code]} {
	return $numeric_code
    } else {
	return $state(http)
    }
}
proc http::size {token} {
    variable $token
    upvar 0 $token state
    return $state(currentsize)
}
proc http::meta {token} {
    variable $token
    upvar 0 $token state
    return $state(meta)
}
proc http::error {token} {
    variable $token
    upvar 0 $token state
    if {[info exists state(error)]} {
	return $state(error)
    }
    return ""
}

# http::cleanup
#
#	Garbage collect the state associated with a transaction
#
# Arguments
#	token	The token returned from http::geturl
#
# Side Effects
#	unsets the state array

proc http::cleanup {token} {
    variable $token
    upvar 0 $token state
    if {[info commands ${token}EventCoroutine] ne {}} {
	rename ${token}EventCoroutine {}
    }
    if {[info exists state(after)]} {
	after cancel $state(after)
	unset state(after)
    }
    if {[info exists state]} {
	unset state
    }
}

# http::Connect
#
#	This callback is made when an asyncronous connection completes.
#
# Arguments
#	token	The token returned from http::geturl
#
# Side Effects
#	Sets the status of the connection, which unblocks
# 	the waiting geturl call

proc http::Connect {token proto phost srvurl} {
    variable $token
    upvar 0 $token state
    set tk [namespace tail $token]
    set err "due to unexpected EOF"
    if {
	[eof $state(sock)] ||
	[set err [fconfigure $state(sock) -error]] ne ""
    } {
	Log "WARNING - if testing, pay special attention to this\
		case (GJ) which is seldom executed - token $token"
	if {[info exists state(reusing)] && $state(reusing)} {
	    # The socket was closed at the server end, and closed at
	    # this end by http::CheckEof.
	    if {[TestForReplay $token write $err b]} {
		return
	    }

	    # else:
	    # This is NOT a persistent socket that has been closed since its
	    # last use.
	    # If any other requests are in flight or pipelined/queued, they will
	    # be discarded.
	}
	Finish $token "connect failed $err"
    } else {
	set state(state) connecting
	fileevent $state(sock) writable {}
	::http::Connected $token $proto $phost $srvurl
    }
}

# http::Write
#
#	Write POST query data to the socket
#
# Arguments
#	token	The token for the connection
#
# Side Effects
#	Write the socket and handle callbacks.

proc http::Write {token} {
    variable http
    variable socketMapping
    variable socketRdState
    variable socketWrState
    variable socketRdQueue
    variable socketWrQueue
    variable socketClosing
    variable socketPlayCmd

    variable $token
    upvar 0 $token state
    set tk [namespace tail $token]
    set sock $state(sock)

    # Output a block.  Tcl will buffer this if the socket blocks
    set done 0
    if {[catch {
	# Catch I/O errors on dead sockets

	if {[info exists state(-query)]} {
	    # Chop up large query strings so queryprogress callback can give
	    # smooth feedback.
	    if {    $state(queryoffset) + $state(-queryblocksize)
		 >= $state(querylength)
	    } {
		# This will be the last puts for the request-body.
		if {    (![catch {fileevent $sock readable} binding])
		     && ($binding eq [list http::CheckEof $sock])
		} {
		    # Remove the "fileevent readable" binding of an idle
		    # persistent socket to http::CheckEof.  We can no longer
		    # treat bytes received as junk. The server might still time
		    # out and half-close the socket if it has not yet received
		    # the first "puts".
		    fileevent $sock readable {}
		}
	    }
	    puts -nonewline $sock \
		[string range $state(-query) $state(queryoffset) \
		     [expr {$state(queryoffset) + $state(-queryblocksize) - 1}]]
	    incr state(queryoffset) $state(-queryblocksize)
	    if {$state(queryoffset) >= $state(querylength)} {
		set state(queryoffset) $state(querylength)
		set done 1
	    }
	} else {
	    # Copy blocks from the query channel

	    set outStr [read $state(-querychannel) $state(-queryblocksize)]
	    if {[eof $state(-querychannel)]} {
		# This will be the last puts for the request-body.
		if {    (![catch {fileevent $sock readable} binding])
		     && ($binding eq [list http::CheckEof $sock])
		} {
		    # Remove the "fileevent readable" binding of an idle
		    # persistent socket to http::CheckEof.  We can no longer
		    # treat bytes received as junk. The server might still time
		    # out and half-close the socket if it has not yet received
		    # the first "puts".
		    fileevent $sock readable {}
		}
	    }
	    puts -nonewline $sock $outStr
	    incr state(queryoffset) [string length $outStr]
	    if {[eof $state(-querychannel)]} {
		set done 1
	    }
	}
    } err]} {
	# Do not call Finish here, but instead let the read half of the socket
	# process whatever server reply there is to get.

	set state(posterror) $err
	set done 1
    }

    if {$done} {
	catch {flush $sock}
	fileevent $sock writable {}
	Log ^C$tk end sending request - token $token
	# End of writing (POST method).  The request has been sent.

	DoneRequest $token
    }

    # Callback to the client after we've completely handled everything.

    if {[string length $state(-queryprogress)]} {
	eval $state(-queryprogress) \
	    [list $token $state(querylength) $state(queryoffset)]
    }
}

# http::Event
#
#	Handle input on the socket. This command is the core of
#	the coroutine commands ${token}EventCoroutine that are
#	bound to "fileevent $sock readable" and process input.
#
# Arguments
#	sock	The socket receiving input.
#	token	The token returned from http::geturl
#
# Side Effects
#	Read the socket and handle callbacks.

proc http::Event {sock token} {
    variable http
    variable socketMapping
    variable socketRdState
    variable socketWrState
    variable socketRdQueue
    variable socketWrQueue
    variable socketClosing
    variable socketPlayCmd

    variable $token
    upvar 0 $token state
    set tk [namespace tail $token]
    while 1 {
	yield
	##Log Event call - token $token

	if {![info exists state]} {
	    Log "Event $sock with invalid token '$token' - remote close?"
	    if {![eof $sock]} {
		if {[set d [read $sock]] ne ""} {
		    Log "WARNING: additional data left on closed socket\
			    - token $token"
		}
	    }
	    Log ^X$tk end of response (token error) - token $token
	    CloseSocket $sock
	    return
	}
	if {$state(state) eq "connecting"} {
	    ##Log - connecting - token $token
	    if {    $state(reusing)
		 && $state(-pipeline)
		 && ($state(-timeout) > 0)
		 && (![info exists state(after)])
	    } {
		set state(after) [after $state(-timeout) \
			[list http::reset $token timeout]]
	    }

	    if {[catch {gets $sock state(http)} nsl]} {
		Log "WARNING - if testing, pay special attention to this\
			case (GK) which is seldom executed - token $token"
		if {[info exists state(reusing)] && $state(reusing)} {
		    # The socket was closed at the server end, and closed at
		    # this end by http::CheckEof.

		    if {[TestForReplay $token read $nsl c]} {
			return
		    }

		    # else:
		    # This is NOT a persistent socket that has been closed since
		    # its last use.
		    # If any other requests are in flight or pipelined/queued,
		    # they will be discarded.
		} else {
		    Log ^X$tk end of response (error) - token $token
		    Finish $token $nsl
		    return
		}
	    } elseif {$nsl >= 0} {
		##Log - connecting 1 - token $token
		set state(state) "header"
	    } elseif {    [eof $sock]
		       && [info exists state(reusing)]
		       && $state(reusing)
	    } {
		# The socket was closed at the server end, and we didn't notice.
		# This is the first read - where the closure is usually first
		# detected.

		if {[TestForReplay $token read {} d]} {
		    return
		}

		# else:
		# This is NOT a persistent socket that has been closed since its
		# last use.
		# If any other requests are in flight or pipelined/queued, they
		# will be discarded.
	    }
	} elseif {$state(state) eq "header"} {
	    if {[catch {gets $sock line} nhl]} {
		##Log header failed - token $token
		Log ^X$tk end of response (error) - token $token
		Finish $token $nhl
		return
	    } elseif {$nhl == 0} {
		##Log header done - token $token
		Log ^E$tk end of response headers - token $token
		# We have now read all headers
		# We ignore HTTP/1.1 100 Continue returns. RFC2616 sec 8.2.3
		if {    ($state(http) == "")
		     || ([regexp {^\S+\s(\d+)} $state(http) {} x] && $x == 100)
		} {
		    set state(state) "connecting"
		    continue
		    # This was a "return" in the pre-coroutine code.
		}

		if {    ([info exists state(connection)])
		     && ([info exists socketMapping($state(socketinfo))])
		     && ($state(connection) eq "keep-alive")
		     && ($state(-keepalive))
		     && (!$state(reusing))
		     && ($state(-pipeline))
		} {
		    # Response headers received for first request on a
		    # persistent socket.  Now ready for pipelined writes (if
		    # any).
		    # Previous value is $token. It cannot be "pending".
		    set socketWrState($state(socketinfo)) Wready
		    http::NextPipelinedWrite $token
		}

		# Once a "close" has been signaled, the client MUST NOT send any
		# more requests on that connection.
		#
		# If either the client or the server sends the "close" token in
		# the Connection header, that request becomes the last one for
		# the connection.

		if {    ([info exists state(connection)])
		     && ([info exists socketMapping($state(socketinfo))])
		     && ($state(connection) eq "close")
		     && ($state(-keepalive))
		} {
		    # The server warns that it will close the socket after this
		    # response.
		    ##Log WARNING - socket will close after response for $token
		    # Prepare data for a call to ReplayIfClose.
		    if {    ($socketRdQueue($state(socketinfo)) ne {})
			 || ($socketWrQueue($state(socketinfo)) ne {})
			 || ($socketWrState($state(socketinfo)) ni
						[list Wready peNding $token])
		    } {
			set InFlightW $socketWrState($state(socketinfo))
			if {$InFlightW in [list Wready peNding $token]} {
			    set InFlightW Wready
			} else {
			    set msg "token ${InFlightW} is InFlightW"
			    ##Log $msg - token $token
			}

			set socketPlayCmd($state(socketinfo)) \
				[list ReplayIfClose $InFlightW \
				$socketRdQueue($state(socketinfo)) \
				$socketWrQueue($state(socketinfo))]

			# - All tokens are preserved for re-use by ReplayCore.
			# - Queues are preserved in case of Finish with error,
			#   but are not used for anything else because
			#   socketClosing(*) is set below.
			# - Cancel the state(after) timeout events.
			foreach tokenVal $socketRdQueue($state(socketinfo)) {
			    if {[info exists ${tokenVal}(after)]} {
				after cancel [set ${tokenVal}(after)]
				unset ${tokenVal}(after)
			    }
			}

		    } else {
			set socketPlayCmd($state(socketinfo)) \
				{ReplayIfClose Wready {} {}}
		    }

		    # Do not allow further connections on this socket.
		    set socketClosing($state(socketinfo)) 1
		}

		set state(state) body

		# If doing a HEAD, then we won't get any body
		if {$state(-v