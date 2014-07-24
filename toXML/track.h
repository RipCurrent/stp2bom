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
	unsigned						uid=0;
	std::vector<std::string>		upperRelation; //could I just make this another uid?
	std::string						assCon;
	std::string						subAssRel;
	

public:

	ROSE_DECLARE_MANAGER_COMMON();

	bool needsSpecifiedOccurrence = false;
	stp_product_definition*			parent;

	void setUid(int u)									{ uid = u; }
	int getUid()										{ return uid; }

	void setUid(std::string u)							{ Suid = u; }
	std::string getSUid()								{ return Suid; }

	int occurence = 1;
	int ParentOccurences = 0;
	int getOccurence()									{ return occurence; }
	int getParentOccurrences()							{ return ParentOccurences; }

	void setSubRelation(std::string u)					{ subAssRel = u; }
	std::string getSubRelation()						{ return subAssRel; }

	void setUpperRelation(std::string pd)				{ upperRelation.push_back(pd); }
	void emptyUpperRelation()							{ upperRelation.empty(); }
	std::string getUpperRelation(int i)					{ return upperRelation[i]; }
	int sizeOfUpperRel()								{ return upperRelation.size(); }

	void setAssemblyContext(std::string u)				{ assCon = u; }
	std::string getAssemblyContext()					{ return assCon; }

	void setPV(boost::property_tree::ptree* t)			{ partViewTree = t; }
	boost::property_tree::ptree* getPV()				{ return partViewTree; }

	static uidTracker* find(RoseObject * obj);
	static uidTracker* make(RoseObject * obj);
};

#endif