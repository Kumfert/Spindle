package petf::Auditor;

BEGIN {require 5.002;}
use Exporter;
use Benchmark;
use Config;
use FileHandle;
use strict;
use File::Basename;
use Sys::Hostname;
use POSIX qw(uname);
use Socket; #for AF_INET


use vars qw($VERSION $verbose $switches $srcdir $have_devel_corestack $curtest $DOTLENGTH
	    @ISA @EXPORT @EXPORT_OK);

$have_devel_corestack = 0;

$VERSION = "1.0";
@ISA=('Exporter');
@EXPORT= qw(&runtests);
@EXPORT_OK= qw($verbose $switches $srcdir);

format STDOUT_TOP =
     Failed Tests                     Status Wstat Total Fail  Failed  List of failed
-------------------------------------------------------------------------------------
.				
				
format STDOUT =			
^### @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @>> @>>>> @>>>> @>>> ^##.##%  @*
{$curtest->{number},
     $curtest->{name},
                $curtest->{estat},
                    $curtest->{wstat},
                          $curtest->{max},
                                $curtest->{failed},
                                     $curtest->{percent},
                                              $curtest->{canon}
}
.


$verbose = 0;
$switches = "";
$srcdir =".";

sub runtests {
    my($testsuitename) = shift;  # $testsuitename.{summary,brief,long}.log
    my(@tests) = @_;             # list of test scripts to execute
    local($|) = 1;
    local $DOTLENGTH = 60;
    my $starttime = localtime();

    ## These variables keep track of test data.
    my($n_pass_parts, $n_xfail_parts, $n_fail_parts) = (0,0,0);
    my($tot_pass_parts, $tot_xfail_parts, $tot_fail_parts) = (0,0,0);
    my($tot_pass_tests, $tot_xfail_tests, $tot_fail_tests) = (0,0,0);
    my($tot_broken_tests) = 0;
    my(@failed,%failedtests,@xfailed,%xfailedtests); ## parts in each test
    my $n_files = 0;
    my $tot_parts = 0;
    my $bindir = "";

    # pass -I flags to children
    my $old5lib = $ENV{PERL5LIB};
    local($ENV{'PERL5LIB'}) = join($Config{path_sep}, @INC);
    
    # Create temporary log files.
    my $fh_summary = IO::File->new("+> $testsuitename.summary.tmp") 
	or die "Couldn't open $testsuitename.summary.tmp for writing: $!";
    my $fh_brief = IO::File->new("+> $testsuitename.brief.tmp") 
	or die "Couldn't open $testsuitename.brief.tmp for writing: $!";
    my $fh_full = IO::File->new("+> $testsuitename.full.tmp") 
	or die "Couldn't open $testsuitename.full.tmp for writing: $!";

    #if ($^O eq 'VMS') { $switches =~ s/-(\S*[A-Z]\S*)/"-$1"/g }
    
    my $t_start = new Benchmark;
    # while there are tests to run
    my $test ="";
    my $canon = "";
    while ($test = shift(@tests)) {
	@failed = ();
	$n_files++;
	($n_pass_parts, $n_xfail_parts, $n_fail_parts) = (0,0,0);
	my $is_broken = 0;
	my $errmsg = "";
	# 1. print out the test that we are going to run
	my $ext = (fileparse( $test, '\..*?' ))[2];  #select extension so 'x.tar.gz' returns '.gz'
	my $base = basename($test);
##### GKK: dir may be $bindir instead of $srcdir
	my $dir = dirname($test);
	###if ($^O eq 'VMS') { $te =~ s/^.*\.t\./[.t./; }
        print ' ' x (3-length($n_files))."$n_files. $test" . '.' x ($DOTLENGTH -5 - length($test));
        print $fh_summary ' ' x (3-length($n_files))."$n_files. $test".'.' x ($DOTLENGTH-5-length($test));
        print $fh_brief ' ' x (3-length($n_files))."$n_files. $test" . '.' x ($DOTLENGTH-5-length($test));
        print $fh_full ' ' x (3-length($n_files))."$n_files. $test". '.' x($DOTLENGTH-5-length($test));
	# 2. verify the test exists
        my( $fh ) = new FileHandle;
	my $mylocation;
	my $mydir;
	if ( !( $fh->open("$srcdir/$test") ) ) { 
	    if ( !($fh->open("$test") ) ) { 
		$errmsg = "\t(can't open $srcdir/$test. or $test $!)";
		print "BROKEN\n$errmsg\n";
		$tot_broken_tests++;
		$is_broken = 1;
		$failedtests{$n_files} = { number => $n_files,
					   canon => "$!", max => "??",
					   failed => "??", name=> $test,
					   estat => "??", wstat=>"??",};
		#next;
	    } else { 
		$mylocation='bindir';
		$mydir="$test";
		print "\n        mylocation = bindir, mydir=$mydir\n" if $verbose > 2;
	    }
	} else { 
	    $mylocation='srcdir';
	    $mydir="$srcdir/$test";
	    print "\n        mylocation = srcdir, mydir=$mydir\n" if $verbose > 2; 
	}
	my $s = $switches;
	# 3. Analyze first line for #!/bin/perl
	if ( $ext eq ".pl" and not $is_broken ) { 
	    my $first = <$fh>;
	    $s .= q[ "-T"] if $first =~ /^#!.*\bperl.*-\w*T/;
	    if ( ! ( $fh->close) ) { 
		$errmsg = "\t(can't close $srcdir/$test. $!)";
		print "BROKEN\n$errmsg\n";
		$tot_broken_tests++;
		$is_broken = 1;
		$failedtests{$n_files} = { number => $n_files,
					canon => "cannot close", max => "??",
					failed => "??",name=> $test,
					estat => "??", wstat=>"??",};
		#next;
	    }
	}
	#3.5 Get the current working directory
	my $bindir_redirect = "";
	if ( not ( $srcdir eq "." ) ) { 
	    print "remote build detected...\n" if $verbose > 0 ;
	    # $srcdir goes from the directory that make was executed
	    # to the directory where the sources (and scripts exist)
	    # we need to create a $bindir so the scripts know where
	    # to look for any compiled binaries.
	    my $tempdir = `pwd`;
	    chop $tempdir;
	    my $olddir = $dir;

	    if ( $mylocation eq 'srcdir' ) { 
		$dir = "$srcdir/$dir";
	    } 
	    $dir =~  s/\/\.\//\//g; # replace '/./' with '/' in path
	    $dir =~ s/\/+/\//g;     # replace '//'  with '/' in path
	    my @srcdirs = split ( "/", $dir );
	    @srcdirs = @srcdirs[ 1..@srcdirs ] if $srcdirs[0] eq "" ; #prune empty front
	    @srcdirs = @srcdirs[ 0..@srcdirs-2 ] if $srcdirs[@srcdirs-1] eq "" ; #prune empty back
	    my $i = @srcdirs - 1;
	    my $count = 0;    #number of directories up
	    my @bindirs;
	    while( $i >= 0 ) { 
		if ( $srcdirs[$i] eq ".." ) {
		    $count++;
		} elsif ( $srcdirs[$i] eq "." ) { 
		    ;
		} elsif ( not ( $srcdirs[$i] eq "" ) ) { 
		    @bindirs = ( @bindirs, ".." );
		}
		--$i;
	    }
	    $tempdir =~ s/\/\.\//\//g;      # replace '/./' with '/' in path
	    $tempdir =~ s/\/+/\//g;	    # replace '//'  with '/' in path
	    my @tempdirs = split( "/", $tempdir );
	    @tempdirs = @tempdirs[ 1..@tempdirs] if $tempdirs[0] eq "" ; #prune empty front
	    @tempdirs = @tempdirs[ 0..@tempdirs-2] if $tempdirs[@tempdirs-1] eq "" ; #prune empty back
	    my $count2 = @tempdirs - 1;
	    $bindir = join( "/", ( @bindirs, @tempdirs[ $count2-$count+1..$count2] ) );
	    $bindir = "$bindir/$olddir";
	    $bindir_redirect = "--bindir=$bindir";
	    if ( $verbose > 0 ) { 
		print "directory where auditor invoked (absolute) = " . join("/", @tempdirs ) 
		    . "\n";
		print "directory where scripts are run (relative) = " . join("/", @srcdirs ) 
		    . "\n";
		print "directory for scripts to find compiled binaries = $bindir \n";
	    }
	}

	# 4. Now execute the command
	my $cmd = "";
	if ( not $is_broken ) { 
	    if ( $ext eq ".pl" ) {
		$cmd = "cd $dir; $^X $s $base $bindir_redirect |";
	    } elsif ( $ext eq ".py" ) { 
		$cmd = "cd $dir; python $s $base $bindir_redirect |";
	    } elsif ( $ext eq ".sh" ) {
		$cmd = "cd $dir; /bin/sh $s $base $bindir_redirect |";
	    } elsif ( -x $mydir ) {
		print "\n       trying to execute $base in dir $dir with args $bindir_redirect\n" if $verbose > 2;
		$cmd = "cd $dir; ./$base $s $bindir_redirect |";
	    } elsif ( -r $mydir ) { 
		$cmd = "cat $mydir |";
	    }
	}
	if ( $verbose > 0 ) { 
	    print "cmd = $cmd\n";
	}
	###if ($^O eq 'VMS') {$cmd = "MCR $cmd";}
	if ( not $is_broken ) { 
	    if ( ! ($fh->open($cmd))) { 
		$errmsg = "\t(can't run $test. $!)";
		print "BROKEN\n$errmsg\n";
		$tot_broken_tests++;
		$is_broken = 1;
		$failedtests{$n_files} = { number => $n_files,
					canon => "cannot execute", max => "??",
					failed => "??", name=> $test,
					estat => "", wstat=>""};
		#next;
	    }
	}
	if ( $is_broken ) { 
	    # couldn't even run the test, so just 
	    # finish up some output stuff and go to next test
	    my $msg = "BROKEN\n$errmsg\n";
	    print $fh_summary $msg;
	    print $fh_brief $msg;
	    print $fh_full $msg;	  
	    next;
	}
	my $fh_temp = IO::File->new_tmpfile 
	    or die "Unable to make temporary files $!";
	# 5. now copy everything from the process that we pipe
	#    to a temporary file so we can use random file access.
	while( <$fh> ) { 
	    print $fh_temp $_;
	    last if /^\s?TEST_RESULT/;
	}
	# 6. analyze how the process cleaned up after itself
	$fh->close; # must close to reap child resources.
	my $wstatus = $?;
	my $estatus;
	if ( $^O eq 'VMS' ) { 
	    eval 'use vmsish "status"; $estatus = $?';
	} else { 
	    $estatus = $wstatus >> 8;
	}
	if ( $estatus or $wstatus ) { 
	    $errmsg = "\t(abnormal termination: estatus = $estatus, wstatus = $wstatus)" ;
	    $is_broken = 1;
	    $tot_broken_tests++;
	}
	# 7. get new temp file with information redistributed
	my $fh_new = IO::File->new_tmpfile;
	seek( $fh_temp, 0, 0 );
	my $parse_error = redistribute( $fh_temp, $fh_new, $fh_summary, $fh_brief, $fh_full );
	$fh_temp->close;
	seek( $fh_new, 0, 0 );

	# 8. Parse reformatted testscript output.
#        @xfailed = ();

	### 8.a. parse the header
	my( $n_parts, $test_result ) = (0,"");
	my $buffer = "";
	while(<$fh_new>) {
	    last if ($n_parts,$test_result,$n_pass_parts,$n_xfail_parts,$n_fail_parts)
		= /NPARTS\W+(\w+)\W+(\w+)\W+(\w+)\W+(\w+)\W+(\w+)/;
	    $buffer .= $_;
	}
	if ( $test_result eq "BROKEN" and not $estatus and not $wstatus ) { 
	    $is_broken = 1;
	    $tot_broken_tests++;
#	    $estatus ="";
#	    $wstatus ="";
	}
	#$test_result = "BROKEN" if $is_broken;

	#my $msg = "$test" . '.' x ($DOTLENGTH - length($test)) . 
	my $msg = "$test_result $n_pass_parts $n_xfail_parts $n_fail_parts\n";
	$msg .= $errmsg . "\n" if $is_broken and not $parse_error;
	print $fh_summary $msg;
	print $fh_brief $msg;
	print $fh_full $msg;

	$tot_pass_tests++ if $test_result eq "PASS" and not $is_broken;
	$tot_xfail_tests++ if $test_result eq "XFAIL" and not $is_broken;
	$tot_fail_tests++ if $test_result eq "FAIL" and not  $is_broken ;

	$tot_pass_parts = $tot_pass_parts + $n_pass_parts;
	$tot_xfail_parts= $tot_xfail_parts + $n_xfail_parts;
	$tot_fail_parts = $tot_fail_parts + $n_fail_parts;

	### 8.b. parse each part of the test
	my( $part_number, $part_result);
	for( my $cur_part=1; $cur_part <= $n_parts; $cur_part++ ) { 
	    # handle stuff until next 'PART <n> <result>' declaration
	    while (<$fh_new>) {
		print $fh_full $_;
		print $fh_brief $_ if ($test_result eq "FAIL");
		print $fh_brief $_ if (($test_result eq "XFAIL") and ($_ =~ /^\s?(?:STDERR|COMMENT)/));
		last if ($part_number,$part_result) = /^\s?PART\W+(\w+)\W+(\w+)/;
	    }
	    push @failed, $part_number if  $part_result =~ /\bFAIL\b|\bXPASS\b|\bUNRESOLVED\b/;
	    $tot_parts++;

	    # handle stuff between 'PART <n> <result>' and 'RESULT <n> <result>'
	    while(<$fh_new>) {
		print $fh_full $_;
		print $fh_brief $_ if ($test_result eq "FAIL");
		print $fh_brief $_ if (($test_result eq "XFAIL") and ($_ =~ /^\s?(?:STDERR|COMMENT)/));
		last if ($part_number,$part_result) = /^\s?RESULT\W+(\w+)\W+(\w+)/;
	    }
#	    print $fh_summary "\t$part_number $part_result\n" if ($test_result eq "FAIL");
	}

	### 8.c. parse the end of the test
	while (<$fh_new>) {
	    print $fh_full $_;
	    print $fh_brief $_ if ($test_result eq "FAIL");
	    print $fh_brief $_ if (($test_result eq "XFAIL") and ($_ =~ /^\s?(?:STDERR|COMMENT)/));
	    last if ($test_result) = /^\s?TEST_RESULT\W+(\w+)/;
	}
	$test_result = "BROKEN" if $is_broken;
	if ( $test_result eq "BROKEN" ) { 
	    my ($txt, $canon ) = canonfailed( $n_parts, @failed );
	    if ($canon eq "" and $parse_error ) { 
		$canon = $parse_error;
	    }
	    my $percent = ($n_parts > 0) ? 100 * $n_fail_parts/$n_parts : ""; 
	    $failedtests{$n_files} = { number => $n_files, canon => $canon, max => $n_parts,
				       failed => $n_fail_parts, name=> $test,
				       percent => $percent,
				       estat => $estatus, wstat=> $wstatus};
	} elsif ( $test_result eq "FAIL" ) {
	    my ($txt, $canon ) = canonfailed( $n_parts, @failed );
	    my $percent = ($n_parts > 0) ? 100 * $n_fail_parts/$n_parts: ""; 
	    $failedtests{$n_files} = { number => $n_files, canon => $canon, max => $n_parts,
				    failed => $n_fail_parts, name=> $test,
				    percent => $percent,
				    estat => "", wstat=>""};
	    #next;
	}
	#next;
	printf( "%6s (%2d, %2d, %2d)\n",$test_result,$n_pass_parts, $n_xfail_parts, $n_fail_parts);
	print $errmsg. "\n" if $is_broken and not $parse_error;
	$fh_new->close;
    } #end while for all tests.
    my $t_total = timediff(new Benchmark, $t_start);
    
    # find my login
    open( TEMP , "whoami |");
    my $whoami = <TEMP>;
    chop $whoami;
    close( TEMP );

    # find my host 
    # NOTE: hostname() is problematic and may not return fully qualified name
    my $hostname = (uname)[1];
    my $address = gethostbyname($hostname);
    $hostname = gethostbyaddr($address, AF_INET);

    # create pretty message
    my $msg = "$starttime\nby $whoami\@$hostname\n";
    $msg .= sprintf("Time: %s\n", timestr($t_total, 'nop'));
    $msg .= sprintf("      %10s  %10s  %10s  %10s  %10s\n",
		    "Total","Passed","Xfailed","Failed","Broken");
    $msg .= sprintf("Tests %10d  %10d  %10d  %10d  %10d\n",
		    $n_files,$tot_pass_tests,$tot_xfail_tests,$tot_fail_tests,$tot_broken_tests);
    $msg .= sprintf("Parts %10d  %10d  %10d  %10d\n",
		    $tot_parts,$tot_pass_parts,$tot_xfail_parts,$tot_fail_parts);
    print "\n" . "*" x 80 . "\n\ntest \`$testsuitename\'\n$msg";

    #now write out the final logs
    my $fh2_summary = IO::File->new("> $testsuitename.summary.log") 
	or die "Couldn't open $testsuitename.summary.log for writing: $!";
    my $fh2_brief = IO::File->new("> $testsuitename.brief.log") 
	or die "Couldn't open $testsuitename.brief.log for writing: $!";
    my $fh2_full = IO::File->new("> $testsuitename.full.log") 
	or die "Couldn't open $testsuitename.full.log for writing: $!";

    print $fh2_summary  "test \`$testsuitename\'\n$msg\n" . "*" x80 . "\n";
    print $fh2_brief  "test \`$testsuitename\'\n$msg\n" . "*" x80 . "\n";
    print $fh2_full  "test \`$testsuitename\'\n$msg\n" . "*" x80 . "\n";

    print "\n". "*" x 80 . "\n\n";
    my $script;
    for $script (sort sort_criteria keys %failedtests) {
	$curtest = $failedtests{$script};
	if (length( $curtest->{canon}) > 25 ) { 
	    $curtest->{canon} = substr( $curtest->{canon}, 0, 25) . "...";
	}
	write;
    }
    select( $fh2_summary );
    $~ = "STDOUT";
    for $script (sort sort_criteria (keys %failedtests) ) {
	$curtest = $failedtests{$script};
	write;
    }
    select( $fh2_brief );
    $~ = "STDOUT";
    for $script (sort sort_criteria (keys %failedtests) ) {
	$curtest = $failedtests{$script};
	write;
    }
    select( $fh2_full );
    $~ = "STDOUT";
    for $script (sort sort_criteria (keys %failedtests) ) {
	$curtest = $failedtests{$script};
	write;
    }
    select( STDOUT );

    print $fh2_summary  "\n" . "*" x80 . "\n\n";
    print $fh2_brief  "\n" . "*" x80 . "\n\n";
    print $fh2_full  "\n" . "*" x80 . "\n\n";

    seek ( $fh_summary, 0, 0 );
    seek ( $fh_brief, 0, 0 );
    seek ( $fh_full, 0, 0 );
    
    while( <$fh_summary> ) { print $fh2_summary $_; }
    while( <$fh_brief> ) { print $fh2_brief $_; }
    while( <$fh_full> ) { print $fh2_full $_; }

    # cleanup resources
    $fh_summary->close;
    $fh_brief->close;
    $fh_full->close;

    $fh2_summary->close;
    $fh2_brief->close;
    $fh2_full->close;

    unlink( "$testsuitename.summary.tmp" );
    unlink( "$testsuitename.brief.tmp" );
    unlink( "$testsuitename.full.tmp" );

    if ( $tot_fail_tests > 0 or $tot_broken_tests > 0 ) {
	die "$tot_fail_tests tests failed and $tot_broken_tests tests broken.";
    } 

} #end runtests


