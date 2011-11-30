package Employee;

use Person;

@ISA = ("Person");

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

    sub peers {
        my $self = shift;
        if (@_) { @{ $self->{peers_} } = @_ }
        return map { "PEON=\U$_" } @{ $self->{peers_} };
    }

1; # Employee
