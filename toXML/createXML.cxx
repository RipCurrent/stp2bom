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
#include <ctype.h>
#include <stix_asm.h>
#include <stix_tmpobj.h>
#include <stix_property.h>
#include <stix_split.h>

#include "track.h"
#include "ROSERange.h"
#pragma comment(lib,"stpcad_stix.lib")

using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;

unsigned uid = 0;
unsigned id_count = 1;

const std::string prefixes[] = { "exa", "peta", "tera", "giga", "mega", "kilo", "hecto", "deca", "deci", "centi", "milli", "micro", "nano", "pico", "femto", "atto" };
const std::string unit_names[] = { "metre",	"gram",	"second",	"ampere",	"kelvin",	"mole",	"candela",	"radian",	"steradian",	"hertz",	"newton",	"pascal",	"joule",	"watt",	"coulomb",	"volt",	"farad",	"ohm",	"siemens",	"weber",	"tesla",	"henry",	"degree_celsius",	"lumen",	"lux",	"becquerel",	"gray",	"sievert" };

void do_nauos(stp_product_definition* pd, ptree* pv, int currentUid);
void add_simple(ptree* scope, RoseAttribute* att, RoseObject* ent, std::string name);

void convertEntity(ptree* scope, RoseObject* ent, int currentUid, std::string name = ""){ //make it dumb so that it can be used on geomertry objects as well
	RoseObject* obj = ent;
	auto atts = ent->attributes();
	if (!ent){ std::cout << "ent is null. WHERE IS YOUR GOD NOW! \n"; }
	for (unsigned i = 0, sz = atts->size(); i < sz; i++){
		RoseAttribute* att = atts->get(i);
		if (att->isEntity()){ //att points to another object //recurse!
			obj = ent->getObject(att);
			convertEntity(scope, obj, uid, att->name());
		}

		if (att->isAggregate()){//recurse!
			obj = ent->getObject(att);
			std::string derp;
			if (obj->attributes()->get(0)->isEntity()){
				std::cout << "Aggregate has multiple levels of attributes " << obj->domain()->name() << "\n"; //currently this makes it ignore manifold solid brep and all of its possible children
			}
			else{
				for (unsigned j = 0, sz = obj->size(); j < sz; j++){
					if (obj->domain() == ROSE_DOMAIN(ListOfint)){
						derp += std::to_string(obj->getInteger(j));
					}
					else if (obj->domain() == ROSE_DOMAIN(ListOfdouble)){
						derp += std::to_string(obj->getDouble(j));
					}
					else if (obj->domain() == ROSE_DOMAIN(ListOffloat)){
						derp += std::to_string(obj->getFloat(j));
					}
					derp += " ";
				}
				if (name.size() > 0){
					if (strcmp("location", name.c_str())){
						scope->add(name, derp);
					}
					else{
						scope->add("postion", derp);
					}
				}
				else{
					scope->add(att->name(), derp);
				}
			}
		}
		else if (att->isSelect()){//recurse!
			obj = ent->getObject(att);
			if (obj){ 
				if (obj->domain() == ROSE_DOMAIN(stp_measure_value)){
					auto tmp = ROSE_CAST(stp_measure_value, obj);
					scope->add(ent->getString((ent->getAttribute("name"))), tmp->_length_measure());
				}
			}
		}
		else if (att->isSimple()){//store this!
			add_simple(scope, att, ent, name);
		}
	}
}

void add_simple(ptree* scope, RoseAttribute* att, RoseObject* ent, std::string name){
	if (att->isString()){
		if (strcmp("NONE", ent->getString(att)) && strcmp("", ent->getString(att))  ){
			if (name.size() > 0){
				if (strcmp("uncertainty_measure_with_unit", name.c_str())){
					scope->add(name, ent->getString(att));
				}
			}
			else{
				scope->add(att->name(), ent->getString(att));
			}
		}
	}
	else if (att->isInteger()){
		if (name.size() > 0){
			scope->add(name, ent->getInteger(att));
		}
		else{
			scope->add(att->name(), ent->getInteger(att));
		}
	}
	else if (att->isDouble()){
		if (name.size() > 0){
			scope->add(name, ent->getDouble(att));
		}
		else{
			scope->add(att->name(), ent->getDouble(att));
		}
	}
	else if (att->isFloat()){
		if (name.size() > 0){
			scope->add(name, ent->getFloat(att));
		}
		else{
			scope->add(att->name(), ent->getFloat(att));
		}
	}
}

