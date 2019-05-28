#pragma once
class CullingAABB
{
public:
	CullingAABB();
	~CullingAABB();
	CullingAABB(glm::vec3 Pos, glm::vec3 size);
	__forceinline glm::vec3 GetMin() const;
	__forceinline glm::vec3 GetMax() const;
	void DebugRender(glm::vec3 colour = glm::vec3(1), float time = 0.0f);

	glm::vec3 ClosestPoint(const glm::vec3 & point) const;

	static CullingAABB * CreateFromMinMax(const glm::vec3 & min, const glm::vec3 & max);

	void AddAABB(CullingAABB * other);
	glm::vec3 GetPos() const;
	glm::vec3 GetHalfExtends_Unscaled() const;
	glm::vec3 GetHalfExtends() const;
	void Reset();
	void SetPos(glm::vec3 pos);
	void SetScale(glm::vec3 scale);
private:
	glm::vec3 Scale = glm::vec3(1, 1, 1);
	glm::vec3 ScaledHalfExtends = glm::vec3();
	glm::vec3 HalfExtends = glm::vec3();
	glm::vec3 Position = glm::vec3();
};

