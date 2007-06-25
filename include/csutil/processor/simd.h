/*
Copyright (C) 2007 by Michael Gist

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "csutil/processorspecdetection.h"

namespace CS
{
    namespace SIMD
    {
        struct AltiVecType
        {
        public:
            static const int iSet = CS::Platform::ALTIVEC;
        };

        struct MMXType
        {
        public:
            static const int iSet = CS::Platform::MMX;
        };

        struct SSEType
        {
        public:
            static const int iSet = CS::Platform::SSE;
        };

        struct SSE2Type
        {
        public:
            static const int iSet = CS::Platform::SSE2;
        };

        struct SSE3Type
        {
        public:
            static const int iSet = CS::Platform::SSE3;
        };


        class SIMD
        {
        private:
            // Checks if the instruction set is available.
            static bool hasISet(int setReq);
        public:

            // TODO: Make function pointer a parameter of the template and use a class to ID SIMD type.
            // Right now, support is for 1-2 SIMD functions + Fallback, with up to 5 arguments.

            /*
            * Syntax is:
            * SIMDCheck<ReturnType, SIMDTypes, ArgumentTypes>(SIMDFunction, C++Function, Arguments);
            */

            // One SIMD

            template<typename R, class SIMDType>
            static R SIMDCheck(R (*SIMDFunction), R (*CPPFunction))
            {
                if(hasISet(SIMDType::iSet))
                    return (*SIMDFunction);
                return (*CPPFunction);
            }
            template<typename R, class SIMDType, typename ArgR>
            static R SIMDCheck(R (*SIMDFunction)(ArgR), R (*CPPFunction)(ArgR), ArgR arg)
            {
                if(hasISet(SIMDType::iSet))
                    return (*SIMDFunction)(arg);
                return (*CPPFunction)(arg);
            }
            template<typename R, class SIMDType, typename ArgR1, typename ArgR2>
            static R SIMDCheck(R (*SIMDFunction)(ArgR1, ArgR2), R (*CPPFunction)(ArgR1, ArgR2), ArgR1 arg1, ArgR2 arg2)
            {
                if(hasISet(SIMDType::iSet))
                    return (*SIMDFunction)(arg1, arg2);
                return (*CPPFunction)(arg1, arg2);
            }
            template<typename R, class SIMDType, typename ArgR1, typename ArgR2, typename ArgR3>
            static R SIMDCheck(R (*SIMDFunction)(ArgR1, ArgR2, ArgR3), R (*CPPFunction)(ArgR1, ArgR2, ArgR3), ArgR1 arg1, ArgR2 arg2, ArgR3 arg3)
            {
                if(hasISet(SIMDType::iSet))
                    return (*SIMDFunction)(arg1, arg2, arg3);
                return (*CPPFunction)(arg1, arg2, arg3);
            }
            template<typename R, class SIMDType, typename ArgR1, typename ArgR2, typename ArgR3, typename ArgR4>
            static R SIMDCheck(R (*SIMDFunction)(ArgR1, ArgR2, ArgR3, ArgR4), R (*CPPFunction)(ArgR1, ArgR2, ArgR3, ArgR4), ArgR1 arg1, ArgR2 arg2, ArgR3 arg3, ArgR4 arg4)
            {
                if(hasISet(SIMDType::iSet))
                    return (*SIMDFunction)(arg1, arg2, arg3, arg4);
                return (*CPPFunction)(arg1, arg2, arg3, arg4);
            }
            template<typename R, class SIMDType, typename ArgR1, typename ArgR2, typename ArgR3, typename ArgR4, typename ArgR5>
            static R SIMDCheck(R (*SIMDFunction)(ArgR1, ArgR2, ArgR3, ArgR4, ArgR5), R (*CPPFunction)(ArgR1, ArgR2, ArgR3, ArgR4, ArgR5), ArgR1 arg1, ArgR2 arg2, ArgR3 arg3, ArgR4 arg4, ArgR5 arg5)
            {
                if(hasISet(SIMDType::iSet))
                    return (*SIMDFunction)(arg1, arg2, arg3, arg4, arg5);
                return (*CPPFunction)(arg1, arg2, arg3, arg4, arg5);
            }

            // Two SIMD

            template<typename R, class SIMDType1, class SIMDType2>
            static R SIMDCheck(R (*SIMDFunction1), R (*SIMDFunction2), R (*CPPFunction))
            {
                if(SIMDType1::iSet < SIMDType2::iSet)
                {
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2);
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1);
                }
                else
                {
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1);
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2);
                }
                return (*CPPFunction);
            }
            template<typename R, class SIMDType1, class SIMDType2, typename ArgR>
            static R SIMDCheck(R (*SIMDFunction1)(ArgR), R (*SIMDFunction2), R (*CPPFunction)(ArgR), ArgR arg)
            {
                if(SIMDType1::iSet < SIMDType2::iSet)
                {
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg);
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg);
                }
                else
                {
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg);
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg);
                }
                return (*CPPFunction)(arg);
            }
            template<typename R, class SIMDType1, class SIMDType2, typename ArgR1, typename ArgR2>
            static R SIMDCheck(R (*SIMDFunction1)(ArgR1, ArgR2), R (*SIMDFunction2), R (*CPPFunction)(ArgR1, ArgR2), ArgR1 arg1, ArgR2 arg2)
            {
                if(SIMDType1::iSet < SIMDType2::iSet)
                {
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2);
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2);
                }
                else
                {
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2);
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2);
                }
                return (*CPPFunction)(arg1, arg2);
            }
            template<typename R, class SIMDType1, class SIMDType2, typename ArgR1, typename ArgR2, typename ArgR3>
            static R SIMDCheck(R (*SIMDFunction1)(ArgR1, ArgR2, ArgR3), R (*SIMDFunction2), R (*CPPFunction)(ArgR1, ArgR2, ArgR3), ArgR1 arg1, ArgR2 arg2, ArgR3 arg3)
            {
                if(SIMDType1::iSet < SIMDType2::iSet)
                {
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2, arg3);
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2, arg3);
                }
                else
                {
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2, arg3);
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2, arg3);
                }
                return (*CPPFunction)(arg1, arg2, arg3);
            }
            template<typename R, class SIMDType1, class SIMDType2, typename ArgR1, typename ArgR2, typename ArgR3, typename ArgR4>
            static R SIMDCheck(R (*SIMDFunction1)(ArgR1, ArgR2, ArgR3, ArgR4), R (*SIMDFunction2), R (*CPPFunction)(ArgR1, ArgR2, ArgR3, ArgR4), ArgR1 arg1, ArgR2 arg2, ArgR3 arg3, ArgR4 arg4)
            {
                if(SIMDType1::iSet < SIMDType2::iSet)
                {
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2, arg3, arg4);
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2, arg3, arg4);
                }
                else
                {
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2, arg3, arg4);
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2, arg3, arg4);
                }
                return (*CPPFunction)(arg1, arg2, arg3, arg4);
            }
            template<typename R, class SIMDType1, class SIMDType2, typename ArgR1, typename ArgR2, typename ArgR3, typename ArgR4, typename ArgR5>
            static R SIMDCheck(R (*SIMDFunction1)(ArgR1, ArgR2, ArgR3, ArgR4, ArgR5), R (*SIMDFunction2), R (*CPPFunction)(ArgR1, ArgR2, ArgR3, ArgR4, ArgR5), ArgR1 arg1, ArgR2 arg2, ArgR3 arg3, ArgR4 arg4, ArgR5 arg5)
            {
                if(SIMDType1::iSet < SIMDType2::iSet)
                {
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2, arg3, arg4, arg5);
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2, arg3, arg4, arg5);
                }
                else
                {
                    if(hasISet(SIMDType1::iSet))
                        return (*SIMDFunction1)(arg1, arg2, arg3, arg4, arg5);
                    if(hasISet(SIMDType2::iSet))
                        return (*SIMDFunction2)(arg1, arg2, arg3, arg4, arg5);
                }
                return (*CPPFunction)(arg1, arg2, arg3, arg4, arg5);
            }
        };
    }
}
