package Fullname;

## inspired by http://perldoc.perl.org/perltoot.html

use strict;
use warnings;

sub new {
        my $proto = shift;
        my $class = ref($proto) || $proto;
        my $self  = {
            title_       => undef,
            christian_   => undef,
            surname_     => undef,
            nick_        => undef,
        };
        bless ($self, $class);
        return $self;
    }

sub christian {
        my $self = shift;
        if (@_) { $self->{christian_} = shift }
        return $self->{christian_};
    }

sub surname {
        my $self = shift;
        if (@_) { $self->{surname_} = shift }
        return $self->{surname_};
    }

sub nickname {
        my $self = shift;
        if (@_) { $self->{nick_} = shift }
        return $self->{nick_};
    }

sub title {
        my $self = shift;
        if (@_) { $self->{title_} = shift }
        return $self->{title_};
    }

sub as_string {
        my $self = shift;
        my $name = join(" ", @$self{'christian_', 'surname_'});
        if ($self->{title_}) {
            $name = $self->{title_} . " " . $name;
        }
        return $name;
    }

1; # end packabe Fullname
