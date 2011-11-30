#!/usr/bin/env perl

## inspired by http://perldoc.perl.org/perltoot.html

use strict;
use warnings;

use Person;
use Employee;

sub END { show_census() }

sub show_census ()  {
    printf "population: %d\n", Person->population;
}

my @all_recs;

# Person->debug(1);

show_census();

my $him = Person->new();

show_census();

push @all_recs, $him;  # save him ;)

###################################################

$him->fullname->christian("Jason");
$him->fullname->surname("Meyers");
$him->fullname->nickname("El Jaso");

$him->fullname->title("Dr");

$him->age(34);

$him->peers( "Tiago", "Ralf", "Rai" );

printf "%s is really %s.\n", $him->name, $him->fullname->as_string();

print $him->name, " has peers : ", join(", ", $him->peers), "\n";

print $him->identify,"\n";

###################################################

my $her = Employee->new();

push @all_recs, $her;  # save her;)

$her->name("Dory");
$her->age(19);
$her->peers( "Jenny", "Jane" );

print $her->name, " has peers : ", join(", ", $her->peers), "\n";

show_census();
