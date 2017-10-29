#include "EditorGizmos.h"
#include "Core/GameObject.h"
#include <GLEW\GL\glew.h>
#include "../Rendering/Core/DebugLineDrawer.h"
EditorGizmos::EditorGizmos()
{
	HandleLength = 5;
}


EditorGizmos::~EditorGizmos()
{
}

void EditorGizmos::RenderGizmos(DebugLineDrawer* lien)
{
	if (target == nullptr)
	{
		return;
	}
	glm::vec3 Xaxisend = target->GetTransform()->GetPos() + glm::vec3(HandleLength, 0, 0);
	glm::vec3 yaxisend = target->GetTransform()->GetPos() + glm::vec3(0, HandleLength, 0);
	glm::vec3 zaxisend = target->GetTransform()->GetPos() + glm::vec3(0, 0, HandleLength);
	if (true)
	{
		lien->AddLine(target->GetTransform()->GetPos(), Xaxisend, glm::vec3(1, 0, 0));
		lien->AddLine(target->GetTransform()->GetPos(), yaxisend, glm::vec3(0, 1, 0));
		lien->AddLine(target->GetTransform()->GetPos(), zaxisend, glm::vec3(0, 0, 1));
	}
	else
	{
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
		glLineWidth(2.5);
		glColor3f(1, 0, 0);
		glVertex3f(target->GetTransform()->GetPos().x, target->GetTransform()->GetPos().y, target->GetTransform()->GetPos().z);
		glVertex3f(Xaxisend.x, Xaxisend.y, Xaxisend.z);

		glColor3f(0, 1, 0);
		glVertex3f(target->GetTransform()->GetPos().x, target->GetTransform()->GetPos().y, target->GetTransform()->GetPos().z);
		glVertex3f(yaxisend.x, yaxisend.y, yaxisend.z);
		glColor3f(0, 0, 1);
		glVertex3f(target->GetTransform()->GetPos().x, target->GetTransform()->GetPos().y, target->GetTransform()->GetPos().z);
		glVertex3f(zaxisend.x, zaxisend.y, zaxisend.z);
		glEnd();
		glEnable(GL_DEPTH_TEST);
	}
	GLenum result = glGetError();
	if (result != GL_NO_ERROR)
	{
		const GLubyte * d = glewGetErrorString(result);
		//__debugbreak();
	}
	//	GLbegin(GL_LINES);
		//glenable();
		//GL.Begin(GL.LINES);
		//// X line
		//GL.Color(new Color(1, 0, 0, 0.5f));
		//GL.Vertex3(target.transform.position.x, target.transform.position.y, target.transform.position.z);
		//GL.Vertex3(xAxisEnd.x, xAxisEnd.y, xAxisEnd.z);
		//// Y line
		//GL.Color(new Color(0, 1, 0, 0.5f));
		//GL.Vertex3(target.transform.position.x, target.transform.position.y, target.transform.position.z);
		//GL.Vertex3(yAxisEnd.x, yAxisEnd.y, yAxisEnd.z);
		//// Z line
		//GL.Color(new Color(0, 0, 1, 0.5f));
		//GL.Vertex3(target.transform.position.x, target.transform.position.y, target.transform.position.z);
		//GL.Vertex3(zAxisEnd.x, zAxisEnd.y, zAxisEnd.z);
		//GL.End();
}

void EditorGizmos::SetTarget(GameObject * t)
{
	target = t;
}
