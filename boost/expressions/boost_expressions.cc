#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

//-----------------------------------------------------------------------------

typedef double scalar_t;

//-----------------------------------------------------------------------------

class Var;
class Exp;

typedef boost::shared_ptr<Var>  VarPtr;
typedef boost::shared_ptr<Exp> ExpPtr;

//-----------------------------------------------------------------------------

class Exp : public boost::enable_shared_from_this<Exp>,
            private boost::noncopyable {
public:

    typedef boost::shared_ptr<Exp> Ptr;

    virtual ~Exp() {}
    virtual VarPtr eval() = 0;

    virtual bool isVar()  const { return false; }
    virtual bool scalar() const { return false; }
    virtual bool vector() const { return false; }

    ExpPtr self() { return shared_from_this(); }

    template< typename T >
    boost::shared_ptr<T> as() { return boost::dynamic_pointer_cast<T,Exp>( shared_from_this() ); }

};

//-----------------------------------------------------------------------------

class Var : public Exp {
public:

    virtual VarPtr eval() { return boost::static_pointer_cast<Var>( shared_from_this() ); }

    virtual size_t size() const = 0;

    virtual bool isVar() const { return true; }

    friend std::ostream& operator<<( std::ostream& os, const Var& v) { v.print(os); return os; }

    friend std::ostream& operator<<( std::ostream& os, const VarPtr& v) { v->print(os); return os; }

    virtual std::ostream& print( std::ostream& ) const = 0;
};

//-----------------------------------------------------------------------------

class Scalar : public Var {
public:

    Scalar( const scalar_t& v ) : v_(v) {}

    virtual size_t size() const { return 1; }
    virtual bool scalar() const { return true; }

    scalar_t value() const { return v_; }

    virtual std::ostream& print( std::ostream& o ) const { return o << v_; }

    static VarPtr make( const scalar_t& v ) { return VarPtr( new Scalar(v) ); }

protected:
    scalar_t v_;
};

//-----------------------------------------------------------------------------

class Vector : public Var {
public:

    typedef scalar_t value_t;
    typedef std::vector<scalar_t> storage_t;

    Vector( const size_t& s, const scalar_t& v = scalar_t() ) : v_(s,v) {}
    Vector( const storage_t& v ) : v_(v) {}

    virtual size_t size() const { return v_.size(); }
    virtual bool vector() const { return true; }

    /// returns a copy of the internal vector
    storage_t value() const { return v_; }

    /// returns a reference to the internal vector
    storage_t& ref_value() { return v_; }

    virtual std::ostream& print( std::ostream& o ) const { std::copy(v_.begin(),v_.end(), std::ostream_iterator<Vector::value_t>(o, " ")); return o; }

    static VarPtr make( const size_t& s, const scalar_t& v = scalar_t() ) { return VarPtr( new Vector(s,v) ); }

protected:
    storage_t v_;
};

//-----------------------------------------------------------------------------

class Add {
public:

    struct Op : public Exp
    {
        ExpPtr lhs_;
        ExpPtr rhs_;

        Op( ExpPtr lhs, ExpPtr rhs ) : lhs_(lhs), rhs_(rhs) {}

        VarPtr eval()
        {
            bool lhs_scalar = lhs_->scalar();
            bool lhs_vector = lhs_->vector();
            bool rhs_scalar = rhs_->scalar();
            bool rhs_vector = rhs_->vector();

            if( lhs_scalar && rhs_scalar )
                return Scalar::make( lhs_->as<Scalar>()->value() + rhs_->as<Scalar>()->value() );
            if( lhs_scalar && rhs_vector )
                return eval_add_vector_scalar( lhs_, rhs_ );
            if( lhs_vector && rhs_scalar )
                return eval_add_vector_scalar( rhs_, lhs_ );
            if( lhs_vector && rhs_vector )
                return eval_add_vector_vector( rhs_, lhs_ );

            assert( ! lhs_->isVar() || ! rhs_->isVar() ); // either one is not a Var

            /// @todo optimize this dispatch by looking into possible reduction of temporaries

            if( ! lhs_->isVar() )
                lhs_ = lhs_->eval(); /// @note creates temporary

            if( ! rhs_->isVar() )
                rhs_ = rhs_->eval(); /// @note creates temporary

            return Op( lhs_, rhs_ ).eval();
        }

