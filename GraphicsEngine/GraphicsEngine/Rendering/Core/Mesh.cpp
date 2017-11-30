#include "Mesh.h"



Mesh::Mesh()
{

}

Mesh::Mesh(std::string filename)
{

	LoadMeshFromFile(filename);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &Indexbuffer);
}


void Mesh::Render(CommandListDef * list)
{


	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuffer);
	glDrawElements(GL_TRIANGLES, m_indexsize, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

void Mesh::Init(Vertex* verts, int vertsize, int* Indicies, int indexsize)
{
	m_verts = verts;
	m_Indicies = Indicies;
	m_indexsize = indexsize;
	m_vertsize = vertsize;
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertsize * sizeof(Vertex), m_verts, GL_STATIC_DRAW);
	glGenBuffers(1, &Indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexsize * sizeof(int), m_Indicies, GL_STATIC_DRAW);

}
void Mesh::LoadMeshFromFile(std::string filename)
{
	//m_meshData = 0;
	//BROKEN!
//	std::cout << " USED BORKEN ASSIMP" << std::endl;
//	return;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename.c_str(), aiProcessPreset_TargetRealtime_Quality);

	if (!scene || scene->mNumMeshes == 0)
	{
		std::cout << "Mesh load failed!: " << filename << std::endl;
		return;
	}

	const aiMesh* model = scene->mMeshes[0];

	std::vector<Vertex> vertices;
	std::vector<int> indices;
	//std::cout << "Texture COrrds" << model->GetNumUVChannels() << std::endl;
	const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
	for (unsigned int i = 0; i < model->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(model->mVertices[i]);
		const aiVector3D* pNormal = &(model->mNormals[i]);
		const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
		const aiVector3D* pTangent = &(model->mTangents[i]);

		Vertex vert(glm::vec3(pPos->x, pPos->y, pPos->z),
			glm::vec2(pTexCoord->x, pTexCoord->y),
			glm::vec3(pNormal->x, pNormal->y, pNormal->z),
			glm::vec3(pTangent->x, pTangent->y, pTangent->z));

		vertices.push_back(vert);
	}

	for (unsigned int i = 0; i < model->mNumFaces; i++)
	{
		const aiFace& face = model->mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
	//InitMesh(&vertices[0], vertices.size(), (int*)&indices[0], indices.size(), false);
	Init(&vertices[0], static_cast<int>(vertices.size()), (int*)&indices[0], static_cast<int>(indices.size()));

}