######################################################################
#
# redistribute
#
# Reformats the output of a test script so that the results are
# available at the beginning.
#
# Takes two file handles (usually temporary files) as arguments.
#
# Also does some error checking
#
sub redistribute {
    my( $fh_old, $fh_new , $fh_summary, $fh_brief, $fh_full ) = @_;
    my $fh_temp = IO::File->new_tmpfile;
    my $debug = 0;
    my $pos = 0;
    my $retval = '';
    #
    # process the header
    #

    # repeat until we get to the NPARTS declaration
    print "step 1\n" if ($debug > 2);
    my $n_promised_tests;
    while (<$fh_old>) { 
	last if ($n_promised_tests) = /^\s?NPARTS\s(\S+)/;
	print $fh_temp $_;
    }
    my $is_broken = ( /^\s?NPARTS/ ) ? 0 : 1 ;
#    print "$n_promised_tests Parts promised\n";
#    if ( $n_promised_tests = 0 ) { 
#	$n_promised_tests = -1;
#    }
    if ( $is_broken ) { 
	my $msg = "        Invalid test: no NPARTS declaration found";
	print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	$n_promised_tests = 0;
	$is_broken = 1;
	$retval .= "NPARTS missing. ";
    }
    $pos = tell( $fh_old );
    # now skip till we find the TEST_RESULT, counting number of PARTs
    print "step 2\n" if ($debug > 2);
    my $n_tests = 0;
    my $initial_result = "";
    while (<$fh_old>) { 
	$n_tests++ if /^\s?PART /;
	last if ($initial_result) = /^\s?TEST_RESULT\W+(\w+)/;
    }
    if ( ($n_promised_tests > 0 ) and ( $n_promised_tests ne $n_tests ) ) { 
	my $msg = "        declared NPARTS $n_promised_tests but found $n_tests PART(s). Assuming $n_tests";
	print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	$is_broken = 1;
	$retval .= "NPARTS wrong. ";
    }
    if ( $n_tests eq 0 ) { 
	my $msg = "        Invalid test: no PART declarations found.";
	print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	$is_broken = 1;
	$retval .= "no PART found. ";
    }
    if ( $is_broken ) { 
	$initial_result =  "BROKEN";
    }
    print $fh_temp "\nNPARTS $n_tests $initial_result\n";
    
    #
    # process the body
    #

    # Now reset and copy everything 'til we get to first PART declaration
    print "step 3\n" if ($debug > 2);
    seek( $fh_old, $pos, 0 );
    my $skip_part = 0; # if true, skip looking for the PART section 
    my $result;
    my $final_result = ($is_broken) ? "BROKEN" : "PASS";
    my($n_passed, $n_xfailed, $n_failed) = (0,0,0);
    my( $number );
    for( my $test_number = 1; $test_number <= $n_tests; $test_number++ ) { 
	print "testnumber $test_number\n" if ($debug > 2);
	if ( not $skip_part ) { 
	    $skip_part = 0;
	    # print stuff until next 'PART <n>' declaration
	    while (<$fh_old>) {
		last if ($number) = /^\s?PART\W+(\w+)/;
		print $fh_temp $_;
	    }
	    # got the 'PART <n>'declaration
	    print "COMMENT: got 'PART $test_number' declaration\n" if ($debug > 2);
	}
	# advance to the test result
	my $buffer = "";    # string buffer 
	$result = "MISSING"; # assume result is missing, for now.
	while (<$fh_old>) { 
	    last if ($number) = /^\s?PART\W+(\w+)/; #no result for last part
	    last if ($number,$result) = /^\s?RESULT\W+(\w+)\W+(\w+)/;	
	    $buffer .= $_;
	}
	if ($_ =~ /^\s?RESULT/) { 
	    if ( $number ne $test_number ) { 
		# we've got a result for a different test
		$skip_part = 1;
		my $msg = "        (PART $test_number - RESULT $number) mismatch. Ignored";
		print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
		print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
		print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
		print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
		$result = "MISSING";
	    }
	} elsif ( $_ =~ /^\s?PART/ ) { 
	    $skip_part = 1;  #print out the next part without looking for it.
	    my $msg = "        Missing RESULT $test_number declaration. Set to UNRESOLVED";
	    print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    $result = "UNRESOLVED";
	} else { 
	    # the only other option is if there was nothing left
	    my $msg = "        Missing RESULT $test_number declaration. Set to UNRESOLVED";
	    print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    $result = "UNRESOLVED";
	}

	if (!($result =~ /\bPASS\b|\bFAIL\b|\bXFAIL\b|\bXPASS\b|\bUNTESTED\b|\bUNSUPPORTED\b|\bUNRESOLVED\b|\bMISSING\b/)) {
	    # if we don't recognize the result, then leave it unresolved.
	    my $msg;
	    if ( $result eq "MISSING" ) {
		$msg = "        Missing RESULT $test_number declaration. Set to UNRESOLVED";
	    } else {
		$msg = "        Unrecognized RESULT $test_number $result. Set to UNRESOLVED";
	    }
	    print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	    $result = "UNRESOLVED";
	}
	if ($result =~ /\bFAIL\b|\bXPASS\b|\bUNRESOLVED\b/) {
	    $final_result = "FAIL";
	}
	if ($result =~ /\bXFAIL\b|\bUNTESTED\b|\bUNSUPPORTED\b/){
	    if ( $final_result ne "FAIL") {
		$final_result = "XFAIL";
	    }
	}

	# now print out everything
	print $fh_temp "\nPART $test_number $result\n";
	print $fh_temp $buffer;
	print $fh_temp "\nRESULT $test_number $result\n";
	$n_passed++ if $result eq "PASS";
	$n_xfailed++ if $result =~ /\bXFAIL\b|\bUNTESTED\b|\bUNSUPPORTED\b/;
	$n_failed++ if $result =~ /\bFAIL\b|\bXPASS\b|\bUNRESOLVED\b/;
    } # end for $test_number < $n_tests
    
    #
    #  Process Footer
    #
    my $claimed_result = "MISSING";
    while( <$fh_old> ) { 
	last if $claimed_result = /^\s?TEST_RESULT\W+(\w+)/;
	print $fh_temp $_;
    }
    if ($claimed_result eq "MISSING" and $n_tests ne 0 ) {
	my $msg = "        Missing TEST_RESULT declaration: Setting to $final_result";
	print "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_summary "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_brief "\n$msg" . '.' x ($DOTLENGTH - length($msg));
	print $fh_full "\n$msg" . '.' x ($DOTLENGTH - length($msg));
    }
    #$final_result = "BROKEN" if $claimed_result eq "MISSING";
    print "step 4\n" if ($debug > 2);
    print $fh_temp "\nTEST_RESULT $final_result\n";

    # 
    # Now finalize the new format, stripping out any blank lines, and making
    # sure the initial_result matches the final_result.
    #   (Such discrepancies can occur when the Reporter doesn't detect 
    #    bad processes or missing results).
    #
    seek( $fh_temp, 0, 0 );
    while( <$fh_temp> ) { 
	last if /^\s?NPARTS/;
	next if ( /^\s*$/ ); # skip blank lines
	print $fh_new $_;
    }	
    print $fh_new "NPARTS $n_tests $final_result $n_passed $n_xfailed $n_failed\n";
    while( <$fh_temp> ) { 
	next if ( /^\s*$/ ); # skip blank lines
	print $fh_new $_;
    }	
    $fh_temp->close; # we're now done with the new stuff
    return $retval;

}



