#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

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

    virtual Exp::Type type() const = 0;

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

/// Dispatcher of binary operations


typedef boost::tuple< Exp::Type, Exp::Type, std::string > binop_key_t;
typedef boost::function< VarPtr ( ExpPtr& , ExpPtr& ) > binop_value_t;

typedef std::map< binop_key_t, binop_value_t > binop_map_t;

static binop_map_t binop_dispatcher;

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

class ExpOp : public Exp
{

    virtual Exp::Type type() const { return Exp::OP; }
    virtual std::string opName() const = 0;

};

//-----------------------------------------------------------------------------

class Add {
public:

    static std::string class_name() { return "Add"; }

    /// Represents a Add expression
    struct Op : public ExpOp
    {
        ExpPtr lhs_;
        ExpPtr rhs_;

        Op( ExpPtr lhs, ExpPtr rhs ) : lhs_(lhs), rhs_(rhs) {}

        virtual std::string opName() const { return Add::class_name(); }

        VarPtr eval()
        {
            binop_key_t k = boost::make_tuple( lhs_->type(), rhs_->type(), opName() );

            binop_map_t::iterator itr = binop_dispatcher.find(k);
            if( itr != binop_dispatcher.end() )
                return ((*itr).second)( lhs_, rhs_ );

            assert( lhs_->isOp() || rhs_->isOp() ); // either one is an Op

            /// @todo optimize this dispatch by looking into possible reduction of temporaries

            if( lhs_->isOp() )
                lhs_ = lhs_->eval(); /// @note creates temporary

            if( rhs_->isOp() )
                rhs_ = rhs_->eval(); /// @note creates temporary

            return Op( lhs_, rhs_ ).eval();
        }
    };

    static VarPtr eval_add_scalar_scalar( ExpPtr& lhs, ExpPtr& rhs )
    {
        assert( lhs->type() == Exp::SCALAR );
        assert( rhs->type() == Exp::SCALAR );
        return maths::scalar( lhs->as<Scalar>()->value() + rhs->as<Scalar>()->value() );
    }

    static VarPtr eval_add_vector_scalar( ExpPtr& v, ExpPtr& s )
    {
        return eval_add_scalar_vector(s,v);
    }

    static VarPtr eval_add_scalar_vector( ExpPtr& s, ExpPtr& v )
    {
        assert( s->type() == Exp::SCALAR );
        assert( v->type() == Exp::VECTOR );

        scalar_t lhs = s->as<Scalar>()->value();                     /// @todo could this be a static_cast?
        Vector::storage_t& rhs = v->as<Vector>()->ref_value();       /// @todo could this be a static_cast?

        Vector* res = new Vector( rhs.size() );
        Vector::storage_t& rv = res->ref_value();

        for( size_t i = 0; i < rv.size(); ++i )
            rv[i] = lhs + rhs[i];

        return VarPtr(res);
    }

    static VarPtr eval_add_vector_vector( ExpPtr& v1, ExpPtr& v2 )
    {
        assert( v1->type() == Exp::VECTOR );
        assert( v2->type() == Exp::VECTOR );

        Vector::storage_t& lhs = v1->as<Vector>()->ref_value();
        Vector::storage_t& rhs = v2->as<Vector>()->ref_value();

        assert( lhs.size() == rhs.size() );

        Vector* res = new Vector( rhs.size() );
        Vector::storage_t& rv = res->ref_value();

        for( size_t i = 0; i < rv.size(); ++i )
            rv[i] = lhs[i] + rhs[i];

        return VarPtr(res);
    }

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

struct AddRegister
{
    AddRegister()
    {
        std::string add( Add::class_name() );
        binop_dispatcher[ boost::make_tuple( Exp::SCALAR, Exp::SCALAR, add ) ] = &(Add::eval_add_scalar_scalar);
        binop_dispatcher[ boost::make_tuple( Exp::SCALAR, Exp::VECTOR, add ) ] = &(Add::eval_add_scalar_vector);
        binop_dispatcher[ boost::make_tuple( Exp::VECTOR, Exp::SCALAR, add ) ] = &(Add::eval_add_vector_scalar);
        binop_dispatcher[ boost::make_tuple( Exp::VECTOR, Exp::VECTOR, add ) ] = &(Add::eval_add_vector_vector);
    }
};

static AddRegister add_register;

//-----------------------------------------------------------------------------

/// @todo find a way to avoid this duplication of classes

class Prod {
public:

