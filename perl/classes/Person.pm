package Person;

## inspired by http://www.xav.com/perl/lib/Pod/perltoot.html

use strict;
use warnings;


###############################################################################
## class variables (as in c++ static member)
## global within this module

my $Census = 0;

###############################################################################
## the object constructor
##
## can be called with
##     $him = Person->new()
##     $him = $me->new()

    sub new
    {
        my $proto = shift;
        my $class = ref($proto) || $proto; # allows calling with reference
        my $self  = {};

        $Census++;

        $self->{name_}   = undef;
        $self->{age_}    = undef;
        $self->{peers_}  = [];
        bless ($self, $class);             # this bless allows inheritance
        return $self;
    }

    # destructor
    sub DESTROY { --$Census }

###############################################################################
## public methods

    sub identify
    {
        my $self = shift;
        return sprintf "name [%s] age [%d] ", $self->name(), $self->age();
    }

    sub exclaim
    {
        my $self = shift;
        return sprintf "Hi, I'm %s, and I work with %s",
            $self->name, join(", ", $self->peers);
    }

    sub happy_birthday
    {
        my $self = shift;
        return $self->age( $self->age() + 1 );
    }

    sub population # access to class variable
    {
        return $Census;
    }

###############################################################################
## methods to access per-object data
##
## With args, they set the value.  Without any, they only retrieve it/them.

sub name
{
    my $self = shift;
    if (@_) { $self->{name_} = shift }
    return $self->{name_};
}

sub age
{
    my $self = shift;
    if (@_) { $self->{age_} = shift }
    return $self->{age_};
}

sub peers
{
    my $self = shift;
    if (@_) { @{ $self->{peers_} } = @_ }
    return @{ $self->{peers_} };
}

1;  # close package Person
