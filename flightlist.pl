#!/usr/bin/perl

use DBI;
use strict;

my $dbh = DBI->connect("DBI:Pg:dbname=FMDB;host=localhost","postgres","database");

my $sth = $dbh->prepare("SELECT * from flights ORDER BY flightid");
$sth->execute();

while(my $row = $sth->fetchrow_hashref()){
    print "ID $row->{'flightid'} - $row->{'flightname'}\n";
}

$dbh->disconnect();