sub canonfailed ($@) {
    my($max,@failed) = @_;
    my %seen;
    @failed = sort {$a <=> $b} grep !$seen{$_}++, @failed;
    my $failed = @failed;
    my @result = ();
    my @canon = ();
    my $min;
    my $last = $min = shift @failed;
    my $canon;
    if (@failed) {
	for (@failed, $failed[-1]) { # don't forget the last one
	    if ($_ > $last+1 || $_ == $last) {
		if ($min == $last) {
		    push @canon, $last;
		} else {
		    push @canon, "$min-$last";
		}
		$min = $_;
	    }
	    $last = $_;
	}
	local $" = ", ";
	push @result, "FAILED tests @canon\n";
	$canon = "@canon";
    } else {
	push @result, "FAILED test $last\n";
	$canon = $last;
    }

    push @result, "\tFailed $failed/$max tests, ";
    push @result, sprintf("%.2f",100*(1-$failed/$max)), "% okay\n" if $max > 0;
    my $txt = join "", @result;
    ($txt, $canon);
}

sub sort_criteria {
    my $retval;
    my $A = $a;
    while ( length $A < 4 ) { 
	$A = "0" . $A;
    } 
    my $B = $b;
    while ( length $B < 4 ) {
	$B = "0" .$B;
    }
    if ($A gt $B) {
	$retval = 1;
    } elsif ( $A eq $B) {
	$retval = 0;
    } else {
	$retval = -1;
    }
    $retval;
}

