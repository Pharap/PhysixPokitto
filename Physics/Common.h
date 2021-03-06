/*
   Copyright (C) 2018 Pharap (@Pharap)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <cstdint>

#define FIXED_POINTS_NO_RANDOM
#include "FixedPoints.h"

// ToDo: try using a larger type on the Pokitto
using Number = SFixed<15, 16>;
using NumberU = UFixed<16, 16>;

constexpr inline NumberU fromSigned(Number value)
{
	return NumberU::fromInternal(value.getInternal());
}

constexpr inline Number fromUnsigned(NumberU value)
{
	return Number::fromInternal(value.getInternal());
}

template< typename T >
constexpr auto square(T value) -> decltype(value * value)
{
	return value * value;
}

template< typename T, size_t size >
constexpr size_t arrayLength(T (&)[size])
{
	return size;
}

template< typename T >
constexpr size_t arrayLength(T (&)[0])
{
	return 0;
}
