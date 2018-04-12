#!/usr/bin/perl
use strict;

sub run( $ )
{
   my $cmd = shift;
   stdout->autoflush();
   stderr->autoflush();
   #open P,"$cmd |" or die "error running command $!";
   #my $retval = $?;
   #my @data=<p>;
   #close P;
   my $out = `$cmd`;
   my $retval = $?;

   if ($retval == -1) {
      print "F\n'$cmd' failed to execute: $!\n";
      print "output: \n" . $out . "\n";
      return -1;
   }
   elsif ($retval & 127) {
      printf "F\n'$cmd' child died with signal %d, %s coredump\n", ($retval & 127), ($retval & 128) ? 'with' : 'without';
      #print "output: \n" . $out . "\n";
      return -1;
   }
   else {
      #printf "child exited with value %d\n", $? >> 8;
      print ".";
      return 0;
   }
}

my $threshold;
sub testKoroutines( $ )
{
   my $cmd = shift;
   print "[testing $cmd]";
   my $x = 0;
   my $retval = 0;
   while ($retval == 0 && $x <= $threshold)
   {
      $x = ($x > 100000) ? $x + 100000 : ($x > 50000) ? $x + 15000 : ($x > 20000) ? $x + 5000 : (($x > 8000) ? $x + 1000 : (($x > 3000) ? $x + 100 : $x + 50));
      $retval = run( "$cmd $x 4000" );
   }
   print $retval == 0 ? "[success]\n" : "";
}
sub testMicrofiber( $ )
{
   my $cmd = shift;
   print "[testing $cmd]";
   my $retval;
   $retval = !(0 == run( "$cmd 0" ) && 0 == run( "$cmd 1" ));
   my $x = 0;
   while ($retval == 0 && $x <= $threshold)
   {
      $x = ($x > 100000) ? $x + 100000 : ($x > 50000) ? $x + 15000 : ($x > 20000) ? $x + 5000 : (($x > 8000) ? $x + 1000 : (($x > 3000) ? $x + 100 : $x + 50));
      #print "$cmd 2 $x\n";
      $retval = run( "$cmd 2 $x" );
   }
   print $retval == 0 ? "[success]\n" : "\n";
}
sub testBasic( $ )
{
   my $cmd = shift;
   print "[testing $cmd]";
   my $retval;
   $retval = !(0 == run( "$cmd" ));
   print $retval == 0 ? "[success]\n" : "\n";
}
$threshold = 200000;
testBasic( "./testYosefk" );
testKoroutines( "./testKoroutinesYosefk" );
testMicrofiber( "./testMicrofiberYosefk" );

testBasic( "./testPicoro" );
testKoroutines( "./testKoroutinesPicoro" );
testMicrofiber( "./testMicrofiberPicoro" );

testBasic( "./testUcontextPortable" );
testKoroutines( "./testKoroutinesUcontextPortable" );
testMicrofiber( "./testMicrofiberUcontextPortable" );

testBasic( "./testUcontextSystem" );
testKoroutines( "./testKoroutinesUcontextSystem" );
testMicrofiber( "./testMicrofiberUcontextSystem" );


