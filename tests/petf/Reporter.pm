package petf::Reporter;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(init end try get get_outmsg get_errmsg pass_only_on);

#
# using declarations go here
#
use IPC::Open3;
use IO::Select;
use IO::Handle;
use Fcntl; 
use POSIX qw(:errno_h);

sub new { 
    my $type = shift; 
    my %parm = @_ ;
    my $this  = {};
    # set values here
    $this{'driver'} = $parm{'driver'} ;    # just the name of the driver
    $this{'cmd'} = $parm{'cmd'} ;          # the actual command executed
    $this{'prompt'} = $parm{'prompt'} ;    # the prompt from the driver
    $this{'timeout'} = $parm{'timeout'} ;  # the amount of time to wait before giving up
    $this{'pid'} = 0;                      # the process id of the child
    $this{'xfails'} = $parm{'xfails'};     # colon separated list of test numbers that expect to fail
    $this{'CMD_IN'} = IO::Handle->new;     # the file handle for the  $cmd's stdin
    $this{'CMD_OUT'} = IO::Handle->new;    # the file handle for the  $cmd's stdout
    $this{'CMD_ERR'} = IO::Handle->new;    # the file handle for the  $cmd's stderr
    $this{'CMD_IN'} -> autoflush(1);
    $this{'CMD_OUT'} -> autoflush(1);
    $this{'CMD_ERR'} -> autoflush(1);
    $this{'nparts'} = ( $parm{'nparts'} ) ? $parm{'nparts'} : -1; #number of parts in test
    $this{'selector'} = '';                # a selector to read from CMD_OUT or CMD_ERR
    $this{'errmsg'} = 'howdy!';
    $this{'outmsg'} = 'howdy!';
    $this{'hello'} = 'howdy!';
    $this{'verbosity'} = $parm{'verbiosity'}; # set 1 for program flow, 2 for data display
    $this{'part_number'} = 0; # test part number
    $this{'test_result'} = 'PASS';
    bless $this;
    return $this;
}


######################################################################

sub init { 
    my $this = shift;  
    my $cmd = $this{'cmd'};
    my $prompt = $this{'prompt'};
    my $rv = 0;
    my $verbiosity = $this{'verbiosity'};
    my $nparts = $this{'nparts'} ;

    print "\nNPARTS $nparts\n";
    $this{'errmsg'} = "" ;
    $this{'outmsg'} = "" ;

    
    print "cmd == $cmd\n"      unless $this{'verbosity'} < 2 ;
    print "prompt = $prompt\n" unless $this{'verbosity'} < 2 ;
    # first check that we're ready for this
    if ($cmd eq  "" ) { 
	print "cmd == $cmd\n";
	return 0; 
    }
    if ($prompt eq "" ) { 
	print "prompt = $prompt\n";
	return 0; 
    }
    
    # launch the application
    # note: open3 die's on error, so this is put in an eval block
    eval {
	$pid = open3( $this{'CMD_IN'}, $this{'CMD_OUT'}, $this{'CMD_ERR'}, $cmd ) ;
    };
    if ($@) {                                   #if an exception occured.
	if ($@ =~ /open3/) {                    #   if its due to open3
	    $error = "open3 failed: $!\n$@\n";  #      handle by packing msg
	    $this{'errmsg'} = $error;
	    $this{'outmsg'} = $output;
	    return 0;		                
	} else {                                #    else
	    die;                                #      rethrow exception
	}
    }

    $this{'selector'} = IO::Select->new();
    $this{'selector'}->add( $this{'CMD_ERR'}, $this{'CMD_OUT'} );
    #print "new This{selector} = " . $this{'selector'} . "\n"; 
    
    # now gobble up anything up to and including the prompt.
    $rv = $this->scan_output();
    die ( "To Implement: timeout detected" ) if ( $rv != 0 );

    # now we're ready to go
    $this{'errmsg'} = $error;
    $this{'outmsg'} = $output;
    return $this{'pid'} = $pid;
}

######################################################################

sub end {
    my( $this ) = shift;
    # print testfile results
    print "\nTEST_RESULT " . $this{'test_result'} . "\n";

    # just close up everything and wait for child to die.
    close( $this{'CMD_IN'} ) ;
    close( $this{'CMD_OUT'} ) ;
    close( $this{'CMD_ERR'} ) ;
    waitpid( $this{'pid'}, 0 );
}

######################################################################

sub try { 
    my $this = shift;
    my $input = shift;
    my $CMD_IN = $this{'CMD_IN'};
    my $rv; 
    my $verbiosity = $this{'verbiosity'};
    
    print "\nPART ". ++($this{'part_number'}) ."\n";
    
    my $prefix_input = $input;
    $prefix_input =~ s/\n/\nSTDIN: /g;
    print "STDIN: ". $prefix_input . "\n";
    $input = $input . "\n";
    $rv = syswrite( $CMD_IN, $input, length $input );
    if ( !defined( $rv) && $! == EAGAIN ) { 
	print "write blocked\n";
    } elsif ($rv != length $input )  {
	print "incomplete write... rv = $rv, length input = " . length $input . "\n\n"; 
    } else { 
	print "sucessfully wrote\n" unless $verbiosity < 2;
    }	  

    $result = $this->scan_output( );
    return $result ;
}

######################################################################

