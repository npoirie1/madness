/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_CONDITION_FROBENIUS_HPP
#define ELEM_CONDITION_FROBENIUS_HPP

#include ELEM_INVERSE_INC
#include ELEM_FROBENIUSNORM_INC

namespace elem {

template<typename F> 
inline Base<F>
FrobeniusCondition( const Matrix<F>& A )
{
    DEBUG_ONLY(CallStackEntry cse("FrobeniusCondition"))
    typedef Base<F> Real;
    Matrix<F> B( A );
    const Real oneNorm = FrobeniusNorm( B );
    try { Inverse( B ); }
    catch( SingularMatrixException& e )
    { return std::numeric_limits<Real>::infinity(); }
    const Real oneNormInv = FrobeniusNorm( B );
    return oneNorm*oneNormInv;
}

template<typename F,Dist U,Dist V> 
inline Base<F>
FrobeniusCondition( const DistMatrix<F,U,V>& A )
{
    DEBUG_ONLY(CallStackEntry cse("FrobeniusCondition"))
    typedef Base<F> Real;
    DistMatrix<F> B( A );
    const Real oneNorm = FrobeniusNorm( B );
    try { Inverse( B ); }
    catch( SingularMatrixException& e )
    { return std::numeric_limits<Real>::infinity(); }
    const Real oneNormInv = FrobeniusNorm( B );
    return oneNorm*oneNormInv;
}

} // namespace elem

#endif // ifndef ELEM_CONDITION_FROBENIUS_HPP
