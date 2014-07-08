//Chris Made this code.
//5/20/14

#include <rose.h>
#include <rose_p28.h>
#include <stp_schema.h>
#include <stix.h>
#include <string>
#include <map>
#include <iostream>
#include <cstdio>
#include <ARM.h>
#include <ctype.h>
#include <stix_asm.h>
#include <stix_tmpobj.h>
#include <stix_property.h>
#include <stix_split.h>

#pragma comment(lib,"stpcad_stix.lib")
 
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;

unsigned uid = 0;
unsigned id_count = 1;

void makePart(ARMObject* a_obj, ptree* tree){
	uid++;
	ListOfRoseObject aimObjs;
	a_obj->getAIMObjects(&aimObjs);
	std::cout << a_obj->getRootObject()->domain()->name() << " is root \n";
	RoseObject* obj = a_obj->getRootObject();
	if (obj->domain() != ROSE_DOMAIN(stp_product_definition)){
		return;
	}
	stp_product_definition* pd = ROSE_CAST(stp_product_definition, obj);
	stp_product_definition_formation * pdf = pd->formation();
	stp_product * p = pdf ? pdf->of_product() : 0;
	if (!p)return;
	std::cout << p->name() << "\n";
	ptree& part = tree->add(std::string("n0:Uos.DataContainer.Part") , "");
	part.add("<xmlattr>.Uid", "p--" + std::to_string(uid));
	ptree& xmlObj = part.add("Id", "");
	xmlObj.add("<xmlattr>.id", p->name());
	
	xmlObj.add("Identifier", "");
	xmlObj.add("Identifier.<xmlattr>.uid", "pid--" + std::to_string(uid) + "--id" + std::to_string(id_count));

	xmlObj.add("Identifier.<xmlattr>.id", p->name());
	xmlObj.add("Identifier.<xmlattr>.idContextRef", "create references");

	part.add("Name.CharacterString", p->name());

	RoseCursor curse;
	curse.domain(ROSE_DOMAIN(stp_product_related_product_category));
	curse.traverse(obj->design());
	//RoseObject* obj;
	while (obj = curse.next()){
		stp_product_related_product_category* tmp = ROSE_CAST(stp_product_related_product_category, obj);
		if (tmp){
			part.add("PartTypes.Class.<xmlattr>.uidRef", "pca--" + std::string(tmp->name()) );
		}
	}
	
	part.add("Versions.PartVersion.<xmlattr>.uid", "pv--" + std::to_string(uid) + "--id" + std::to_string(id_count));

	ptree& pv = part.add("Versions.PartVersion.Views.PartView", "");
	pv.add("<xmlattr>.xsi:type", "n0:AssemblyDefinition");
	pv.add("<xmlattr>.uid", "pvv--" + std::to_string(uid) + "--id" + std::to_string(id_count));
	id_count++;


	return;
}

void copyHeader(ptree* tree, RoseDesign* master){
	//place holder. ask what needs to go in header on monday
	ptree& head = tree->add("n0:Uos.Header", "");
	head.put("<xmlattr>.xmlns", "");
	head.put("Name", master->name());
	head.put("Documentation", "this is a beta xml converter for non-geometry data");

}

int main(int argc, char* argv[]){
	if (argc < 2){
		std::cout << "Usage: .\\STEPSplit.exe filetosplit.stp\n" << "\tCreates new file SplitOutput.stp as master step file with seperate files for each product" << std::endl;
		return EXIT_FAILURE;
	}
	ROSE.quiet(1);	//Suppress startup info.
	stplib_init();	// initialize merged cad library
	//    rose_p28_init();	// support xml read/write
	FILE *out;
	out = fopen("log.txt", "w");
	//ROSE.error_reporter()->error_file(out);
	RoseP21Writer::max_spec_version(PART21_ED3);	//We need to use Part21 Edition 3 otherwise references won't be handled properly.
	/* Create a RoseDesign to hold the output data*/
	std::string infilename(argv[1]);
	if (NULL == rose_dirname(infilename.c_str()))	//Check if there's already a path on the input file. If not, add '.\' AKA the local directory.
	{
		infilename = ".\\" + infilename;
	}
	if (!rose_file_readable(infilename.c_str()))	//Make sure file is readable before we open it.
	{
		std::cout << "Error reading input file." << std::endl;
		return EXIT_FAILURE;
	}
	RoseDesign * master = ROSE.useDesign(infilename.c_str());
	stix_tag_units(master);
	ARMpopulate(master);

	std::string name = "test.xml";
	std::string wrapperUrls[] = { "http://www.w3.org/2001/XMLSchema-instance", "http://standards.iso.org/iso/ts/10303/-3001/-ed-1/tech/xml-schema/bo_model", "http://standards.iso.org/iso/ts/10303/-3000/-ed-1/tech/xml-schema/common", "http://standards.iso.org/iso/ts/10303/-3001/-ed-1/tech/xml-schema/bo_model AP242_BusinessObjectModel.xsd" };
	std::string atts[] = {"xmlns:xsi","xmlns:n0","xmlns:cmn","xsi:schemaLocation" };
	ptree tree;

	tree.add("n0:Uos.<xmlattr>."+atts[0], wrapperUrls[0]);
	for (int i = 1; i < 4; i++){
		tree.put("n0:Uos.<xmlattr>." + atts[i], wrapperUrls[i]);
	}
	
	//Add header
	copyHeader(&tree, master);
	ptree& dat = tree.add("n0:Uos.DataContainer","");
	dat.put("<xmlattr>.xmlns", "");
	dat.add("<xmlattr>.xsi:type", "n0:AP242DataContainer");
	ARMCursor cur; //arm cursor
	ARMObject *a_obj;
	cur.domain(Workpiece::type());
	cur.traverse(master);
	ListOfRoseObject aimObjs;
	unsigned i, sz;
	while (a_obj = cur.next()){
	
		aimObjs.emptyYourself();	
		std::cout << a_obj->getModuleName() << std::endl;
		//a_obj->getAIMObjects(&aimObjs);

		makePart(a_obj, &tree);


		
		
	}
	write_xml(std::string(master->fileDirectory() + name), tree, std::locale(), xml_writer_settings<char>(' ', 4));
	
	return 0;
};