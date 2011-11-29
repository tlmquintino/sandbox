#!/usr/bin/env perl

use strict;
use warnings;

use Person;

my $him = Person->new();

$him->name("Jason");
$him->age(23);
$him->peers( "Norbert", "Rhys", "Phineas" );

my @all_recs;
push @all_recs, $him;  # save object in array for later

printf "%s is %d years old.\n", $him->name, $him->age;
print "His peers are: ", join(", ", $him->peers), "\n";
printf "Last rec's name is %s\n", $all_recs[-1]->name;

print $him->identify(),"\n";

$him->happy_birthday();

print $him->identify(),"\n";

print $him->exclaim(),"\n";
