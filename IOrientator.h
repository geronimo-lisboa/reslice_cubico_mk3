#pragma once
#include "stdafx.h"
class IOrientator {
public:
	virtual void UpdateReslice() = 0;
	virtual ~IOrientator() {}
};