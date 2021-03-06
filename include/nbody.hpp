#ifndef _NBODY_HPP_INCLUDED_SDOFIJ4EO8USFDLIK4T587DF98GVBIKFSD987SFDIUYVC98FDS987VXC987SDF8VXC987SFD987XVCIOUYSFD98SFD7DI
#define _NBODY_HPP_INCLUDED_SDOFIJ4EO8USFDLIK4T587DF98GVBIKFSD987SFDIUYVC98FDS987VXC987SDF8VXC987SFD987XVCIOUYSFD98SFD7DI

#include <particle.hpp>
#include <vg.hpp>

#include <vector>
#include <cstddef>
#include <iostream>

namespace feng
{

    template< typename T >
    struct nbody
    {
        typedef T                                       value_type;
        typedef particle<value_type>                    particle_type;
        typedef typename particle_type::position_type   position_type;
        typedef typename particle_type::velocity_type   velocity_type;
        typedef typename particle_type::force_type      force_type;
        typedef std::vector<particle_type>              particle_collection_type;

        particle_collection_type collection_;
        value_type x_min_, x_max_;
        value_type y_min_, y_max_;
        value_type z_min_, z_max_;

        //octree
        particle_collection_type tree_[4][4][4];

        nbody( const std::size_t n = 1000, // number of particles to generate
               const value_type x_min = -1, const value_type x_max = 1, // range x
               const value_type y_min = -1, const value_type y_max = 1, // range y
               const value_type z_min = -1, const value_type z_max = 1 )// range z
            :   x_min_( x_min ), x_max_( x_max ),
                y_min_( y_min ), y_max_( y_max ),
                z_min_( z_min ), z_max_( z_max )
        {}

        void operator()( const value_type dt = 0.1 ) //elapse time dt
        {
            build_tree();
            collection_.clear();
            update( dt );
        }

        void update( const value_type dt ) //elapse time dt
        {
            //for_each particle in collection, calculate its postions and velocities after time dt
            for ( std::size_t i = 0; i < 4; ++i )
                for ( std::size_t j = 0; j < 4; ++j )
                    for ( std::size_t k = 0; k < 4; ++k )
                        for ( std::size_t ii = 0; ii < tree_[i][j][k].size(); ++ii )
                        {
                            //calculate k1
                            particle_type& particle = tree_[i][j][k][ii];
                            force_type f = gravity( particle, i, j, k, ii );
                            const value_type m = particle.m();
                            force_type a = f/m;    
                            position_type p = particle.p();
                            velocity_type v = particle.v();
                            velocity_type p_tmp1 = dt * ( v + 0.5 * a * dt ); 
                            position_type dp1(p_tmp1.x(), p_tmp1.y(), p_tmp1.z());
                            velocity_type dv1 = dt * a;
                            particle_type p_k1( m, p + 0.5 * dp1, v + 0.5 * dv1 ); // 0.5 k1 

                            //calculate k2
                            f = gravity( p_k1, i, j, k, ii );
                            a = f/m;
                            p = p_k1.p();
                            v = p_k1.v();
                            velocity_type p_tmp2 = dt * ( v + 0.5 * a * dt ); 
                            position_type dp2(p_tmp2.x(), p_tmp2.y(), p_tmp2.z());
                            velocity_type dv2 = dt * a;
                            particle_type p_k2( m, p + 0.5 * dp2, v + 0.5 * dv2 ); // 0.5 k2
                            
                            //calculate k3
                            f = gravity( p_k1, i, j, k, ii );
                            a = f/m;
                            p = p_k2.p();
                            v = p_k2.v();
                            velocity_type p_tmp3 = dt * ( v + 0.5 * a * dt ); 
                            position_type dp3(p_tmp3.x(), p_tmp3.y(), p_tmp3.z());
                            velocity_type dv3 = dt * a;
                            particle_type p_k3( m, p + dp3, v + dv3 ); // k3 

                            //calculate k4
                            f = gravity( p_k1, i, j, k, ii );
                            a = f/m;
                            p = p_k3.p();
                            v = p_k3.v();
                            velocity_type p_tmp4 = dt * ( v + 0.5 * a * dt ); 
                            position_type dp4(p_tmp4.x(), p_tmp4.y(), p_tmp4.z());
                            velocity_type dv4 = dt * a;
                            particle_type p_k4( m, p + dp4, v + dv4 ); // k4

                            //integration
                            p = particle.p() + 0.16666666666666666666666667 * ( dp1 + dp2 + dp2 + dp3 + dp3 + dp4 );
                            v = particle.v() + 0.16666666666666666666666667 * ( dv1 + dv2 + dv2 + dv3 + dv3 + dv4 );
                           
                            //save calculation result
                            collection_.push_back( particle_type( m, p, v ) );
                        }
        }

        // input :
        // p        --      particle p
        // i j k    --      index of particle in octree tree_
        // index    --      index of particle in collection of tree_[i][j][k]
        // output:
        //          gravity force 
        const typename particle_type::force_type
        gravity( const particle_type& p, const std::size_t i, const std::size_t j, const std::size_t k, const size_t index )
        {
            typename particle_type::force_type ans(0.0, 0.0, 0.0);
            assert( i < 4 && j < 4 && k < 4 );
            for ( std::size_t ii = 0; ii < 4; ++ii )
                for ( std::size_t jj = 0; jj < 4; ++jj )
                    for ( std::size_t kk = 0; kk < 4; ++ kk )
                    {
                        if ( i == ii && j == jj && k == kk )
                            ans += near_gravity( tree_[ii][jj][kk], p, index );
                        else if ( 1 == abs(i-ii) || 1 == abs(j-jj) || 1 == abs(k-kk) )
                            ans += near_gravity( tree_[ii][jj][kk], p );
                        else
                            ans += far_gravity( tree_[ii][jj][kk], p );
                    }
            return ans;
        }

