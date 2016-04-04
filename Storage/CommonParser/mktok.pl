#!/usr/bin/perl

my $first=1;

while (<>)
{
    my ($tokname,$tokid) = m/([^=]*)=(.*)/;
    if ($first!=1)
    {
        print ",\n";
    }
    $tokname =~ s/'(.*)'/\1/;
    $tokname =~ s/"/\\"/;
    print "std::make_pair($tokid,\"$tokname\")";
    $first=0;
}
