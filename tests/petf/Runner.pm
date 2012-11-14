package petf::Runner;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(init end test get get_outmsg get_errmsg get_errcode pass_only_on);

#
# using declarations go here
#
use IPC::Open3;
use IO::Select;
use IO::Handle;
use Fcntl; 
use POSIX qw(:errno_h);

#
# other stuff goes here
#

######################################################################

sub new { 
    my $type = shift; 
    my %parm = @_ ;
    my $this  = {};
    # set values here
    $this{'timeout'} = $parm{'timeout'} ;  # the amount of time to wait before giving up
    $this{'pid'} = 0;                      # the process id of the child
    $this{'xfails'} = $parm{'xfails'};     # colon separated list of test numbers that expect to fail
    $this{'nparts'} = ( $parm{'nparts'} ) ? $parm{'nparts'} : -1; #number of parts in test
#    $this{'selector'} = '';                # a selector to read from CMD_OUT or CMD_ERR
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
    my $rv = 0;
    my $verbiosity = $this{'verbiosity'};
    my $nparts = $this{'nparts'} ;

    print "\nNPARTS $nparts\n";
    $this{'errmsg'} = "" ;
    $this{'outmsg'} = "" ;

    
}

######################################################################

sub end {
    my( $this ) = shift;
    # print testfile results
    print "\nTEST_RESULT " . $this{'test_result'} . "\n";

    # just close up everything and wait for child to die.
    # waitpid( $this{'pid'}, 0 );
}

######################################################################

sub try { 
    my $this = shift;
    my $cmd = shift;
    my $input = shift;
    my $comment = shift;
    my $output ='';
    my $error ='';
    $this{'errorcode'} = 0;

#    $input = $input . ''; #helps with some apps to have an EOF character.

    $this{'errmsg'} = $error;
    $this{'outmsg'} = $output; 

    print "\nPART ". ++($this{'part_number'}) ."\n";

    if ( defined($comment) and length $comment > 0 ) { 
        my $prefix_comment = $comment;
	$prefix_comment =~ s/\n/\nCOMMENT: /g;
	print "COMMENT: ". $prefix_comment . "\n";
    }

    my $rv =''; 
    my $pid;
    my $verbiosity = $this{'verbiosity'};
   
    my $CMD_IN = IO::Handle->new;     # the file handle for the  $cmd's stdin
    my $CMD_OUT = IO::Handle->new;    # the file handle for the  $cmd's stdout
    my $CMD_ERR = IO::Handle->new;    # the file handle for the  $cmd's stderr
    
    # launch the application
    # note: open3 die's on error, so this is put in an eval block
    eval {
	$pid = open3( $CMD_IN, $CMD_OUT, $CMD_ERR, $cmd ) ;
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

    $CMD_IN -> autoflush(1);
    $CMD_OUT -> autoflush(1);
    $CMD_ERR -> autoflush(1);
    $SIG{CHLD} = sub {
	if (waitpid($pid,0) > 0  and $? != 0) {
	    print "\nCOMMENT: exit status $? on process $pid\n";
	    $this{'errorcode'} = $?;
	}
    };
    $SIG{ALRM} = sub { print "\nCOMMENT: TIMED OUT\n"  };
    
    print $CMD_IN $input . "\n";
    close( $CMD_IN );

#    waitpid( $pid, 0);
    my( @ready );
    my( $fh );

    my $prefix_input = $input;
    $prefix_input =~ s/\n/\nSTDIN: /g;
    $prefix_input = "STDIN: " . $prefix_input . "\n" ;
    $prefix_input =~ s/^STDIN:\s*$//g;
    print $prefix_input;

    sleep 1; #okay, so this is a hack.... but it works.
    $selector = IO::Select->new();
    $selector -> add( $CMD_ERR, $CMD_OUT );
    
    my $temperr='';
    # now gobble up anything up to and including the prompt.
    while ( @ready = $selector->can_read($this{'timeout'} )) {
	foreach $fh (@ready) { 
	    if ( fileno ($fh) == fileno( $CMD_ERR ) ) { 
		$temperr = <$CMD_ERR>;
		if ( $temperr =~ /\S/ ) { 
		    print "STDERR: $temperr";
		    $error = $error . $temperr;
		}
	    } else {
		my $tempout = <$CMD_OUT>;
		if ( $tempout =~ /\S/ ) {    
		    print "STDOUT: $tempout";
		    $output = $output . $tempout;
		}
	    }
	    $selector->remove ( $fh ) if eof ( $fh );
	}
    }	
    
    $this{'errmsg'} = $error;
    $this{'outmsg'} = $output;

    close( $CMD_OUT );
    close( $CMD_ERR );
    
    waitpid($pid,0);
    if ($? == -1 ) { 
	$this{'errorcode'} = 0;
    } else {
	$this{'errorcode'} = ($? << 8);
	print STDERR "\nwaitcode = " . $this{'errorcode'} ."\n;";
    }
    return 0;
}

######################################################################

sub pass {
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

sub get_errcode  {
    my $this = shift;
    return $this{'errorcode'};
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
