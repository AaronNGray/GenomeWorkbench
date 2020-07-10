#ifndef GUI_MATH___PROMOTE__HPP
#define GUI_MATH___PROMOTE__HPP


#include <util/math/promote.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

//
// prmotion rules for non-builtin types
// we add these as we need them
//


//
// promote: int + CVect/CMatrix<int> --> ???
NCBI_PROMOTE2_TRAITS(int, CVect2<int>, CVect2<int>);
NCBI_PROMOTE2_TRAITS(int, CVect3<int>, CVect3<int>);
NCBI_PROMOTE2_TRAITS(int, CVect4<int>, CVect4<int>);
NCBI_PROMOTE2_TRAITS(int, CMatrix3<int>, CMatrix3<int>);
NCBI_PROMOTE2_TRAITS(int, CMatrix4<int>, CMatrix4<int>);

//
// promote: int + CVect/CMatrix<float> --> ???
NCBI_PROMOTE2_TRAITS(int, CVect2<float>, CVect2<float>);
NCBI_PROMOTE2_TRAITS(int, CVect3<float>, CVect3<float>);
NCBI_PROMOTE2_TRAITS(int, CVect4<float>, CVect4<float>);
NCBI_PROMOTE2_TRAITS(int, CMatrix3<float>, CMatrix3<float>);
NCBI_PROMOTE2_TRAITS(int, CMatrix4<float>, CMatrix4<float>);

//
// promote: float + CVect/CMatrix<int> --> ???
NCBI_PROMOTE2_TRAITS(float, CVect2<int>, CVect2<float>);
NCBI_PROMOTE2_TRAITS(float, CVect3<int>, CVect3<float>);
NCBI_PROMOTE2_TRAITS(float, CVect4<int>, CVect4<float>);
NCBI_PROMOTE2_TRAITS(float, CMatrix3<int>, CMatrix3<float>);
NCBI_PROMOTE2_TRAITS(float, CMatrix4<int>, CMatrix4<float>);

//
// promote: float + CVect/CMatrix<float> --> ???
NCBI_PROMOTE2_TRAITS(float, CVect2<float>, CVect2<float>);
NCBI_PROMOTE2_TRAITS(float, CVect3<float>, CVect3<float>);
NCBI_PROMOTE2_TRAITS(float, CVect4<float>, CVect4<float>);
NCBI_PROMOTE2_TRAITS(float, CMatrix3<float>, CMatrix3<float>);
NCBI_PROMOTE2_TRAITS(float, CMatrix4<float>, CMatrix4<float>);

//
// promote: double + CVect/CMatrix<double> --> ???
NCBI_PROMOTE2_TRAITS(double, CVect2<double>, CVect2<double>);
NCBI_PROMOTE2_TRAITS(double, CVect3<double>, CVect3<double>);
NCBI_PROMOTE2_TRAITS(double, CVect4<double>, CVect4<double>);
NCBI_PROMOTE2_TRAITS(double, CMatrix3<double>, CMatrix3<double>);
NCBI_PROMOTE2_TRAITS(double, CMatrix4<double>, CMatrix4<double>);

//
// promote: CVect/CMatrix<float> + CVect/CMatrix<float> --> ???
NCBI_PROMOTE2_TRAITS(CVect3<float>, CVect4<float>, CVect4<float>);
NCBI_PROMOTE2_TRAITS(CVect3<float>, CMatrix3<float>, CMatrix3<float>);
NCBI_PROMOTE2_TRAITS(CVect3<float>, CMatrix4<float>, CMatrix4<float>);
NCBI_PROMOTE2_TRAITS(CVect4<float>, CMatrix4<float>, CMatrix4<float>);
NCBI_PROMOTE2_TRAITS(float, CVect4< CVect3<float> >, CVect4< CVect3<float> >);
NCBI_PROMOTE2_TRAITS(float, CMatrix4< CVect3<float> >, CMatrix4< CVect3<float> >);

END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___PROMOTE__HPP
