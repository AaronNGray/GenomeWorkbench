#!/usr/bin/env perl
use Term::ReadKey;
use MIME::Base64;
use Data::Dumper;
use JSON;
my $rc_installed = eval {
  require REST::Client;
  REST::Client->import();
  1;
};
die <<'<<END>>' unless $rc_installed;
#####
##### REST::Client perl module is not available. Please install it!
#####
##### in Linux:
#####  * add the following line to your .profile:
#####    eval $(perl -Mlocal::lib)
#####  * restart your shell session
#####  * run the following command:
#####    cpanm REST::Client
#####
##### in Windows:
#####  * ask PC-SYSTEMS to install it on your PC
#####
<<END>>

my $user;
if ($^O eq 'MSWin32') {
  require Win32;
  $user = Win32::LoginName;
}
else {
  require POSIX;
  $user = POSIX::cuserid();
}
use strict;

####
#### Reading arguments
####

my $pass;
my @rev;
my $html;

while (@ARGV) {
  my $a = shift @ARGV;
  if ($a eq '-u') {
    $user = shift @ARGV;
  }
  elsif ($a eq '-p') {
    $pass = shift @ARGV;
  }
  elsif ($a eq '-html') {
    $html = 1;
  }
  else {
    push(@rev, $a);
  }
}

if (scalar @rev < 1 || scalar @rev > 2) {
  die "##### Usage:\n\n  release-summary.pl <old-rev> [<new-rev>] [-html] [-u <user>] [-p <password>]\n\n";
}

####
#### User name and password
####

if ($user eq '') {
  print STDERR "User: ";
  $user = ReadLine(0); chomp $user;
}
if ($pass eq '') {
  ReadMode('noecho');
  print STDERR "Password for $user: ";
  $pass = ReadLine(0); chomp $pass;
  print STDERR "\n";
  ReadMode('restore');
}
my $auth = encode_base64("$user:$pass"); chomp $auth;
#print "[$user]:[$pass] [$auth]\n";

####
#### Built-in settings
####

my $jira_url = 'https://jira.ncbi.nlm.nih.gov';
my $fish_url = 'https://fisheye.ncbi.nlm.nih.gov';
my @search = (
  ['gbench', '/trunk/src/gui/packages/pkg_sequence_edit'],
  ['gbench', '/trunk/src/gui/widgets/edit'],
  ['toolkit', '/production/components/objects/23.0/src/objects/macro'],
  ['toolkit', '/production/components/objtools/23.0/src/objtools/cleanup'],
  ['toolkit', '/production/components/objtools/23.0/src/objtools/validator'],
  ['toolkit', '/production/components/misc/23.0']
);

#print Dumper(\@search);

my %jira;
my $from_date;
my $to_date;

$from_date = commit_date($rev[0]) or die "Cannot retrieve initial commit $rev[0]\n";

print STDERR "Retrieving information. Please wait...\n";

if ($rev[1] ne '') {
  $to_date = commit_date($rev[1]) or die "Cannot retrieve final commit $rev[1]\n";
}

for (my $i=0; $i < scalar @search; $i++) {
  collect_tickets($search[$i][0], $search[$i][1], $from_date, $to_date);
}

print STDERR "\n";
print $html ? "<h1>RELEASE SUMMARY</h1>\n" : "RELEASE SUMMARY\n\n";

foreach my $t(sort keys %jira) {
  my @tt = ticket_info($t);
  print $html ? "<div><a href=$jira_url/browse/$tt[0]>$tt[0]</a> [$tt[2]]: $tt[1]</div>\n" : "$tt[0] [$tt[2]]: $tt[1]\n";
}


#############################################################

sub ticket_info {
  my $ticket = shift;
  my $client = REST::Client->new();
  $client->addHeader('Accept', 'application/json');
  $client->addHeader('Authorization', "Basic $auth");
  $client->GET("$jira_url/rest/api/2/issue/$ticket");
  my $result = from_json($client->responseContent());
  my $status = $result->{fields}{status}{name};
  my $summary = $result->{fields}{summary};
  return ($ticket, $summary, $status);
}

sub commit_info {
  my $repo = shift;
  my $rev = shift;
  my $client = REST::Client->new();
  $client->addHeader('Accept', 'application/json');
  $client->addHeader('Authorization', "Basic $auth");
  $client->GET("$fish_url/rest-service-fe/revisionData-v1/changeset/$repo/$rev");
  my $result = from_json($client->responseContent());
  my $comment = $result->{comment};
  my $date = $result->{date};
  return ($date, $comment);
}

sub commit_date {
  my $rev = shift;
  my @c = commit_info('gbench', $rev);
  return $c[0] if $c[0];
  @c = commit_info('toolkit', $rev);
  return $c[0];
}

sub collect_tickets {
  my $repo = shift;
  my $path = shift;
  my $from = shift;
  my $to = shift;
  my $client = REST::Client->new();
  $client->addHeader('Accept', 'application/json');
  $client->addHeader('Authorization', "Basic $auth");
  $client->GET("$fish_url/rest-service-fe/revisionData-v1/changesetList/$repo?path=$path");
  my $result = from_json($client->responseContent());
  my $revs = $result->{csid};
  #print STDERR $#{$revs}."\n";
  for (my $i=0; $i <= $#{$revs}; $i++) {
    my @rev = commit_info($repo, $revs->[$i]);
    next if $to ne '' && $rev[0] > $to;
    last if $rev[0] <= $from;
    extract_jira($rev[1]);
#print STDERR "$revs->[$i]: $rev[1]\n";
print STDERR '.';
  }
}

sub extract_jira {
  my $str = shift;
  return unless $str =~ s/^.*\bJIRA://;
  while ($str =~ /^\s*(\w+-\d+)/) {
    $jira{$1} = 1;
    $str =~ s/^\s*(\w+-\d+)//;
  }
}
