#ifndef _track_h_
#define _track_h_
#include <rose.h>
#include <rose_p28.h>
#include <stp_schema.h>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

class uidTracker : public RoseManager{
private:
	boost::property_tree::ptree *	partViewTree;
	std::string						Suid;
	int								uid;

public:

	ROSE_DECLARE_MANAGER_COMMON();

	void setUid(int u)		{ uid = u; }
	int getUid()			{ return uid; }

	void setUid(std::string u)		{ Suid = u; }
	std::string getSUid()			{ return Suid; }

	int occurence = 1;
	int getOccurence()				{ return occurence; }

	void setPV(boost::property_tree::ptree* t)			{ partViewTree = t; }
	boost::property_tree::ptree* getPV()					{ return partViewTree; }

	static uidTracker* find(RoseObject * obj);
	static uidTracker* make(RoseObject * obj);
};

#endif