1;

#	 if ($wstatus) {
#	     if (corestatus($wstatus)) { # until we have a wait module
#		 if ($have_devel_corestack) {
#		     Devel::CoreStack::stack($^X);
#		 } else {
#		     print "\ttest program seems to have generated a core\n";
#		 }
#	     }
#	 }


#my $tried_devel_corestack;
#sub corestatus {
#    my($st) = @_;
#    my($ret);
#
#    eval {require 'wait.ph'};
#    if ($@) {
#      SWITCH: {
#	     $ret = ($st & 0200); # Tim says, this is for 90%
#	 }
#    } else {
#	 $ret = WCOREDUMP($st);
#    }
#
#    eval { require Devel::CoreStack; $have_devel_corestack++ } 
#      unless $tried_devel_corestack++;
#
#    $ret;
#}


__END__

=head1 NAME

Test::Auditor - run test scripts and summarize results

=head1 SYNOPSIS

use Test::Auditor;

runtests( $testsuitename @tests );

=head1 DESCRIPTION

Test scripts can be written in perl C<*.pl>, python C<*.py>, shell C<*.sh>, 
or be compiled executables. The Auditor reads the standard output of the 
test scripts looking for specific symbols. 

Test::Auditor::runtests( $testscriptname, @tests) runs all the 
testscripts named as arguments and checks standard output for 
the expected strings.