        const typename particle_type::force_type
        near_gravity( const particle_collection_type& collection, const particle_type& p, const std::size_t i ) const 
        {
            typename particle_type::force_type ans;
            for ( unsigned int ii = 0; ii < collection.size(); ++ii )
                if ( i != ii )
                    ans += p.gravity( collection[ii] );
            return ans;
        }

        const typename particle_type::force_type
        near_gravity( const particle_collection_type& collection, const particle_type& p ) const 
        {
            typename particle_type::force_type ans;

            for ( auto pp : collection )
            {
                ans += p.gravity( pp );
            }

            return ans;
        }

        const typename particle_type::force_type
        far_gravity( const particle_collection_type& collection, const particle_type& p ) const 
        {
            const typename particle_type::force_type ans = p.gravity( as_particle(collection) );
            return ans;
        }

        const particle_type
        as_particle( const particle_collection_type& collection ) const 
        {
            value_type m = 0.0;
            position_type mp( 0.0, 0.0, 0.0 );
            velocity_type mv( 0.0, 0.0, 0.0 );

            if ( collection.size() == 0 )
                return particle_type();

            for ( auto p : collection )
            {
                m += p.m();
                mp += p.m() * p.p();
                mv += p.m() * p.v();
            }

            particle_type ans( m, mp / m, mv / m );
            return ans;
        }

        const particle_type
        as_particle( ) const
        {
            return as_particle( collection_ );
        }

        void random_initialize (    const std::size_t n = 1000, //number of particles
                                    const value_type m_min = 1.0e5, const value_type m_max = 1.0e6) //mass range
                                    //const value_type m_min = 1.0e-5, const value_type m_max = 1.0e-4, //mass range
                                    //const value_type v_min = -1.0e-1, const value_type v_max = 1.0e-1 ) //velocity range
        {
            vg::variate_generator<value_type> vg;

            collection_.clear();
            collection_.reserve(n);

            const value_type square_x = ( x_max_ - x_min_ ) * ( x_max_ - x_min_ ); 
            const value_type square_y = ( y_max_ - y_min_ ) * ( y_max_ - y_min_ ); 
            const value_type square_z = ( z_max_ - z_min_ ) * ( z_max_ - z_min_ ); 
            const value_type sq = 0.25 * std::min( square_x, std::min( square_y, square_z ) );
            const position_type mid( 0.5*(x_max_-x_min_)+x_min_, 
                                     0.5*(y_max_-y_min_)+y_min_,
                                     0.5*(z_max_-z_min_)+z_min_ );

            for ( std::size_t i = 0; i < n; ++i )
            {
                particle_type p;
                p.m() = m_min + vg() * ( m_max - m_min );
                p.v().x() = value_type();
                p.v().y() = value_type();
                p.v().z() = value_type();
                //p.v().x() = v_min + vg() * ( v_max - v_min );
                //p.v().y() = v_min + vg() * ( v_max - v_min );
                //p.v().z() = v_min + vg() * ( v_max - v_min );
                do
                {
                    p.p().x() = x_min_ + vg() * ( x_max_ - x_min_ );;
                    p.p().y() = y_min_ + vg() * ( x_max_ - y_min_ );;
                    p.p().z() = z_min_ + vg() * ( x_max_ - z_min_ );;
                }
                while ( square_distance( p.p(), mid ) > sq ); 
                collection_.push_back( p );
            }
        }

        //build octree
        void build_tree()
        {
            for ( std::size_t i = 0; i < 4; ++i )
                for ( std::size_t j = 0; j < 4; ++j )
                    for ( std::size_t k = 0; k < 4; ++k )
                        { tree_[i][j][k].clear(); }

            const value_type x1 = 0.25 * ( x_min_ + x_min_ + x_min_ + x_max_ );
            const value_type x2 = 0.5 * ( x_min_ + x_max_ );
            const value_type x3 = 0.25 * ( x_min_ + x_max_ + x_max_ + x_max_ );
            const value_type y1 = 0.25 * ( y_min_ + y_min_ + y_min_ + y_max_ );
            const value_type y2 = 0.5 * ( y_min_ + y_max_ );
            const value_type y3 = 0.25 * ( y_min_ + y_max_ + y_max_ + y_max_ );
            const value_type z1 = 0.25 * ( z_min_ + z_min_ + z_min_ + z_max_ );
            const value_type z2 = 0.5 * ( z_min_ + z_max_ );
            const value_type z3 = 0.25 * ( z_min_ + z_max_ + z_max_ + z_max_ );

        for ( auto p : collection_ )
            {
                const std::size_t i =   p.p().x() < x1 ? 0 : 
                                        p.p().x() < x2 ? 1 :
                                        p.p().x() < x3 ? 2 :
                                        3;
                const std::size_t j =   p.p().y() < y1 ? 0 : 
                                        p.p().y() < y2 ? 1 :
                                        p.p().y() < y3 ? 2 :
                                        3;
                const std::size_t k =   p.p().z() < z1 ? 0 : 
                                        p.p().z() < z2 ? 1 :
                                        p.p().z() < z3 ? 2 :
                                        3;
                tree_[i][j][k].push_back( p );
            }
        }

    };

    template< typename T >
    std::ostream&
    operator << ( std::ostream& os, const nbody<T>& nb )
    {
       for ( auto p : nb.collection_ )
           os << p << "\n";
       return os;
    }

}//namespace feng

#endif//_NBODY_HPP_INCLUDED_SDOFIJ4EO8USFDLIK4T587DF98GVBIKFSD987SFDIUYVC98FDS987VXC987SDF8VXC987SFD987XVCIOUYSFD98SFD7DI

