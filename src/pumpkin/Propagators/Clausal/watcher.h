#ifndef CLAUSE_WATCHER_H
#define CLAUSE_WATCHER_H

#include "../../Basic Data Structures/boolean_literal.h"
#include "two_watched_clause.h"

namespace Pumpkin
{

class WatcherClause
{
public:
	WatcherClause() :clause_(NULL), cached_literal_(BooleanLiteral()) {}
	WatcherClause(TwoWatchedClause *watched_clause, BooleanLiteral blocking_literal) :clause_(watched_clause), cached_literal_(blocking_literal) {}
	WatcherClause& operator=(const WatcherClause &w)
	{
		this->clause_ = w.clause_;
		this->cached_literal_ = w.cached_literal_;
		return *this;
	}

	TwoWatchedClause * clause_;
	BooleanLiteral cached_literal_;
};

} //end Pumpkin namespace

#endif // !CLAUSE_WATCHER_H
