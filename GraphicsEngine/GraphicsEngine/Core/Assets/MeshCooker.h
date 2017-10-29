#pragma once
#include <iostream>
#include <ios>
#include <ostream>
#include <fstream>
#include <vector>
#include "../Rendering/Core/Mesh.h"
#include <string>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <sys/stat.h>
class MeshCooker
{
public:
	MeshCooker();
	~MeshCooker();
	
	static bool LoadMeshFile(const char * filename, Mesh * meshobject);
	static void SaveMeshFile(const char *  filename,std::vector<Vertex> verts, std::vector<int> indices);
	//the cooker will generate binaryfiles of the mesh data that wiil be loaded into memory the first bytes will refrance the information about the file
	// number of vertexs
	//a splitting char will be inbetween data to pevent overlap!
	//much faster than process the model each time
	// the number of indices 
	template<typename Out>
	static  void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}


	static std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}
};

