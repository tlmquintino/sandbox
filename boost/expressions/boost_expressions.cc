#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace maths {

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

    enum Type { SCALAR, VECTOR, OP };

    virtual Exp::Type type() const { return OP; }

    virtual ~Exp() {}
    virtual VarPtr eval() = 0;

    ExpPtr self() { return shared_from_this(); }

    template< typename T >
    boost::shared_ptr<T> as() { return boost::dynamic_pointer_cast<T,Exp>( shared_from_this() ); }

    bool isVar() { Type t = type(); return ( t == SCALAR || t == VECTOR ); }
    bool isOp() { return type() == OP; }
    bool isScalar() { return type() == SCALAR; }
    bool isVector() { return type() == VECTOR; }

};

//-----------------------------------------------------------------------------

class Var : public Exp {
public:

    virtual VarPtr eval() { return boost::static_pointer_cast<Var>( shared_from_this() ); }

    virtual size_t size() const = 0;

    friend std::ostream& operator<<( std::ostream& os, const Var& v) { v.print(os); return os; }

    friend std::ostream& operator<<( std::ostream& os, const VarPtr& v) { v->print(os); return os; }

    virtual std::ostream& print( std::ostream& ) const = 0;
};

//-----------------------------------------------------------------------------

class Scalar : public Var {
public:

    Scalar( const scalar_t& v ) : v_(v) {}

    virtual size_t size() const { return 1; }
    virtual Exp::Type type() const { return Exp::SCALAR; }

    scalar_t value() const { return v_; }

    /// returns a reference to the scalar
    scalar_t& ref_value() { return v_; }

    virtual std::ostream& print( std::ostream& o ) const { return o << v_; }

protected:
    scalar_t v_;
};

VarPtr scalar( const scalar_t& s  ) { return VarPtr( new Scalar(s) ); }

//-----------------------------------------------------------------------------

class Vector : public Var {
public:

    typedef scalar_t value_t;
    typedef std::vector<scalar_t> storage_t;

    Vector( const size_t& s, const scalar_t& v = scalar_t() ) : v_(s,v) {}
    Vector( const storage_t& v ) : v_(v) {}

    virtual Exp::Type type() const { return Exp::VECTOR; }

//    virtual ~Vector() { std::cout << "deleting Vector" << std::endl; }

    virtual size_t size() const { return v_.size(); }

    /// returns a copy of the internal vector
    storage_t value() const { return v_; }

    /// returns a reference to the internal vector
    storage_t& ref_value() { return v_; }

    virtual std::ostream& print( std::ostream& o ) const { std::copy(v_.begin(),v_.end(), std::ostream_iterator<Vector::value_t>(o, " ")); return o; }

protected:
    storage_t v_;
};

VarPtr vector( const size_t& sz, const scalar_t& v = scalar_t()  ) { return VarPtr( new Vector(sz,v) ); }
VarPtr vector( const Vector::storage_t& v  ) { return VarPtr( new Vector(v) ); }

//-----------------------------------------------------------------------------

class Add {
public:

    /// Represents a Add expression
    struct Op : public Exp
    {
        ExpPtr lhs_;
        ExpPtr rhs_;

        Op( ExpPtr lhs, ExpPtr rhs ) : lhs_(lhs), rhs_(rhs) {}

