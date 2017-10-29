#include "OBJFileReader.h"
#include "glm\glm.hpp"
static int firstPassOBJRead(LPCWSTR filename, int* vertex_count, int* vert_normal_count, int* vert_texcoord_count, int* triangle_count)
{
	FILE* pfile;

	*vertex_count = 0;
	*vert_normal_count = 0;
	*vert_texcoord_count = 0;
	*triangle_count = 0;

	_wfopen_s(&pfile, filename, L"r");

	if (!pfile)
	{
		//something has gone wrong when opening the file.
		printf("Failed to load file %ls", filename);
		return 1;
	}

	char tempbuffer[1024];
	char* strread = fgets(tempbuffer, 1024, pfile);

	do
	{
		char* v = &(tempbuffer[0]);
		if (*v == 'v')
		{
			if (*(v + 1) == 'n')		*vert_normal_count += 1;
			else if (*(v + 1) == 't')	*vert_texcoord_count += 1;
			else					*vertex_count += 1;
		}
		else if (*v == 'f')
		{
			int numIndices = 0;
			char* vs = strstr(v, " ");

			//vs = strstr(vs+1, " ");
			while (vs)
			{
				//numIndices += 1;
				vs = strstr(vs + 1, "/");
				if (vs)
					numIndices += 1;
			}

			int numTriangles = 1;//numIndices / 2 - 2;
			*triangle_count += numTriangles;
		}

		strread = fgets(tempbuffer, 1024, pfile);

	} while (strread != NULL);

	fclose(pfile);

	return 0;
}

static int secondPassOBJRead(LPCWSTR filename, int nVerts, int nNormals, int nTexcoords, Triangle* mesh)
{
	FILE* pfile;

	_wfopen_s(&pfile, filename, L"r");

	if (!pfile)
	{
		//something has gone wrong when opening the file.
		printf("Failed to load file %ls", filename);
		return 1;
	}

	char tempbuffer[1024];
	char* strread = fgets(tempbuffer, 1024, pfile);
	int vertex_read = 0;
	int normal_read = 0;
	int texcoord_read = 0;
	int triangle_read = 0;

	glm::vec3 *normal_buffer = new glm::vec3[nNormals];
	glm::vec3 *vertex_buffer = new glm::vec3[nVerts];
	glm::vec3 *texcoord_buffer = new glm::vec3[nTexcoords];

	do
	{
		char* v = &(tempbuffer[0]);
		if (*v == 'v')
		{
			if (*(v + 1) == 'n')
			{
				sscanf_s(v, "%*s %f %f %f", &(normal_buffer[normal_read][0]),
					&(normal_buffer[normal_read][1]),
					&(normal_buffer[normal_read][2]));
				normal_read += 1;
			}
			else if (*(v + 1) == 't')
			{
				sscanf_s(v, "%*s %f %f", &(texcoord_buffer[texcoord_read][0]),
					&(texcoord_buffer[texcoord_read][1]));
				texcoord_read += 1;
			}
			else
			{
				sscanf_s(v + 1, "%f %f %f", &(vertex_buffer[vertex_read][0]),
					&(vertex_buffer[vertex_read][1]),
					&(vertex_buffer[vertex_read][2]));
				vertex_read += 1;
			}
		}
		else if (*v == 'f')
		{
//			int dummy;
			int index1;
			int index2;
			int index3;
//			int index4;

			int nidx1, nidx2, nidx3;// , nidx4;

			int tidx1, tidx2, tidx3;// , tidx4;

			if (sscanf_s(v, "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &index1, &tidx1, &nidx1,
				&index2, &tidx2, &nidx2,
				&index3, &tidx3, &nidx3) == 9)
			{
				mesh[triangle_read].SetVertices(vertex_buffer[index1 - 1], vertex_buffer[index2 - 1], vertex_buffer[index3 - 1]);
				mesh[triangle_read].SetNormals(normal_buffer[nidx1 - 1], normal_buffer[nidx2 - 1], normal_buffer[nidx3 - 1]);
				mesh[triangle_read].SetTexCoords(texcoord_buffer[tidx1 - 1], texcoord_buffer[tidx2 - 1], texcoord_buffer[tidx3 - 1]);

				glm::vec3 edge = vertex_buffer[index2 - 1] - vertex_buffer[index1 - 1];
				glm::vec3 edge2 = vertex_buffer[index3 - 1] - vertex_buffer[index1 - 1];
				glm::vec3 uvedge = texcoord_buffer[tidx2 - 1] - texcoord_buffer[tidx1 - 1];
				glm::vec3 uvedge2 = texcoord_buffer[tidx3 - 1] - texcoord_buffer[tidx1 - 1];

				//	glm::vec3 tangent = glm::normalize(edge - uvedge.y);
				float r = 1.0f / ((edge.x * uvedge2.y) - (uvedge.y * uvedge2.x));
				//	glm::vec3 tangent = (edge * uvedge2.y - edge2 - uvedge.y)*r;


				glm::vec3 tangent = glm::normalize(r*(uvedge2.y *edge - uvedge.y * edge2));

				mesh[triangle_read].SetTangent(tangent, tangent, tangent);
				//glm::vec3 bitangent = glm::normalize(r*(-uvedge2.y *edge + uvedge.y * edge2));
				//mesh[triangle_read].SetBiTangent(bitangent, bitangent, bitangent);
				mesh[triangle_read].SetBiTangent(glm::cross(tangent, normal_buffer[nidx1 - 1]), glm::cross(tangent, normal_buffer[nidx2 - 1]), glm::cross(tangent, normal_buffer[nidx3 - 1]));
			}
			else
			{
				//something is seriously fucked. abort
				//I don't want to handle them
				break;
			}
			triangle_read += 1;
		}

		strread = fgets(tempbuffer, 1024, pfile);

	} while (strread != NULL);

	fclose(pfile);

	delete[] vertex_buffer;
	delete[] normal_buffer;
	delete[] texcoord_buffer;

	return 0;
}

int importOBJMesh(LPCWSTR filename, Triangle** triangles)
{
	int num_triangles = 0;
	int num_vertices = 0;
	int num_normals = 0;
	int num_texcoords = 0;

	firstPassOBJRead(filename, &num_vertices, &num_normals, &num_texcoords, &num_triangles);

	*triangles = new Triangle[num_triangles];

	secondPassOBJRead(filename, num_vertices, num_normals, num_texcoords, *triangles);

	return num_triangles;
}