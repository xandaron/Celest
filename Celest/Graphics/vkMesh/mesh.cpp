#include "mesh.h"
#include "OpenFBX/ofbx.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <TinyOBJ/tiny_obj_loader.h>
#include "../vkUtil/memory.h"

inline std::vector<uint32_t> vkMesh::triangulate(uint32_t vertexCount) {
	std::vector<uint32_t> result;
	if (vertexCount < 2) { return result; }
	if (vertexCount == 3) {
		result.push_back(0);
		result.push_back(1);
		result.push_back(2);
	}
	else {
		for (int tri = 0; tri < vertexCount - 2; ++tri) {
			result.push_back(0);
			result.push_back(1 + tri);
			result.push_back(2 + tri);
		}
	}
	return result;
}

static vkMesh::MeshData* vkMesh::loadMesh(const MeshLoadInfo* info) {
    std::vector<std::string> words = split(info->filepath, ".");
    if (words[1] == "obj") {
        return loadMeshOBJ(info);
    }
    else if (words[1] == "fbx") {
        return loadMeshFBX(info);
    }
    throw std::invalid_argument("invalid file type");
}

inline vkMesh::MeshData* vkMesh::loadMeshFBX(const MeshLoadInfo* info) {
	FILE* fp;
	fopen_s(&fp, info->filepath.c_str(), "rb");

	if (!fp) return false;

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);

	ofbx::LoadFlags flags =
		//ofbx::LoadFlags::IGNORE_MODELS |
		ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
		ofbx::LoadFlags::IGNORE_CAMERAS |
		ofbx::LoadFlags::IGNORE_LIGHTS |
		//ofbx::LoadFlags::IGNORE_TEXTURES |
		ofbx::LoadFlags::IGNORE_SKIN |
		ofbx::LoadFlags::IGNORE_BONES |
		ofbx::LoadFlags::IGNORE_PIVOTS |
		//ofbx::LoadFlags::IGNORE_MATERIALS |
		ofbx::LoadFlags::IGNORE_POSES |
		ofbx::LoadFlags::IGNORE_VIDEOS |
		ofbx::LoadFlags::IGNORE_LIMBS |
		//ofbx::LoadFlags::IGNORE_MESHES |
		ofbx::LoadFlags::IGNORE_ANIMATIONS;

	ofbx::IScene* g_scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u16)flags);

	if (!g_scene) {
		return false;
	}

	delete[] content;
	fclose(fp);

	const ofbx::Mesh& fbxMesh = *g_scene->getMesh(0);
	const ofbx::GeometryData& geom = fbxMesh.getGeometryData();
	const ofbx::Vec3Attributes positions = geom.getPositions();
	const ofbx::Vec3Attributes normals = geom.getNormals();
	const ofbx::Vec2Attributes uvs = geom.getUVs();

	MeshData* meshData = new MeshData;
	std::unordered_map<vkMesh::Vertex, uint32_t> vertexLookup{};
	for (int partitionID = 0; partitionID < geom.getPartitionCount(); partitionID++) {
		const ofbx::GeometryPartition& partition = geom.getPartition(partitionID);

		for (int polygonID = 0; polygonID < partition.polygon_count; polygonID++) {
			const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygonID];
			std::vector<uint32_t> triIndices = triangulate(polygon.vertex_count);

			for (int i = 0; i < triIndices.size(); i++) {
				const ofbx::Vec3 p = positions.get(polygon.from_vertex + triIndices[i]);
				const ofbx::Vec3 n = normals.get(polygon.from_vertex + triIndices[i]);
				const ofbx::Vec2 uv = uvs.get(polygon.from_vertex + triIndices[i]);

				vkMesh::Vertex vert;
				vert.pos = info->preTransform * glm::vec3(p.x, p.y, p.z);
				vert.texCoord = { uv.x, 1.0f - uv.y };
				vert.normal = info->preTransform * glm::vec3(n.x, n.y, n.z);

				if (!vertexLookup.contains(vert)) {
					vertexLookup[vert] = static_cast<uint32_t>(meshData->vertices.size());
					meshData->vertices.push_back(vert);
				}

				meshData->indices.push_back(vertexLookup[vert]);
			}
		}
	}
	return meshData;
}