        VarPtr eval()
        {
            bool lhs_scalar = lhs_->isScalar();
            bool lhs_vector = lhs_->isVector();
            bool rhs_scalar = rhs_->isScalar();
            bool rhs_vector = rhs_->isVector();

            if( lhs_scalar && rhs_scalar )
                return maths::scalar( lhs_->as<Scalar>()->value() + rhs_->as<Scalar>()->value() );
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

// version with stand alone functions

ExpPtr add( ExpPtr l, ExpPtr r ) { return ExpPtr( new Add::Op(l,r) ); }
ExpPtr add( Exp&   l, ExpPtr r ) { return ExpPtr( new Add::Op(l.self(),r) ); }
ExpPtr add( ExpPtr l, Exp&   r ) { return ExpPtr( new Add::Op(l,r.self()) ); }
ExpPtr add( Exp&   l, Exp&   r ) { return ExpPtr( new Add::Op(l.self(),r.self()) ); }

//-----------------------------------------------------------------------------

/// @todo find a way to avoid this duplication of classes

class Prod {
public:

    /// Represents a Prod expression
    struct Op : public Exp
    {
        ExpPtr lhs_;
        ExpPtr rhs_;

        Op( ExpPtr lhs, ExpPtr rhs ) : lhs_(lhs), rhs_(rhs) {}

        VarPtr eval()
        {
            bool lhs_scalar = lhs_->isScalar();
            bool lhs_vector = lhs_->isVector();
            bool rhs_scalar = rhs_->isScalar();
            bool rhs_vector = rhs_->isVector();

            if( lhs_scalar && rhs_scalar )
                return maths::scalar( lhs_->as<Scalar>()->value() * rhs_->as<Scalar>()->value() );
            if( lhs_scalar && rhs_vector )
                return eval_prod_vector_scalar( lhs_, rhs_ );
            if( lhs_vector && rhs_scalar )
                return eval_prod_vector_scalar( rhs_, lhs_ );
            if( lhs_vector && rhs_vector )
                return eval_prod_vector_vector( rhs_, lhs_ );

            assert( ! lhs_->isVar() || ! rhs_->isVar() ); // either one is not a Var

            /// @todo optimize this dispatch by looking into possible reduction of temporaries

            if( ! lhs_->isVar() )
                lhs_ = lhs_->eval(); /// @note creates temporary

            if( ! rhs_->isVar() )
                rhs_ = rhs_->eval(); /// @note creates temporary

            return Op( lhs_, rhs_ ).eval();
        }

    private:

        VarPtr eval_prod_vector_scalar( ExpPtr& s, ExpPtr& v )
        {
            scalar_t lhs = s->as<Scalar>()->value();
            Vector::storage_t& rhs = v->as<Vector>()->ref_value();

            Vector* res = new Vector( rhs.size() );
            Vector::storage_t& rv = res->ref_value();

            for( size_t i = 0; i < rv.size(); ++i )
                rv[i] = lhs * rhs[i];

            return VarPtr(res);
        }

        VarPtr eval_prod_vector_vector( ExpPtr& v1, ExpPtr& v2 )
        {
            Vector::storage_t& lhs = v1->as<Vector>()->ref_value();
            Vector::storage_t& rhs = v2->as<Vector>()->ref_value();

            assert( lhs.size() == rhs.size() );

            Vector* res = new Vector( rhs.size() );
            Vector::storage_t& rv = res->ref_value();

            for( size_t i = 0; i < rv.size(); ++i )
                rv[i] = lhs[i] * rhs[i];

            return VarPtr(res);
        }

    };

    Prod() {}

    ExpPtr operator() ( ExpPtr l, ExpPtr r ) { return ExpPtr( new Prod::Op(l,r) ); }
    ExpPtr operator() ( Exp&   l, ExpPtr r ) { return ExpPtr( new Prod::Op(l.self(),r) ); }
    ExpPtr operator() ( ExpPtr l, Exp&   r ) { return ExpPtr( new Prod::Op(l,r.self()) ); }
    ExpPtr operator() ( Exp&   l, Exp&   r ) { return ExpPtr( new Prod::Op(l.self(),r.self()) ); }

};

// version with stand alone functions

ExpPtr prod( ExpPtr l, ExpPtr r ) { return ExpPtr( new Prod::Op(l,r) ); }
ExpPtr prod( Exp&   l, ExpPtr r ) { return ExpPtr( new Prod::Op(l.self(),r) ); }
ExpPtr prod( ExpPtr l, Exp&   r ) { return ExpPtr( new Prod::Op(l,r.self()) ); }
ExpPtr prod( Exp&   l, Exp&   r ) { return ExpPtr( new Prod::Op(l.self(),r.self()) ); }

//-----------------------------------------------------------------------------

} // namespace maths

//-----------------------------------------------------------------------------

int main()
{
    using namespace maths;

    VarPtr a = scalar( 2. );
    VarPtr c = scalar( 4. );

    VarPtr v1 = maths::vector( 10, 5. );
    VarPtr v2 = maths::vector( 10, 7. );

    std::cout << "a = " << a << std::endl;
    std::cout << "c = " << c << std::endl;
    std::cout << "v1 = " << v1 << std::endl;
    std::cout << "v2 = " << v2 << std::endl;

    // scalar + scalar --> 6 = 2 + 4

    VarPtr r = Add()( a , c )->eval();
    std::cout << "r = " <<  r << std::endl;

    // scalar + vector --> 7 = 2 + 5

    VarPtr r2 = Add()( a , v1 )->eval();
    std::cout << "r2 = " << r2 << std::endl;

    // vector + scalar  --> 7 = 5 + 2

    std::cout << "r2 = " << Add()( v1 , a )->eval() << std::endl;

    // vector + vector --> 12 = 5 + 7

    std::cout << "r3 = " << Add()( *v1, *v2 )->eval() << std::endl;

    // vector + vector --> 10 = 5 + 5

    ExpPtr addop = Add()( v1, v1 );             // look, we can store expressions
    VarPtr r4 = addop->eval();                  // and evaluate them later...
    std::cout << "r4 = " << r4 << std::endl;

    // vector + vector --> 12 = 5 + 7

    Add addx; // or we can store the expression generators and delay generation ( usefull in worker threads )
    std::cout << "r5 = " << addx( v1, v2 )->eval() << std::endl;

    // vector + expression --> 17 = 5 + ( 5 + 7 )

    std::cout << "r6 = " << add( v1 , add( *v1, v2 ) )->eval() << std::endl;

    // chain v = v1 * ( v1 + v2 ) --> 60 = 5 * ( 5 + 7 )

    std::cout << "r7 = " << prod( v1 , add( *v1, v2 ) )->eval() << std::endl;

    // optimization : v = s * ( v1 + v2 ) --> 24 = 2 * ( 5 + 7 )

    std::cout << "r8 = " << prod( scalar(2.) , add( *v1, v2 ) )->eval() << std::endl;

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
