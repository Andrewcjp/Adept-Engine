#pragma once

#include <iostream>
#include <vector>
#include "RenderBaseTypes.h"
#include "Renderable.h"
#include "../RHI/RHICommandList.h"
class Mesh :public Renderable
{
public:
	Mesh();
	Mesh(std::string filename);
	~Mesh();
	void Render(RHICommandList* list) override;	
	void LoadMeshFromFile(std::string filename);
private:
	RHIBuffer* VertexBuffer = nullptr;
	RHIBuffer* IndexBuffer = nullptr;
};

