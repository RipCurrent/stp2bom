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

#include "ROSERange.h"
#pragma comment(lib,"stpcad_stix.lib")

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;

unsigned uid = 0;
unsigned id_count = 1;

void convertEntity(ptree* scope, RoseObject* ent, int currentUid){ //make it dumb so that it can be used on geomertry objects as well
	RoseObject* obj;
	auto atts = ent->attributes();
	if (!ent){ std::cout << "ent is null. WHERE IS YOUR GOD NOW! \n"; }
	for (unsigned i = 0, sz = atts->size(); i < sz; i++){
		RoseAttribute* att = atts->get(i);
		if (att->isEntity()){ //att points to another object //recurse!
			obj = ent->getObject(att);
			convertEntity(scope, obj, uid);
			//scope->add()
		}

		if (att->isAggregate()){//recurse!
			obj = ent->getObject(att);
			//std::cout << "obj size " << obj->size() << obj->domain()->name() << "\n";
			std::string derp;
			//ptree& miniScope = scope->add(att->name(),"");
			for (unsigned j = 0, sz = obj->size(); j < sz; j++){
				//std::cout << obj->getDouble(j) << "\n";
				derp += std::to_string(obj->getDouble(j)); 
				derp += " ";
				//convertEntity(&miniScope, obj, uid);
				/*for (auto k = 0u; k < atts->size(); k++){
					//if (!atts->get(k)->isSimple()){

						
						
					//}
					/*else{
						std::cout << atts->get(k)->className() << "\n";
						if (atts->get(k)->isString()){
							if (strcmp("NONE", ent->getString(atts->get(k)))){
								derp += ent->getString(atts->get(k));
								derp += " ";
							} 
						}
						else if (atts->get(k)->isInteger()){
							std::cout << ent->getInteger(atts->get(k)) << ", " << atts->get(k)->name() << "\n";
							derp += std::to_string(ent->getInteger(atts->get(k))) + " ";
						}
						else if (atts->get(k)->isDouble()){
							std::cout << ent->getDouble(atts->get(k)) << ", " << atts->get(k)->name() << "\n";
							derp += std::to_string(ent->getDouble(atts->get(k))) + " ";
						}
						else if (atts->get(k)->isFloat()){
							std::cout << ent->getFloat(atts->get(k)) << ", " << atts->get(k)->name() << "\n";
							derp += std::to_string(ent->getFloat(atts->get(k))) + " ";
						}
					}
				}*/
			}
			scope->add(att->name(), derp);
		}
		else if (att->isSelect()){//recurse!
			obj = rose_get_nested_object(ROSE_CAST(RoseUnion, ent->getObject(att)));
			std::cout << att->name() << "is select\n";
		}
		else if (att->isSimple()){//store this!
			if (att->isString()){
				if (strcmp("NONE", ent->getString(att))){
					scope->add(att->name(), ent->getString(att));
				}
			}
			else if(att->isInteger()){
				//std::cout << ent->getString(att) << ", " << att->name() << "\n";
				scope->add(att->name(), ent->getInteger(att) );
			}
			else if (att->isDouble()){
				//std::cout << ent->getString(att) << ", " << att->name() << "\n";
				scope->add(att->name(), ent->getDouble(att));
			}
			else if (att->isFloat()){
				//std::cout << ent->getString(att) << ", " << att->name() << "\n";
				scope->add(att->name(), ent->getFloat(att));
			}

		}
		//else { std::cout << att->slotDomain()->name() << "\n"; }
	}

	//obj = rose_get_nested_object(ROSE_CAST(RoseUnion, ent));

}

