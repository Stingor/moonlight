// [Stingor] Custom map declarations
extern bool event_drop;
extern bool event_exp;

// [Stingor] Admin channel (src/map/admin.cpp). Declared here so map.cpp can
// call it from do_init()/do_final() without including a non-upstream header
// (this file is already pulled in by map.hpp), keeping core edits minimal.
void do_init_admin( void );
void do_final_admin( void );
// Output capture hook called from clif_displaymessage(): returns true (and
// swallows the line) while an admin-channel atcommand is running.
bool admin_capture_line( const char* mes );
