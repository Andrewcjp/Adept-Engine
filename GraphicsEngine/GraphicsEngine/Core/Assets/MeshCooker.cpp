#include "MeshCooker.h"



MeshCooker::MeshCooker()
{
}


MeshCooker::~MeshCooker()
{
}


bool FileExists(std::string filename) {
	struct stat fileInfo;
	return stat(filename.c_str(), &fileInfo) == 0;
}
bool MeshCooker::LoadMeshFile(const char * filename,Mesh* meshobject)
{
	if (FileExists(filename) == false) {
		return false;
	}
	//filename = "data.bin";
	//we load the baked asset file 
	//if its changed we cause a reread
	int datalength = 17;
	char * attribdata = new char[datalength];//two points of data



	std::ifstream ifp(filename, std::ios::in | std::ios::binary);
	ifp.read(reinterpret_cast<char*>(attribdata), datalength * sizeof(char));

	std::vector<std::string> split = MeshCooker::split(std::string(attribdata), '|');
	int vertsize = std::stoi(split[0]);
	int indicesize = std::stoi(split[1]);
	//printf("attrib is %s and decode is %d and %d", attribdata, vertsize, indicesize);
	std::vector<Vertex> verts(vertsize);
	std::vector<int> indices(indicesize);
	ifp.read(reinterpret_cast<char*>(verts.data()), verts.size() * sizeof(Vertex));
	ifp.read(reinterpret_cast<char*>(indices.data()), indices.size() * sizeof(int));
	ifp.close();

	//printf("Reading  %d length vertex one has positon %f %f\n", verts.size(), verts[0].GetPos()->x, verts[0].GetPos()->y);
	delete attribdata;
	meshobject->Init(&verts[0], static_cast<int>(verts.size()), (int*)&indices[0], static_cast<int>(indices.size()));
	return true;
}

void MeshCooker::SaveMeshFile(const char *  filename, std::vector<Vertex> verts, std::vector<int> indices)
{

	std::ofstream ofp(filename, std::ios::out | std::ios::binary);
	std::stringstream stream;
	stream << std::setfill('0')<< std::setw(8) <<(verts.size())<<"|" << std::setfill('0') << std::setw(8) << (indices.size());
	std::string data = stream.str();
//	std::string data(std::to_string(verts.size())+"|"+ std::to_string(indices.size()));//todo format!
	ofp.write(reinterpret_cast<const char*>((data.c_str())), data.length());
	//printf("Writting %d length vertex one has positon %f %f\n", verts.size(), verts[0].GetPos()->x, verts[0].GetPos()->y);
	ofp.write(reinterpret_cast<const char*>(verts.data()), verts.size() * sizeof(Vertex));
	ofp.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(int));
	ofp.close();
}
