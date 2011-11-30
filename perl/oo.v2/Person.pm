package Person;

## inspired by http://perldoc.perl.org/perltoot.html

use strict;
use warnings;

# perl packages
use Carp;
our $AUTOLOAD;  # it's a package global

# local packages
use Fullname;

###############################################################################
## class variables

    my $debugging_ = 0;
    my $census_ = 0;

###############################################################################
## member fields

    my %fields = (
        fullname    => Fullname->new(),
        age         => undef,
        peers       => [],
    );

###############################################################################
## constructors and destructors

    # 'bimodal' constructor - can be called with:
    #     $him = Person->new()
    #     $him = $me->new()
    sub new {
        my $proto = shift;
        my $class = ref($proto) || $proto; # allows calling with reference
        my $self  = {
            _permitted => \%fields,
            %fields,
        };

        # "private" data
        $self->{"census_"} = \$census_;
        ++ ${ $self->{"census_"} };
        bless $self, $class;
        return $self;
    }

    # proxy method
    sub AUTOLOAD {
        my $self = shift;
        my $type = ref($self) or croak "$self is not an object";
        my $name = $AUTOLOAD;
        $name =~ s/.*://;   # strip fully-qualified portion
        unless (exists $self->{_permitted}->{$name} ) {
            croak "Can't access `$name' field in class $type";
        }
        if (@_) {
            return $self->{$name} = shift; # works as mutator
        } else {
            return $self->{$name};         # works as accessor
        }
    }

    # destructor
    sub DESTROY {
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
        my $self = shift;
        confess "usage: thing->debug(level)" unless @_ == 1;
        my $level = shift;
        if (ref($self))  {
            $self->{"debug_"} = $level;
        } else {
            $debugging_ = $level;            # whole class
        }
        $self->SUPER::debug($level);
    }

###############################################################################
## public methods

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
    return $self->{fullname}->christian(@_);
}


1;  # close package Person
