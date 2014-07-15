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
	boost::property_tree::ptree * partViewTree;
	std::string		uid;

public:

	ROSE_DECLARE_MANAGER_COMMON();

	void setUid(std::string u)		{ uid = u; }
	std::string getUid()			{ return uid; }

	int occurence = 0;
	int getOccurence()				{ return occurence; }

	void setPV(boost::property_tree::ptree* t)			{ partViewTree = t; }
	boost::property_tree::ptree* getPV()					{ return partViewTree; }

	static uidTracker* find(RoseObject * obj);
	static uidTracker* make(RoseObject * obj);
};

#endif