inline vkMesh::MeshData* vkMesh::loadMeshOBJ(const MeshLoadInfo* info) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, info->filepath.c_str())) {
		throw std::runtime_error(std::format("Failed to load {}. Reason:\n\t{}\n\t{}", info->filepath, warn, err));
	}

	MeshData* meshData = new MeshData;
	std::unordered_map<vkMesh::Vertex, uint32_t> vertexLookup{};
	for (const tinyobj::shape_t& shape : shapes) {
		for (const tinyobj::index_t& index : shape.mesh.indices) {
			vkMesh::Vertex vertex{};

			vertex.pos = info->preTransform * glm::vec3(
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			);

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal = info->preTransform * glm::vec3(
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			);

			if (vertexLookup.count(vertex) == 0) {
				vertexLookup[vertex] = static_cast<uint32_t>(meshData->vertices.size());
				meshData->vertices.push_back(vertex);
			}

			meshData->indices.push_back(vertexLookup[vertex]);
		}
	}
	return meshData;
}

inline void vkMesh::loadVertexBuffer(
	const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::CommandBuffer& commandBuffer, const vk::Queue graphicsQueue,
	const std::vector<const MeshData*&>& meshesData, const std::vector<const vkImage::Texture*&>& textures, const std::vector<std::string>& types,
	std::unordered_map<std::string, Mesh>& meshes, MeshBuffer& meshBuffer
) {
	int indexOffset = 0;
	std::vector<float> vertexLump;
	std::vector<uint32_t> indexLump;

	for (uint32_t i = 0; i <= meshesData.size(); i++) {
		meshes.insert(
			std::make_pair(
				types[i],
				Mesh{
					static_cast<uint32_t>(indexLump.size()),
					static_cast<uint32_t>(meshesData[i]->indices.size()),
					*textures[i]
				}
			)
		);
		for (Vertex vertex : meshesData[i]->vertices) {
			const glm::vec3& p = vertex.pos;
			const glm::vec2& t = vertex.texCoord;
			const glm::vec3& n = vertex.normal;

			vertexLump.push_back(p.x);
			vertexLump.push_back(p.y);
			vertexLump.push_back(p.z);

			vertexLump.push_back(t.x);
			vertexLump.push_back(t.y);

			vertexLump.push_back(n.x);
			vertexLump.push_back(n.y);
			vertexLump.push_back(n.z);
		}
		for (uint32_t index : meshesData[i]->indices) {
			indexLump.push_back(index + indexOffset);
		}
		indexOffset += static_cast<uint32_t>(meshesData[i]->vertices.size() / 8);
		delete meshesData[i];
	}

	uint32_t vertexLumpSize = sizeof(float) * vertexLump.size();
	Buffer stagingBuffer;
	vkUtil::createBuffer(
		device, physicalDevice, vertexLumpSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer
	);
	void* memoryLocation = device.mapMemory(stagingBuffer.bufferMemory, 0, vertexLumpSize);
	memcpy(memoryLocation, vertexLump.data(), vertexLumpSize);
	device.unmapMemory(stagingBuffer.bufferMemory);

	vkUtil::createBuffer(
		device, physicalDevice, vertexLumpSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal, meshBuffer.vertexBuffer
	);
	vkUtil::copyBuffer(stagingBuffer, meshBuffer.vertexBuffer, vertexLumpSize, graphicsQueue, commandBuffer);

	device.destroyBuffer(stagingBuffer.buffer);
	device.freeMemory(stagingBuffer.bufferMemory);

	uint32_t indexLumpSize = sizeof(uint32_t) * indexLump.size();
	vkUtil::createBuffer(
		device, physicalDevice, indexLumpSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer
	);
	memoryLocation = device.mapMemory(stagingBuffer.bufferMemory, 0, indexLumpSize);
	memcpy(memoryLocation, indexLump.data(), indexLumpSize);
	device.unmapMemory(stagingBuffer.bufferMemory);

	meshBuffer.indexBuffer;
	vkUtil::createBuffer(device, physicalDevice, indexLumpSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal, meshBuffer.indexBuffer
	);
	vkUtil::copyBuffer(
		stagingBuffer, meshBuffer.indexBuffer, indexLumpSize, graphicsQueue, commandBuffer
	);

	device.destroyBuffer(stagingBuffer.buffer);
	device.freeMemory(stagingBuffer.bufferMemory);
}
