package Person;

## inspired by http://www.xav.com/perl/lib/Pod/perltoot.html

use strict;
use warnings;

use Carp;

# local packages
use Fullname;

###############################################################################
## class variables (as in c++ static member)
## global within this module
##
## anyway this is to avoid, because it messes up with inheritance

my $debugging_ = 0;
my $census_ = 0;

###############################################################################
## constructors and destructors


    # 'bimodal' constructor - can be called with:
    #     $him = Person->new()
    #     $him = $me->new()
    sub new
    {
        my $proto = shift;
        my $class = ref($proto) || $proto; # allows calling with reference
        my $self  = {};

        # "private" data
        $self->{"census_"} = \$census_;

        ++ ${ $self->{"census_"} };

        $self->{name_}   = Fullname->new();
        $self->{age_}    = undef;
        $self->{peers_}  = [];
        bless ($self, $class);             # this bless allows inheritance
        return $self;
    }

    # destructor
    sub DESTROY
    {
        my $self = shift;
        if ($debugging_) { carp "destroying $self " . $self->name }
        -- ${ $self->{"census_"} };
    }

    # class destructor
    sub END {
        if ($debugging_) {
            print "All persons are going away now.\n";
        }
    }

###############################################################################
## debugging support

    sub debug {
        my $class = shift;
        if (ref $class)  { confess "Class method called as object method" }
        unless (@_ == 1) { confess "usage: CLASSNAME->debug(level)" }
        $debugging_ = shift;
    }

###############################################################################
## public methods

    sub fullname {
        my $self = shift;
        return $self->{name_};
    }

    sub identify {
        my $self = shift;
        return sprintf "name [%s] age [%d] ", $self->name(), $self->age();
    }

    sub exclaim {
        my $self = shift;
        return sprintf "Hi, I'm %s, and I work with %s",
            $self->name, join(", ", $self->peers);
    }

    sub happy_birthday {
        my $self = shift;
        return $self->age( $self->age() + 1 );
    }

    sub population{
        my $self = shift;

        if (ref $self) {
            return ${ $self->{"census_"} };
        } else {
            return $census_;
        }
    }

###############################################################################
## methods to access per-object data
##
## With args, they set the value.  Without any, they only retrieve it/them.

sub name
{
    my $self = shift;
    return $self->{name_}->christian(@_);
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