    private:

        VarPtr eval_add_vector_scalar( ExpPtr& s, ExpPtr& v )
        {
            scalar_t lhs = s->as<Scalar>()->value();
            Vector::storage_t& rhs = v->as<Vector>()->ref_value();

            Vector* res = new Vector( rhs.size() );
            Vector::storage_t& rv = res->ref_value();

            for( size_t i = 0; i < rv.size(); ++i )
                rv[i] = lhs + rhs[i];

            return VarPtr(res);
        }

        VarPtr eval_add_vector_vector( ExpPtr& v1, ExpPtr& v2 )
        {
            Vector::storage_t& lhs = v1->as<Vector>()->ref_value();
            Vector::storage_t& rhs = v2->as<Vector>()->ref_value();

            assert( lhs.size() == rhs.size() );

            Vector* res = new Vector( rhs.size() );
            Vector::storage_t& rv = res->ref_value();

            for( size_t i = 0; i < rv.size(); ++i )
                rv[i] = lhs[i] + rhs[i];

            return VarPtr(res);
        }

    };

    Add() {}

    ExpPtr operator() ( ExpPtr l, ExpPtr r ) { return ExpPtr( new Add::Op(l,r) ); }
    ExpPtr operator() ( Exp&   l, ExpPtr r ) { return ExpPtr( new Add::Op(l.self(),r) ); }
    ExpPtr operator() ( ExpPtr l, Exp&   r ) { return ExpPtr( new Add::Op(l,r.self()) ); }
    ExpPtr operator() ( Exp&   l, Exp&   r ) { return ExpPtr( new Add::Op(l.self(),r.self()) ); }

};

//-----------------------------------------------------------------------------

int main()
{
    VarPtr a = Scalar::make( 2. );
    VarPtr c = Scalar::make( 4. );

    VarPtr v1 = Vector::make( 10, 5. );
    VarPtr v2 = Vector::make( 10, 7. );

    std::cout << "a = " << a << std::endl;
    std::cout << "c = " << c << std::endl;
    std::cout << "v1 = " << v1 << std::endl;
    std::cout << "v2 = " << v2 << std::endl;

    // scalar + scalar

    VarPtr r = Add()( a , c )->eval();
    std::cout << "r = " <<  r << std::endl;

    // scalar + vector

    VarPtr r2 = Add()( a , v1 )->eval();
    std::cout << "r2 = " << r2 << std::endl;

    // vector + scalar

    std::cout << "r2 = " << Add()( v1 , a )->eval() << std::endl;

    // vector + vector

    std::cout << "r3 = " << Add()( *v1, *v2 )->eval() << std::endl;

    // vector + vector

    ExpPtr add = Add()( v1, v1 );                // look, we can store expressions
    VarPtr r4 =  add->eval();                  // and evaluate them later...
    std::cout << "r4 = " << r4 << std::endl;

    // vector + expression

    std::cout << "r5 = " << Add()( Add()( v1, v2 ) , Add()( v1, v2 ) )->eval() << std::endl;

#if 0

// ---

    FieldSet f1 = IO::Source("lolo.grib")->eval();
    FieldSet f2 = IO::Source("polo.grib")->eval();

    FieldSet r = Add()( f1, f2 ).eval();

    Exp r = Add()( f1, f2 );

    FieldSet r2 = Regrid( '0.5/0.5' )( r )->eval();

    IO::Dump( r, "out.grib" );

// ---

    Var f1 = IO::Source("http://sdkjnfodjfskjdfbsk/grib");

    IO::Dump( Regrid( '0.5/0.5' )( f1 ), "out.grib" );

#endif

}