void exchangeContext(ptree* scope, RoseDesign* des){
	unsigned i, sz;
	RoseCursor curse;
	RoseObject* obj;

	ptree& exchange = scope->add("ExchangeContext", "");
	exchange.add("<xmlattr>.uid", "ExchangeContext--" + std::to_string(uid));
	curse.domain(ROSE_DOMAIN(stp_language));
	curse.traverse(des);
	if (curse.size() > 0){ std::cout << curse.size() << "\n"; }
	else{ exchange.add("DefaultLanguage", "English"); }
	//units
	//deal with making ref to units later

	for (i = 0, sz = des->header_description()->description()->size(); i < sz; i++){
		exchange.add("Documentation", des->header_description()->description()->get(i));
	}
	curse.domain(ROSE_DOMAIN(stp_organization));
	curse.traverse(des);
	if (curse.size() > 0){
		obj = curse.next();
		exchange.add("IdentificationContext", obj->domain()->name() + std::string("--") + std::to_string(obj->entity_id()));
	}
}



std::string handleGeometry(Workpiece * wkpc, ptree* tree){
	//returns the uid for the created geometricrepresentation  for use in PartView and storing geometry
	stp_shape_representation* srep = wkpc->get_its_geometry();
	uid++;
	int currentUid = uid;
	std::string uidForRef("gm--" + std::to_string(currentUid));
	ptree& geo = tree->add("n0:Uos.DataContainer.GeometricRepresentation", "");
	geo.add("<xmlattr>.uid", uidForRef);
	//ContextOfItems uidRef = coordinatespace(srep->context of items
	uid++;
	geo.add( srep->context_of_items()->className() + std::string(".<xmlattr>.uidRef"), "gcs--" + std::to_string(uid));
	ptree& dat = tree->add("n0:Uos.DataContainer.GeometricCoordinateSpace", "");
	dat.add("<xmlattr>.uid", "gcs--" + std::to_string(uid));
	auto gc = Geometric_context::find(srep->context_of_items());
	dat.add("DimensionCount", gc->get_dimensions());
	//handle units
	//handle getting uncertainty
	dat.add("Id.<xmlattr>.id", wkpc->get_its_id());
	
	ptree& items = geo.add("Items", "");
	for (unsigned j = 0, sz = srep->items()->size(); j < sz; j++){
		//children[j] is every RoseObject that is geometry
		RoseAttribute* att = srep->getAttribute("items");
		ptree& repItem = items.add(srep->items()->className(), "");
		uid++;
		repItem.add("<xmlattr>.uid", srep->items()->get(j) ->domain()->name() + std::string("--") + std::to_string(uid));
		repItem.add("<xmlattr>.xsi:type", std::string("n1:") + srep->items()->get(j)->domain()->name());
		convertEntity(&repItem , srep->items()->get(j), currentUid);
	}

	return uidForRef;
}

