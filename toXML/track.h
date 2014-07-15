#ifndef _track_h_
#define _track_h_
#include <rose.h>
#include <rose_p28.h>
#include <stp_schema.h>
#include <string>

class uidTracker : public RoseManager{
private: 
	std::string		uid;
public:

	ROSE_DECLARE_MANAGER_COMMON();

	void setUid(std::string u)		{ uid = u; }
	std::string getUid()			{ return uid; }

	int occurence = 0;
	int getOccurence()				{ return occurence; }

	static uidTracker* find(RoseObject * obj);
	static uidTracker* make(RoseObject * obj);
};

#endif