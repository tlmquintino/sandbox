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

class Var : public boost::enable_shared_from_this<Var>,
            private boost::noncopyable {
public:

    typedef boost::shared_ptr<Var> Ptr;

    virtual size_t size() const = 0;
    virtual bool scalar() const { return false; }
    virtual bool vector() const { return false; }
//    virtual bool tensor() const { return false; }

    friend std::ostream& operator<<( std::ostream& os, const Var& v) { v.print(os); return os; }

    friend std::ostream& operator<<( std::ostream& os, const Var::Ptr& v) { v->print(os); return os; }

    virtual std::ostream& print( std::ostream& ) const = 0;

    template< typename T > boost::shared_ptr<T> as() { return boost::dynamic_pointer_cast<T,Var>( shared_from_this() ); }

};

//-----------------------------------------------------------------------------

class Scalar : public Var {
public:

    Scalar( const scalar_t& v ) : v_(v) {}

    virtual size_t size() const { return 1; }
    virtual bool scalar() const { return true; }

    scalar_t value() const { return v_; }

    virtual std::ostream& print( std::ostream& o ) const { return o << v_; }

    static Var::Ptr make( const scalar_t& v ) { return Var::Ptr( new Scalar(v) ); }

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

    static Var::Ptr make( const size_t& s, const scalar_t& v = scalar_t() ) { return Var::Ptr( new Vector(s,v) ); }

protected:
    storage_t v_;
};

//-----------------------------------------------------------------------------

class Expr : public boost::enable_shared_from_this<Expr>,
             private boost::noncopyable {
public:

    typedef boost::shared_ptr<Expr> Ptr;

    virtual ~Expr() {}
    virtual Var::Ptr eval() = 0;

};

//-----------------------------------------------------------------------------

class Add {
public:

    struct XpVarVar : public Expr
    {
        Var::Ptr l_;
        Var::Ptr r_;

        XpVarVar( Var::Ptr lhs, Var::Ptr rhs ) : l_(lhs), r_(rhs) {}

        Var::Ptr eval()
        {
            bool lhs_scalar = l_->scalar();
            bool rhs_scalar = r_->scalar();

            if( lhs_scalar && rhs_scalar ) // both scalar
                return Scalar::make( l_->as<Scalar>()->value() + r_->as<Scalar>()->value() );
            if( lhs_scalar ) // rhs is vector
                return eval_add_vector_scalar( l_, r_ );
            if( rhs_scalar ) // lhs is vector
                return eval_add_vector_scalar( r_, l_ );

            return eval_add_vector_vector( r_, l_ );
        }

    private:

        Var::Ptr eval_add_vector_scalar( Var::Ptr& s, Var::Ptr& v )
        {
            scalar_t lhs = s->as<Scalar>()->value();
            Vector::storage_t& rhs = v->as<Vector>()->ref_value();

            Vector* res = new Vector( rhs.size() );
            Vector::storage_t& rv = res->ref_value();

            for( size_t i = 0; i < rv.size(); ++i )
                rv[i] = lhs + rhs[i];

            return Var::Ptr(res);
        }

        Var::Ptr eval_add_vector_vector( Var::Ptr& v1, Var::Ptr& v2 )
        {
            Vector::storage_t& lhs = v1->as<Vector>()->ref_value();
            Vector::storage_t& rhs = v2->as<Vector>()->ref_value();

            assert( lhs.size() == rhs.size() );

            Vector* res = new Vector( rhs.size() );
            Vector::storage_t& rv = res->ref_value();

            for( size_t i = 0; i < rv.size(); ++i )
                rv[i] = lhs[i] + rhs[i];

            return Var::Ptr(res);
        }

    };

//    struct XpVarExpr : public Expr
//    {
//        Var::Ptr l_;
//        Expr::Ptr r_;
//        XpVarExpr( Var::Ptr lhs, Expr::Ptr rhs ) : l_(lhs), r_(rhs) {}
//        Var::Ptr eval()
//        {
//            if( l_->scalar() ) // scalar op expr
//            {
//                scalar_t lhs = s->as<Scalar>()->value();

//            }
//            else // vector op expr
//            {

//            }
//        }

//    };

    Add() {}

    Expr::Ptr operator() ( Var::Ptr l, Var::Ptr  r ) { return Expr::Ptr( new Add::XpVarVar(l,r) ); }
//    Expr::Ptr operator() ( Var::Ptr l, Expr::Ptr r ) { return Expr::Ptr( new Add::XpVarExpr(l,r) ); }

};

//-----------------------------------------------------------------------------

int main()
{
    Var::Ptr a = Scalar::make( 2. );
    Var::Ptr b = Vector::make( 10, 5. );
    Var::Ptr c = Scalar::make( 4. );

    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "c = " << c << std::endl;

    // scalar + scalar

    Var::Ptr r = Add()( a , c )->eval();
    std::cout << "r = " <<  r << std::endl;

    // scalar + vector

    Var::Ptr r2 = Add()( a , b )->eval();
    std::cout << "r2 = " << r2 << std::endl;

    // vector + scalar

    std::cout << "r2 = " << Add()( b , a )->eval() << std::endl;

    // vector + vector

    Expr::Ptr add = Add()( b, b );                // look, we can store expressions
    Var::Ptr r3 =  add->eval();                   // and evaluate them later...
    std::cout << "r3 = " << r3 << std::endl;

    // vector + expression

//    std::cout << "r4 = " << Add()( b , Add()( b, b ) ) << std::endl;

#if 0

// ---

    FieldSet f1 = IO::Source("lolo.grib")->eval();
    FieldSet f2 = IO::Source("polo.grib")->eval();

    FieldSet r = Add()( f1, f2 ).eval();

    Expr r = Add()( f1, f2 );

    FieldSet r2 = Regrid( '0.5/0.5' )( r )->eval();

    IO::Dump( r, "out.grib" );

// ---

    Var f1 = IO::Source("http://sdkjnfodjfskjdfbsk/grib");

    IO::Dump( Regrid( '0.5/0.5' )( f1 ), "out.grib" );

#endif

}
