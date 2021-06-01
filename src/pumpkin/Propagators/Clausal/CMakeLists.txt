
add_library(clausal
        clause_database.cpp  explanation_clausal.h  explanation_clausal_binary.h propagator_clausal.h propagator_clausal_binary.h  reason_clausal.h        two_watched_clause.h  watch_list.cpp
        clause_database.h    explanation_clausal.cpp explanation_clausal_binary.cpp propagator_clausal.cpp propagator_clausal_binary.cpp reason_clausal.cpp    two_watched_clause.cpp  watcher.h             watch_list.h

        )

target_link_libraries(clausal propagator_generic)