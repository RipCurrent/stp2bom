#include "track.h"

ROSE_IMPLEMENT_MANAGER_COMMON(uidTracker);

uidTracker * uidTracker::find(RoseObject * obj)
{
	return (uidTracker*)(obj ? obj->find_manager(type()) : 0);
}

uidTracker * uidTracker::make(RoseObject * obj)
{
	uidTracker* mgr = uidTracker::find(obj);
	if (!mgr) {
		mgr = new uidTracker;
		obj->add_manager(mgr);
	}
	return mgr;
}



ROSE_IMPLEMENT_MANAGER_COMMON(nauoTracker);

nauoTracker * nauoTracker::find(RoseObject * obj)
{
	return (nauoTracker*)(obj ? obj->find_manager(type()) : 0);
}

nauoTracker * nauoTracker::make(RoseObject * obj)
{
	nauoTracker* mgr = nauoTracker::find(obj);
	if (!mgr) {
		mgr = new nauoTracker;
		obj->add_manager(mgr);
	}
	return mgr;
}