After all tests have been performed, runtests() prints some
performance statistics that are computed by the Benchmark module.

=head2 The test script output

The test script begins with
C<NPARTS "<n>"> where C<"<n>"> is either the number of parts in the test
or C<-1> if the number of parts is unknown at the beginning.
All output of a single part is enclosed between C<PART "<p>"> and 
C<RESULT "<p>" "<result>"> where C<"<p>"> is the part number and 
C<"<result>"> is one of C<PASS>, C<FAIL>, C<XFAIL>, C<XPASS>, C<UNTESTED>, C<UNRESOLVED>.

It is tolerated if the test numbers after C<ok> are omitted. In this
case Test::Auditor maintains temporarily its own counter until the
script supplies test numbers again. So the following test script

    print <<END;
    NPARTS -1
    PART 1
    RESULT 1 FAIL
    PART 2 
    RESULT 2 PASS
    PART 3
    RESULT 3 XPASS
    PART 4
    RESULT 4 PASS
    PART 5
    RESULT 5 PASS
    PART 6
    RESULT 6 FAIL
    END

will generate 

    FAILED tests 1, 3, 6
    Failed 3/6 tests, 50.00% okay

Note that C<XPASS>, and C<UNRESOLVED> are considered C<FAIL>s as they all 
require the developer to intervene and correct the problem.
Similarly C<UNTESTED> and C<UNSUPPORTED> are considered C<XFAIL>s as they
are expected failures, that while not as desireable as a C<PASS>, do
not require immediate attention.

