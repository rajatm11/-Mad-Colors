#pragma once
#include "../../Common/Matrix4.h"
#include "../../Common/TextureBase.h"
#include "../../Common/ShaderBase.h"
#include "../../Common/Vector4.h"
#include<vector>
//8508
#include "../../Common/MeshAnimation.h"

namespace NCL {
	using namespace NCL::Rendering;

	class MeshGeometry;
	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader);
			~RenderObject();

			void SetDefaultTexture(TextureBase* t) {
				texture = t;
			}

			TextureBase* GetDefaultTexture() const {
				return texture;
			}

			MeshGeometry*	GetMesh() const {
				return mesh;
			}

			Transform*		GetTransform() const {
				return transform;
			}

			ShaderBase*		GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return colour;
			}

			//8508
			int GetFlag() const {
				return objFlag;
			}

			void SetFlag(int flag) {
				objFlag = flag;
			}

			void SetTextures(std::vector<unsigned int> texs) {
				textures = texs;
			}

			std::vector<unsigned int> GetTextures() const {
				return textures;
			}
			
			void SetAnimation(MeshAnimation* a) {
				animation = a;
			}

			MeshAnimation* GetAnimation() const {
				return animation;
			}

			MeshAnimation* GetAnimation() {
				return animation;
			}

			void SetRenderShadow(bool r) { renderShadow = r; }

			bool RenderShadow() const { return renderShadow; }

		protected:
			std::vector<unsigned int> textures;

			MeshGeometry*	mesh;
			MeshAnimation*	animation;
			TextureBase*	texture;
			ShaderBase*		shader;
			Transform*		transform;
			Vector4			colour;
			//8508
			int				objFlag = 0;
			bool			renderShadow;
		};
	}
}
