#pragma once
#include "../vkCfg.h"
#include <glm/gtx/hash.hpp>
#include "../vkImage/texture.h"

namespace vkMesh {
    struct MeshBuffer {
        Buffer vertexBuffer;
        Buffer indexBuffer;
    };
    
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 texCoord;
        glm::vec3 normal;

        static vk::VertexInputBindingDescription getBindingDescription() {
            /*
            VertexInputBindingDescription(
                uint32_t            binding_   = {},
                uint32_t            stride_    = {},
                vk::VertexInputRate inputRate_ = vk::VertexInputRate::eVertex
            ) VULKAN_HPP_NOEXCEPT
            */
            return{
                0,
                sizeof(Vertex),
                vk::VertexInputRate::eVertex
            };
        }

        static const std::array<vk::VertexInputAttributeDescription, 3>& getAttributeDescriptions() {
            /*
            VertexInputAttributeDescription(
                uint32_t   location_ = {},
                uint32_t   binding_  = {},
                vk::Format format_   = vk::Format::eUndefined,
                uint32_t   offset_   = {}
            ) VULKAN_HPP_NOEXCEPT
            */
            return std::array<vk::VertexInputAttributeDescription, 3>{
                vk::VertexInputAttributeDescription{
                    0,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    offsetof(Vertex, pos)
                },
                vk::VertexInputAttributeDescription{
                    2,
                    0,
                    vk::Format::eR32G32Sfloat,
                    offsetof(Vertex, texCoord)
                },
                vk::VertexInputAttributeDescription{
                    3,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    offsetof(Vertex, normal)
                }
            };
        }

        bool operator==(const Vertex& other) const {
            return pos == other.pos && texCoord == other.texCoord && normal == other.normal;
        }
    };

	struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
	};

    struct Mesh {
        const uint32_t firstVertexIndex;
        const uint32_t vertexIndexCount;
        const vkImage::Texture texture;
    };

	struct MeshLoadInfo {
		const std::string& filepath;
		const glm::mat3& preTransform;
	};
    
    inline std::vector<uint32_t> triangulate(uint32_t vertexCount);

    static MeshData* loadMesh(const MeshLoadInfo* info);

    inline MeshData* loadMeshFBX(const MeshLoadInfo* info);

    inline MeshData* loadMeshOBJ(const MeshLoadInfo* info);

    inline void loadVertexBuffer(
        const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::CommandBuffer& commandBuffer, const vk::Queue graphicsQueue,
        const std::vector<const MeshData*&>& meshesData, const std::vector<const vkImage::Texture*&>& textures, const std::vector<std::string>& types,
        std::unordered_map<std::string, Mesh>& meshes, MeshBuffer& meshBuffer
    );
}

namespace std {
    template<> struct hash<vkMesh::Vertex> {
        size_t operator()(vkMesh::Vertex const& vertex) const {
            return ((((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1);
        }
    };
}