sub scan_output {
    my $this = shift;  
    my $output = '';
    my $error = '';
    my $verbiosity = $this{'verbiosity'};
    my $prompt = $this{'prompt'};
    my( @ready );
    my $fh;
    my $tempout = '';
    my $tempsz = 64000;
    my $rv = "";
    $this{'errmsg'} = '';
    $this{'outmsg'} = '';
    #print "This{selector} = " . $this{'selector'} . "\n"; 
    print "In scan_output\n"   unless $verbiosity < 1;
    while ( @ready = ( $this{'selector'}->can_read($this{'timeout'}) )  ) { 
	foreach $fh (@ready) { 
	    if ( fileno ($fh) == fileno( $this{'CMD_ERR'} ) ) { 
		$rv = sysread( $this{'CMD_ERR'}, $tempout, $tempsz );
		if (!defined( $rv) && $! == EAGAIN ) { 
		    print "read blocked.\n"; 
		} else { 
		    print "read worked $rv bytes.\n" unless $verbiosity < 2;
		}
		#$tempout = $this{'CMD_ERR'}->getline();
		#chop( $tempout );
		$error = $error . $tempout ;
		$tempout =~ s/\n/\nSTDERR: /g;
		print "STDERR: " . $tempout . "\n";
	    } else {
		$rv = sysread( $this{'CMD_OUT'}, $tempout, $tempsz );
		if (!defined( $rv ) && $! eq EAGAIN ) { 
		    print "read blocked.\n"; 
		} else { 
		    print "read worked $rv bytes.\n"  unless $verbiosity < 2;
		}
		#$tempout = scalar  $this{'CMD_OUT'} -> getline();
		#chop( $tempout );
		$output = $output . $tempout;
		$tempout =~ s/\n/\nSTDOUT: /g;
		print "STDOUT: ". $tempout . "\n"; 
		if ( $tempout =~ /$prompt/ ) { 
		    print "Leaving scan_output.  Got prompt.\n"  
			unless $verbiosity < 1;
		    $this{'errmsg'} = $error;
		    $this{'outmsg'} = $output;
		    return 0;
		}
	    }
	    $this{'selector'}->remove( $fh ) if eof ( $fh );
	}
    }	
    print "Leaving scan_output.  No prompt found.\n"
	unless $verbiosity < 1;
    $this{'errmsg'} = $error;
    $this{'outmsg'} = $output;
    $this{'selector'} = $selector;
    return 1;
}

######################################################################

sub pass{
    my $this = shift;
    my $part_number = $this{'part_number'};
    my $result = 'PASS';
    my $xfails = $this{'xfails'};
    if ( grep ( /\b$part_number\b/, split(':', $xfails ) ) ) {
	$result = 'XPASS';
	$this{'test_result'} = 'FAIL';

    }
    print "\nRESULT $part_number $result\n";
}

sub fail{
    my $this = shift;
  my $part_number = $this{'part_number'};
    my $result = 'PASS';
    my $xfails = $this{'xfails'};
    if ( grep ( /\b$part_number\b/, split(':', $xfails ) ) ) {
	$result = 'XFAIL';
	$this{'test_result'} = 'XFAIL' unless $this{'test_result'} eq 'FAIL';
    } else { 
	$result = 'FAIL';
	$this{'test_result'} = 'FAIL';
    }
    print "\nRESULT $part_number $result\n";
}

#sub xpass{
#    my $this = shift;
#    print "\nRESULT " . $this{'part_number'} . " XPASS\n";
#    $this{'test_result'} = 'FAIL';
#}

#sub xfail{
#    my $this = shift;
#    print "\nRESULT " . $this{'part_number'} . " XFAIL\n";
#    if ( ! ( $this{'test_result'} eq 'FAIL' )) { 
#	$this{'test_result'} = 'XFAIL';
#    }
#}


sub unresolved{
    my $this = shift;
    print "\nRESULT " . $this{'part_number'} . " UNRESOLVED\n";
    $this{'test_result'} = 'FAIL';
}


sub untested{
    my $this = shift;
    print "\nRESULT " . $this{'part_number'} . " UNTESTED\n";
    unless ( $this{'test_result'} eq 'FAIL' )  { 
	$this{'test_result'} = 'XFAIL';
    }
}


sub unsupported{
    my $this = shift;
    print "\nRESULT " . $this{'part_number'} . " UNSUPPORTED\n";
    unless ( $this{'test_result'} eq 'FAIL' ) { 
	$this{'test_result'} = 'XFAIL';
    }
}

sub get{ 
    my $this = shift;
    return $this{'hello'};
}

sub get_pid{
    my $this = shift;
    return $this{'pid'};
}

sub get_errmsg  {
    my $this = shift;
    return $this{'errmsg'};
}

sub get_outmsg {
    my $this = shift;
    return $this{'outmsg'};
}
    
sub set_verbiosity { 
    my $this = shift;
    my $verb = shift;
    $this{'verbiosity'} = $verb;
}

sub pass_only_on {
    my $this = shift;
    my $pattern = shift;
    if ( $this{'errorcode'} ) {
	$this->fail();
    } elsif ( $this{'outmsg'} =~ /$pattern/ ) {
	$this->pass();
	return 1;
    } elsif ( $this{'outmsg'} ) {
	$this->fail();
    } else {
	$this->unresolved();
    }
    return;
}


1; #mandatory 1 statement at end of each perl package