void exchangeContext(ptree* scope, RoseDesign* des){
	unsigned i, sz;
	RoseCursor curse;
	RoseObject* obj;

	ptree& exchange = scope->add("ExchangeContext", "");
	exchange.add("<xmlattr>.uid", "ExchangeContext--" + std::to_string(uid));
	curse.domain(ROSE_DOMAIN(stp_language));
	curse.traverse(des);
	if (curse.size() > 0){ i = 0; }
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

/*void doUnits(Workpiece * wkpc, ptree* tree){
	auto gc = Geometric_context::find(wkpc->get_its_geometry()->context_of_items());
	if (gc){
		auto l_u = gc->get_length_unit();
		if (l_u){
			//to get info out of units cast to type I want and just take it
			uidTracker* mgr = uidTracker::find(gc->get_length_unit());
			if (!mgr){
				mgr = uidTracker::make(gc->get_length_unit());
				uid++;
				std::string tmpUid = std::string("u--") + std::to_string(uid);
				ptree& unit = tree->add("Unit", "");
				unit.add("<xmlattr>.uid", tmpUid);
				mgr->setUid(tmpUid);
				auto SI = ROSE_CAST(stp_si_unit, l_u);
				if (SI){
					unit.add("Kind.ClassString", "SI system");
					if (SI->name() >= 0){ unit.add("Name.ClassString", unit_names[SI->name()]); }
					if (SI->prefix() >= 0){ unit.add("Prefix.ClassString", prefixes[SI->prefix()]); }
				}
			}
		}
		//std::cout << gc->get_plane_angle_unit()->domain()->name() << "\n";
		auto pa_u = gc->get_plane_angle_unit();
		if (pa_u){
			uidTracker* mgr = uidTracker::find(gc->get_plane_angle_unit());
			if (!mgr){
				mgr = uidTracker::make(gc->get_plane_angle_unit());
				uid++;
				std::string tmpUid = std::string("u--") + std::to_string(uid);
				ptree& unit = tree->add("Unit", "");
				unit.add("<xmlattr>.uid", tmpUid);
				mgr->setUid(tmpUid);
				auto SI = ROSE_CAST(stp_si_unit, pa_u);
				auto conversion_unit = ROSE_CAST(stp_conversion_based_unit, pa_u);
				if (SI){
					unit.add("Kind.ClassString", "SI system");
					if (SI->name() >= 0){ unit.add("Name.ClassString", unit_names[SI->name()]); }
					if (SI->prefix() >= 0){ unit.add("Prefix.ClassString", prefixes[SI->prefix()]); }
				}
				else if (conversion_unit){
					unit.add("Kind.ClassString", "Conversion Based system");
					//std::cout << conversion_unit->name() << "\n";
					unit.add("Name.ClassString", conversion_unit->name());

				}
			}
		}
		//std::cout << gc->get_solid_angle_unit()->domain()->name() << "\n";
		auto sa_u = gc->get_solid_angle_unit();
		if (sa_u){
			uidTracker* mgr = uidTracker::find(gc->get_solid_angle_unit());
			if (!mgr){
				mgr = uidTracker::make(gc->get_solid_angle_unit());
				uid++;
				std::string tmpUid = std::string("u--") + std::to_string(uid);
				ptree& unit = tree->add("Unit", "");
				unit.add("<xmlattr>.uid", tmpUid);
				auto SI = ROSE_CAST(stp_si_unit, sa_u);

				if (SI){
					unit.add("Kind.ClassString", "SI system");
					if (SI->name() >= 0){ unit.add("Name.ClassString", unit_names[SI->name()]); }
					if (SI->prefix() >= 0){
						//std::cout << SI->prefix() << "\n";
						unit.add("Prefix.ClassString", prefixes[SI->prefix()]);
					}
				}
			}
		}
	}
}

void doShapeDependentProperty(Workpiece * wkpc, ptree* tree){ //waiting for joe to implement metafunctions for vole, surface area, centroid, etc..
	uid++;
	ptree& sdp = tree->add("ShapeDependentProperty", "");
	sdp.add("<xlmattr>.uid", "sdp--" + std::to_string(uid));
	sdp.add("<xlmattr>.xsi:type", "n1:GeneralShapeDependentProperty");
}*/

void doPartProperty(ptree* tree, RoseObject* ent){//filler code for demo
	uid++;
	stp_product_definition * pd = ROSE_CAST(stp_product_definition, ent);
	ptree& pv = tree->add("AssignedPropertyValue.PropertyValue", "");
	pv.add("<xmlattr>.uid", "pv--" + std::to_string(uid));
	pv.add("<xmlattr>.xsi:type", "n1:StringValue");
	pv.add("Definition.PropertyDefinitionString", "Part Origin"); // place of origin. is this for the design or manufacture of the part?
	pv.add("Name.CharacterString", pd->formation()->of_product()->name());
	pv.add("ValueComponent.CharacterString", "Step Tools Inc");
}

std::string handleGeometry(stp_shape_definition_representation* sdr, ptree* tree){
	//returns the uid for the created geometricrepresentation  for use in PartView and storing geometry
	stp_shape_representation* srep = ROSE_CAST(stp_shape_representation, sdr->used_representation());
	stp_product_definition* pd = ROSE_CAST(stp_product_definition, rose_get_nested_object(ROSE_CAST(stp_product_definition_shape, rose_get_nested_object(sdr->definition()))->definition()));
	uid++;
	int currentUid = uid;
	std::string uidForRef("gm--" + std::to_string(currentUid));
	ptree& geo = tree->add("n0:Uos.DataContainer.GeometricRepresentation", "");
	geo.add("<xmlattr>.uid", uidForRef);

	uid++;
	geo.add( srep->context_of_items()->className() + std::string(".<xmlattr>.uidRef"), "gcs--" + std::to_string(uid));
	ptree& dat = tree->add("n0:Uos.DataContainer.GeometricCoordinateSpace", "");
	dat.add("<xmlattr>.uid", "gcs--" + std::to_string(uid));
	auto gc = srep->context_of_items(); 
	RoseAttribute* tmpAtt = gc->getAttribute("coordinate_space_dimension");
	dat.add("DimensionCount", gc->getInteger(tmpAtt));
	ptree& acc = dat.add("Accuracies", ""); //may need check for existance of any accuracy units
	//handles Geometric coordinateSpace 
	for (unsigned i = 0; i < gc->attributes()->size(); i++){
		RoseAttribute* att = gc->attributes()->get(i);
		std::cout << gc->attributes()->get(i)->name() << ": ";
		if (att->isSimple()){
			if (att->isInteger()){ std::cout << gc->getInteger(att) << "int\n";	}
		}
		else{
			if (att->isSelect()){
				RoseObject* obj = rose_get_nested_object(ROSE_CAST(RoseUnion, gc->getObject(att)));
				std::cout << obj->size() << "\n";
			}
			else if (att->isAggregate()){ 
				convertEntity(&acc, gc->getObject(att), currentUid);
			}
			else{ std::cout << gc->getObject(att)->domain()->name() << "\n"; } 
		}
	}

	dat.add("Id.<xmlattr>.id", pd->formation()->of_product()->name());
	
	ptree& items = geo.add("Items", "");
	for (unsigned j = 0, sz = srep->items()->size(); j < sz; j++){
		//children[j] is every RoseObject that is geometry
		RoseAttribute* att = srep->getAttribute("items");
		if (srep->items()->get(j)->domain() != ROSE_DOMAIN(stp_manifold_solid_brep)){
			ptree& repItem = items.add("RepresentationItem", "");
			uid++;
			if (srep->items()->get(j)->isa(ROSE_DOMAIN(stp_placement))){
				repItem.add("<xmlattr>.uid", srep->items()->get(j)->domain()->name() + std::string("--") + std::to_string(uid));
				repItem.add("<xmlattr>.xsi:type", "n1:AxisPlacement");
			}
			else{
				repItem.add("<xmlattr>.uid", srep->items()->get(j)->domain()->name() + std::string("--") + std::to_string(uid));
				repItem.add("<xmlattr>.xsi:type", std::string("n1:") + srep->items()->get(j)->domain()->name());
			}
			convertEntity(&repItem, srep->items()->get(j), currentUid);
		}
	}

	return uidForRef;
}

void makePart(stp_shape_definition_representation * sdr, ptree* tree){
	// void makePart(stp_shape_definition_representation * sdr, ptree* tree){
	uid++;
	unsigned i;
	int currentUid = uid;
	RoseObject* obj;
	ListOfRoseObject children;
	stp_product_definition* pd = ROSE_CAST(stp_product_definition, rose_get_nested_object(ROSE_CAST(stp_product_definition_shape, rose_get_nested_object(sdr->definition()))->definition()));
	pd->findObjects(&children, INT_MAX, false);

	uidTracker* mgr = uidTracker::find(pd);
	if (mgr){
		if (mgr->getUid()){
			currentUid = mgr->getUid();
		}
	}

	ptree& part = tree->add(std::string("n0:Uos.DataContainer.Part"), "");
	part.add("<xmlattr>.id", pd->domain()->name() + std::string("--") + std::to_string(currentUid));

	ptree& xmlObj = part.add("Id", "");
	xmlObj.add("<xmlattr>.id", pd->formation()->of_product()->name() );

	xmlObj.add("Identifier", "");
	xmlObj.add("Identifier.<xmlattr>.uid", "pid--" + std::to_string(currentUid) + "--id" + std::to_string(id_count));

	xmlObj.add("Identifier.<xmlattr>.id", pd->formation()->of_product()->name());
	xmlObj.add("Identifier.<xmlattr>.idContextRef", "create references");

	part.add("Name.CharacterString", pd->formation()->of_product()->name());

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

	if (pd->formation()->id() != NULL && strcmp(pd->formation()->id(), "None") && strcmp(pd->formation()->id(), "")){ //DO STRCMP or similarto check if none or empty
		part.add("Versions.Id.<xmlattr>.id", pd->formation()->id());
	}
	else { part.add("Versions.Id.<xmlattr>.id", "/NULL"); } //replace null with a check for versioning that returns a string 

	ptree& pv = part.add("Versions.PartVersion.Views.PartView", "");

	pv.add("<xmlattr>.xsi:type", "n0:AssemblyDefinition");
	pv.add("<xmlattr>.uid", "pvv--" + std::to_string(currentUid) + "--id" + std::to_string(id_count));

	std::string geoRef = handleGeometry(sdr, tree);	//ptree& geoRep = tree->add("n0:Uos.DataContainer.GeometricRepresentation", "");
	pv.put("DefiningGeometry.<xmlattr>.uidRef", geoRef);
	///do single occurence, 
	StixMgrAsmProduct * pm = StixMgrAsmProduct::find(pd);
	if (mgr){
		if (mgr->getPV()){
			for (i = 0; i < pm->parent_nauos.size() ; i++){
				ptree& pi = pv.add("Occurrence", "");
				pi.add("<xmlattr>.xsi:type", "n0:SingleOccurrence");
				pi.add("<xmlattr>.uid", "pi--" + std::to_string(mgr->getUid()) + "--id" + std::to_string((i)) );
				pi.add("Id.<xmlattr>. id", pd->formation()->of_product()->name() + std::string(".") + std::to_string((i)) );
				pi.add("PropertyValueAssignment.<xmlattr>.uid", "pva--" + std::to_string(currentUid));
				mgr->setSubRelation("pi--" + std::to_string(mgr->getUid()) + "--id" + std::to_string((i + 1)));
			}

			uid++;
			if ( mgr->needsSpecifiedOccurrence && mgr->getParentOccurrences() > 1){//need to have it check occurrences of the parent design?
				for (i = 1; i < mgr->getParentOccurrences()+1; i++){
					ptree& pi = pv.add("Occurrence", "");
					pi.add("<xmlattr>.xsi:type", "n0:SpecifiedOccurrence");
					pi.add("<xmlattr>.uid", "spo--" + std::to_string(uid));
					pi.add("AssemblyContext.<xmlattr>.uidRef", mgr->getAssemblyContext());
					pi.add("SubAssemblyRelationship.<xmlattr>.uidRef", mgr->getSubRelation());
					pi.add("UpperAssemblyRelationship.<xmlattr>.uidRef", "");
				}
			}
		}
		std::cout << "Occurrence: " << pd->formation()->of_product()->name() << ", " << pm->parent_nauos.size() << "\n";
	}
	do_nauos(pd, &pv, currentUid); //deals with objects below the current product in a tree view of the design while maintaining acg
	ptree& pva = pv.add("PropertyValueAssignment", "");
	pva.add("<xmlattr>.uid", "pva--" + std::to_string(currentUid));
	doPartProperty(&pva, pd);
	//stp_mass_measure_with_unit
	id_count++;
	return;
}

void do_nauos(stp_product_definition* pd, ptree* pv, int currentUid){
	StixMgrAsmProduct * pm = StixMgrAsmProduct::find(pd);
	uidTracker* pgMgr = uidTracker::find(pd);
	for (unsigned i = 0; i < pm->child_nauos.size(); i++){
		//uid++; 
		ptree& pi = pv->add("ViewOccurrenceRelationship", "");
		uidTracker* mgr = uidTracker::make(stix_get_related_pdef(pm->child_nauos[i])); //may need to label something different
		std::cout << stix_get_related_pdef(pm->child_nauos[i])->formation()->of_product()->name() << mgr->getOccurence() << "\n";
		if (mgr->getUid() == 0){
			uid++;
			mgr->setUid(uid);
			if (pgMgr){
				if (pgMgr->needsSpecifiedOccurrence){
					mgr->needsSpecifiedOccurrence = true;
					mgr->setAssemblyContext(pgMgr->getAssemblyContext());
				}
			}
		}
		else{ //specified occurnce
			mgr->needsSpecifiedOccurrence = true; //child parts require (occurence-1) specified occurrences
			// use a list attached to nauo or pd to keep an ordered of how many single occurrences are needed?
		}

		mgr->setPV(pv);
		pi.add("<xmlattr>.uid", "pvvid--" + std::to_string(uid) + "--id" + std::to_string(mgr->occurence));
		pi.add("<xmlattr>.xsi:type", std::string("n0:") + pm->child_nauos[i]->domain()->name());
		pi.add("Related.<xmlattr>.uidRef", "pi--" + std::to_string(uid) + "--id" + std::to_string(mgr->occurence));
		pi.add("Id.<xmlattr>.id", pm->child_nauos[i]->id() + std::string(".") + std::to_string(mgr->getOccurence()));
		pi.add("Description", pm->child_nauos[i]->description());
		pi.add("PropertyValueAssignment.<xmlattr>.uidRef", "pva--" + std::to_string(currentUid));
		mgr->occurence++;

		if (!mgr->needsSpecifiedOccurrence){
			mgr->setAssemblyContext("pvv--" + std::to_string(currentUid) + "--id" + std::to_string(id_count));
		}
	}
	std::cout << "Parent nauo of " << pd->formation()->of_product()->name() << ": " << pm->parent_nauos.size() << "\n";
	//specified occurrences
	for (unsigned i = 0; i < pm->child_nauos.size(); i++){
		if (pgMgr){
			if (pgMgr->getOccurence() > 2 && pgMgr->needsSpecifiedOccurrence){//need to have it check occurrences of the parent design?
				for (unsigned i = 0; i < pm->child_nauos.size(); i++){//set Upper Assemly Relationship
					uidTracker* mgr = uidTracker::find(stix_get_related_pdef(pm->child_nauos[i]));
					if (pgMgr->getParentOccurrences() > 1){
						mgr->ParentOccurences = pgMgr->getParentOccurrences() * pm->parent_nauos.size();
					}
					else{ mgr->ParentOccurences = pm->parent_nauos.size(); }
				}
			}
		}
	}
}

void copyHeader(ptree* tree, RoseDesign* master){
	unsigned i, sz;
	//TODO: place holder. ask what needs to go in header on monday
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

int CountSubs(stp_product_definition * root){ //return the total count of subassemblies in a product
	unsigned subs = 0;
	StixMgrAsmProduct * pm = StixMgrAsmProduct::find(root);
	if (pm->child_nauos.size()) {
		unsigned i, sz;
		for (i = 0, sz = pm->child_nauos.size(); i < sz; i++){
			subs += CountSubs(stix_get_related_pdef(pm->child_nauos[i]));
		}
		subs += sz;
	}
	return subs;
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

	stix_tag_asms(master);

	for (auto &i : ROSE_RANGE(stp_shape_definition_representation, master)){
		//doUnits(&i, &tree);
		makePart(&i, &tree);
	}

	/*ARMCursor cur; //arm cursor
	ARMObject *a_obj;
	cur.domain(Workpiece::type());
	cur.traverse(master);
	ListOfRoseObject aimObjs;
	//unsigned i, sz;
	while (a_obj = cur.next()){
		doUnits(a_obj->castToWorkpiece(), &tree);
		//makePart(a_obj->castToWorkpiece(), &tree);
	}
	cur.traverse(master);
	cur.domain(NULL);
	while (a_obj = cur.next()){
		//std::cout << a_obj->getModuleName() << std::endl;
	}
	*/
	write_xml(std::string(master->fileDirectory() + name), tree, std::locale(), xml_writer_settings<char>(' ', 4));

	return 0;
};