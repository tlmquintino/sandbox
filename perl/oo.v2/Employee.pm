package Employee;

use Person;

@ISA = ("Person");

    # constructor
    sub new {
        my $class = shift;
        my $self  = $class->SUPER::new();
        $self->{salary_}        = undef;
        $self->{id_}            = undef;
        bless ($self, $class); # reconsecrate
        return $self;
    }

    sub salary {
        my $self = shift;
        if (@_) { $self->{salary_} = shift }
        return $self->{salary_};
    }

    sub id_number {
        my $self = shift;
        if (@_) { $self->{id_} = shift }
        return $self->{id_};
    }

    # overridden methods (c++ virtual)
    sub peers {
        my $self = shift;
        if (@_) { @{ $self->{peers_} } = @_ }
        return map { "[$_]" } @{ $self->{peers_} };
    }

1; # Employee
