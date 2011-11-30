package Employee;

use strict;
use warnings;
use Person;

my %fields = (
    id          => undef,
    salary      => undef,
);

our @ISA = ("Person");

    # constructor
    sub new {
        my $class = shift;
        my $self  = $class->SUPER::new();
        my($element);
        foreach $element (keys %fields) { # place subclass fields into parent class
            $self->{_permitted}->{$element} = $fields{$element};
        }
        @{$self}{keys %fields} = values %fields;
        return $self;
    }

    # overridden methods (c++ virtual)
    sub peers {
        my $self = shift;
        if (@_) { @{ $self->{peers} } = @_ }
        return map { "[$_]" } @{ $self->{peers} };
    }

1; # Employee