    static std::string class_name() { return "Prod"; }

    /// Represents a Prod expression
    struct Op : public ExpOp
    {
        ExpPtr lhs_;
        ExpPtr rhs_;

        virtual std::string opName() const { return Prod::class_name(); }

        Op( ExpPtr lhs, ExpPtr rhs ) : lhs_(lhs), rhs_(rhs) {}

        VarPtr eval()
        {
            binop_key_t k = boost::make_tuple( lhs_->type(), rhs_->type(), opName() );

            binop_map_t::iterator itr = binop_dispatcher.find(k);
            if( itr != binop_dispatcher.end() )
                return ((*itr).second)( lhs_, rhs_ );

            assert( lhs_->isOp() || rhs_->isOp() ); // either one is not a Var

            /// @todo optimize this dispatch by looking into possible reduction of temporaries

            if( lhs_->isOp() )
                lhs_ = lhs_->eval(); /// @note creates temporary

            if( rhs_->isOp() )
                rhs_ = rhs_->eval(); /// @note creates temporary

            return Op( lhs_, rhs_ ).eval();
        }

    };

    static VarPtr eval_prod_scalar_scalar( ExpPtr& lhs, ExpPtr& rhs )
    {
        assert( lhs->type() == Exp::SCALAR );
        assert( rhs->type() == Exp::SCALAR );
        return maths::scalar( lhs->as<Scalar>()->value() * rhs->as<Scalar>()->value() );
    }

    static VarPtr eval_prod_scalar_vector( ExpPtr& s, ExpPtr& v )
    {
        assert( s->type() == Exp::SCALAR );
        assert( v->type() == Exp::VECTOR );

        scalar_t lhs = s->as<Scalar>()->value();
        Vector::storage_t& rhs = v->as<Vector>()->ref_value();

        Vector* res = new Vector( rhs.size() );
        Vector::storage_t& rv = res->ref_value();

        for( size_t i = 0; i < rv.size(); ++i )
            rv[i] = lhs * rhs[i];

        return VarPtr(res);
    }

    static VarPtr eval_prod_vector_scalar( ExpPtr& v, ExpPtr& s )
    {
        return eval_prod_scalar_vector(s,v);
    }

    static VarPtr eval_prod_vector_vector( ExpPtr& v1, ExpPtr& v2 )
    {
        assert( v1->type() == Exp::VECTOR );
        assert( v2->type() == Exp::VECTOR );

        Vector::storage_t& lhs = v1->as<Vector>()->ref_value();
        Vector::storage_t& rhs = v2->as<Vector>()->ref_value();

        assert( lhs.size() == rhs.size() );

        Vector* res = new Vector( rhs.size() );
        Vector::storage_t& rv = res->ref_value();

        for( size_t i = 0; i < rv.size(); ++i )
            rv[i] = lhs[i] * rhs[i];

        return VarPtr(res);
    }

    ExpPtr operator() ( ExpPtr l, ExpPtr r ) { return ExpPtr( new Prod::Op(l,r) ); }
    ExpPtr operator() ( Exp&   l, ExpPtr r ) { return ExpPtr( new Prod::Op(l.self(),r) ); }
    ExpPtr operator() ( ExpPtr l, Exp&   r ) { return ExpPtr( new Prod::Op(l,r.self()) ); }
    ExpPtr operator() ( Exp&   l, Exp&   r ) { return ExpPtr( new Prod::Op(l.self(),r.self()) ); }

};

struct ProdRegister
{
    ProdRegister()
    {
        std::string prod( Prod::class_name() );
        binop_dispatcher[ boost::make_tuple( Exp::SCALAR, Exp::SCALAR, prod ) ] = &(Prod::eval_prod_scalar_scalar);
        binop_dispatcher[ boost::make_tuple( Exp::SCALAR, Exp::VECTOR, prod ) ] = &(Prod::eval_prod_scalar_vector);
        binop_dispatcher[ boost::make_tuple( Exp::VECTOR, Exp::SCALAR, prod ) ] = &(Prod::eval_prod_vector_scalar);
        binop_dispatcher[ boost::make_tuple( Exp::VECTOR, Exp::VECTOR, prod ) ] = &(Prod::eval_prod_vector_vector);
    }
};

static ProdRegister prod_register;

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
