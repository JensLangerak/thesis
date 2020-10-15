#ifndef WATCHER_PSEUDO_BOOLEAN_H
#define WATCHER_PSEUDO_BOOLEAN_H

#include "watched_pseudo_boolean_constraint.h"

namespace Pumpkin{

class WatcherPseudoBoolean
{
public:
	WatcherPseudoBoolean() :constraint_(NULL) {}
	WatcherPseudoBoolean(WatchedPseudoBooleanConstraint *watched_constraint) : constraint_(watched_constraint) {}
	WatcherPseudoBoolean& operator=(const WatcherPseudoBoolean &w)
	{
		this->constraint_ = w.constraint_;
		return *this;
	}

	WatchedPseudoBooleanConstraint * constraint_;
};

} //end Pumpkin namespace

#endif // !WATCHER_PSEUDO_BOOLEAN_H