The global variable $Test::Auditor::verbose is exportable and can be
used to let runtests() display the standard output of the script
without altering the behavior otherwise.

=head1 EXPORT

C<&runtests> is exported by Test::Auditor per default.

=head1 DIAGNOSTICS
%
%=over 4
%
%=item C<All tests successful.\nFiles=%d,  Tests=%d, %s>
%
%If all tests are successful some statistics about the performance are
%printed.
%
%=item C<FAILED tests %s\n\tFailed %d/%d tests, %.2f%% okay.>
%
%For any single script that has failing subtests statistics like the
%above are printed.
%
%=item C<Test returned status %d (wstat %d)>
%
%Scripts that return a non-zero exit status, both C<$? E<gt>E<gt> 8> and C<$?> are
%printed in a message similar to the above.
%
%=item C<Failed 1 test, %.2f%% okay. %s>
%
%=item C<Failed %d/%d tests, %.2f%% okay. %s>
%
%If not all tests were successful, the script dies with one of the
%above messages.
%
=back

=head1 SEE ALSO

See L<Benchmark> for the underlying timing routines.

=head1 AUTHORS

Gary Kumfert, based largely on Test::Auditor.

=head1 BUGS

Test::Auditor uses $^X to determine the perl binary to run the tests
with. Test scripts running via the shebang (C<#!>) line may not be
portable because $^X is not consistent for shebang scripts across
platforms. This is no problem when Test::Harness is run with an
absolute path to the perl binary or when $^X can be found in the path.

=cut
