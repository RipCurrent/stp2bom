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