void makePart(Workpiece * wkpc, ptree* tree){
	// void makePart(stp_shape_definition_representation * sdr, ptree* tree){
	uid++;
	int currentUid = uid;
	RoseObject* obj;
	ListOfRoseObject children;
	stp_product_definition* pd;
	
	stp_shape_representation* srep = wkpc->get_its_geometry();
	pd = wkpc->getRoot();
	pd->findObjects(&children, INT_MAX, false);

	ptree& part = tree->add(std::string("n0:Uos.DataContainer.Part"), "");
	part.add("<xmlattr>.id", pd->domain()->name() + std::string("--") + std::to_string(currentUid));

	ptree& xmlObj = part.add("Id", "");
	xmlObj.add("<xmlattr>.id", wkpc->get_its_id() );

	xmlObj.add("Identifier", "");
	xmlObj.add("Identifier.<xmlattr>.uid", "pid--" + std::to_string(uid) + "--id" + std::to_string(id_count));

	xmlObj.add("Identifier.<xmlattr>.id", wkpc->get_its_id());
	xmlObj.add("Identifier.<xmlattr>.idContextRef", "create references");

	part.add("Name.CharacterString", wkpc->get_its_id());

	RoseCursor curse;
	curse.domain(ROSE_DOMAIN(stp_product_related_product_category));
	curse.traverse(pd->design());
	//RoseObject* obj;
	while (obj = curse.next()){
		stp_product_related_product_category* tmp = ROSE_CAST(stp_product_related_product_category, obj);
		if (tmp){
			//convertEntity(&part, tmp, uid);
			part.add("PartTypes.Class.<xmlattr>.uidRef", tmp->domain()->name() + std::string("--") + std::string(tmp->name()));
		}
	}
	part.add("Versions.PartVersion.Views.VersionOf", "p--" + std::to_string(currentUid));
	part.add("Versions.PartVersion.<xmlattr>.uid", "pv--" + std::to_string(currentUid) + "--id" + std::to_string(id_count));

	if (wkpc->get_revision_id() != NULL && strcmp(wkpc->get_revision_id(), "None") && strcmp(wkpc->get_revision_id(), "")){ //DO STRCMP or similarto check if none or empty
		part.add("Versions.Id.<xmlattr>.id", wkpc->get_revision_id());
	}
	else { part.add("Versions.Id.<xmlattr>.id", "/NULL"); } //replace null with a check for versioning that returns a string 

	ptree& pv = part.add("Versions.PartVersion.Views.PartView", "");
	pv.add("<xmlattr>.xsi:type", "n0:AssemblyDefinition");
	pv.add("<xmlattr>.uid", "pvv--" + std::to_string(currentUid) + "--id" + std::to_string(id_count));

	std::string geoRef = handleGeometry(wkpc, tree);	//ptree& geoRep = tree->add("n0:Uos.DataContainer.GeometricRepresentation", "");
	pv.put("DefiningGeometry.<xmlattr>.uidRef", geoRef);
	pv.add("PropertyValueAssignment.<xmlattr>.uid", "pva--" + std::to_string(currentUid));

	id_count++;
	return;
}

void copyHeader(ptree* tree, RoseDesign* master){
	unsigned i, sz;
	//place holder. ask what needs to go in header on monday
	master->initialize_header();

	ptree& head = tree->add("n0:Uos.Header", "");
	head.put("<xmlattr>.xmlns", "");
	head.put("Name", master->name());
	for (i = 0, sz = master->header_name()->author()->size(); i < sz; i++){
		head.add("Author", master->header_name()->author()->get(i));
	}
	for (i = 0, sz = master->header_name()->author()->size(); i < sz; i++){
		head.add("Organization", master->header_name()->organization()->get(i));
	}
	head.put("PreprocessorVersion", master->header_name()->preprocessor_version());
	head.put("OriginatingSystem", master->header_name()->originating_system());
	head.put("Authorization", master->header_name()->authorisation());
	for (i = 0, sz = master->header_description()->description()->size(); i < sz; i++){
		head.put("Documentation", master->header_description()->description()->get(i));
	}


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
	std::string atts[] = { "xmlns:xsi", "xmlns:n0", "xmlns:cmn", "xsi:schemaLocation" };
	ptree tree;

	tree.add("n0:Uos.<xmlattr>." + atts[0], wrapperUrls[0]);
	for (int i = 1; i < 4; i++){
		tree.put("n0:Uos.<xmlattr>." + atts[i], wrapperUrls[i]);
	}

	//Add header
	copyHeader(&tree, master);
	ptree& dat = tree.add("n0:Uos.DataContainer", "");
	exchangeContext(&dat, master);
	dat.put("<xmlattr>.xmlns", "");
	dat.add("<xmlattr>.xsi:type", "n0:AP242DataContainer");

	ARMCursor cur; //arm cursor
	ARMObject *a_obj;
	cur.domain(Workpiece::type());
	cur.traverse(master);
	ListOfRoseObject aimObjs;
	//unsigned i, sz;
	while (a_obj = cur.next()){
		std::cout << a_obj->getModuleName() << std::endl;
		makePart(a_obj->castToWorkpiece(), &tree);
	}


	for (auto &i : ROSE_RANGE(stp_shape_definition_representation, master)){
		//makePart(&i, &tree);
	}

	write_xml(std::string(master->fileDirectory() + name), tree, std::locale(), xml_writer_settings<char>(' ', 4));

	return 0;
};