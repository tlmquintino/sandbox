#!/usr/bin/env perl

## inspired by http://perldoc.perl.org/perltoot.html

use strict;
use warnings;

# perl packages

use Data::Dumper;

# local packages

use Person;
use Employee;
use Boss;

###############################################################################
# local functions

sub END { show_census() }

sub show_census ()  {
    printf "population: %d\n", Person->population;
}

###############################################################################
# main

my @all_recs;

# Person->debug(1);

show_census();

my $him = Person->new();

push @all_recs, $him;  # save him ;)

print "\n";
#--------------------------------------
show_census();

$him->fullname->christian("Jason");
$him->fullname->surname("Meyers");
$him->fullname->nickname("El Jaso");

$him->fullname->title("Dr");

$him->age(34);

$him->peers( "Tiago", "Ralf", "Rai" );

printf "%s is really %s.\n", $him->name, $him->fullname->as_string();

print $him->name, " has peers : ", join(", ", $him->peers), "\n";

print $him->identify,"\n";

print "\n";
#--------------------------------------
show_census();

my $her = Employee->new();

push @all_recs, $her;  # save her;)

$her->name("Dory");
$her->age(19);
$her->peers( "Jenny", "Jane" );

print $her->name, " has peers : ", join(", ", $her->peers), "\n";

print "\n";
#--------------------------------------
show_census();


my $boss = Boss->new();
    $boss->fullname->title("Don");
    $boss->fullname->surname("Pichon Alvarez");
    $boss->fullname->christian("Federico Jesus");
    $boss->fullname->nickname("Fred");
    $boss->age(47);
    $boss->peers("Frank", "Felipe", "Faust");
    printf "%s is age %d.\n", $boss->fullname->as_string, $boss->age;
    printf "His peers are: %s\n", join(", ", $boss->peers);

# print Dumper($boss);

print "\n";
#--------------------------------------
show_census();
