//
// Created by luc on 28/10/22.
//
#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "Model.h"
#include "external/objloader/tiny_obj_loader.h"
#include <glm/gtx/hash.hpp>

namespace std {
    template <>
    struct hash<vtt::Model::Vertex> {
        size_t operator()(vtt::Model::Vertex const &vertex) const {
            size_t seed = 0;
            lve::hashCombine(seed, vertex.pos, vertex.color, vertex.texCoord);
            return seed;
        }
    };
}

namespace vtt {

    Model::Model(Device &device, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices): m_deviceRef(device) {
        m_hasIndexBuffer = true;
        createVertexBuffer(vertices);
        createIndexBuffer(indices);
    }

    Model::Model(Device &device, std::vector<Vertex>& vertices): m_deviceRef(device) {
        createVertexBuffer(vertices);
    }

    void Model::createVertexBuffer(std::vector<Vertex>& vertices){
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        m_vertexCount = static_cast<uint32_t>(vertices.size());

        vtt::Buffer stagingBuffer(m_deviceRef, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.singleWrite(vertices.data());

        m_vertexBuffer = std::make_unique<vtt::Buffer>(m_deviceRef, bufferSize,
                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_deviceRef.copyBuffer(stagingBuffer.get(), m_vertexBuffer->get(), bufferSize);
    }

    void Model::createIndexBuffer(std::vector<uint32_t>& indices){
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
        m_indexCount = static_cast<uint32_t>(indices.size());

        vtt::Buffer stagingBuffer(m_deviceRef, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.singleWrite(indices.data());

        m_indexBuffer = std::make_unique<vtt::Buffer>(m_deviceRef, bufferSize,
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_deviceRef.copyBuffer(stagingBuffer.get(), m_indexBuffer->get(), bufferSize);
    }

    std::unique_ptr<Model> Model::createModelFromFile(Device &device, const std::string &filepath) {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape: shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                };
                vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };
                vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                };

                if (uniqueVertices.count(vertex) == 0){
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
        return std::make_unique<Model>(device, vertices, indices);

    }

    void Model::bind(VkCommandBuffer commandBuffer) {
        VkBuffer vertexBuffers[] = {m_vertexBuffer->get()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        if (m_hasIndexBuffer)
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->get(), 0, VK_INDEX_TYPE_UINT32);
    }

    void Model::draw(VkCommandBuffer commandBuffer) const {
        if (m_hasIndexBuffer)
            vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, m_vertexCount, 0, 0, 0);
    }

    VkVertexInputBindingDescription Model::Vertex::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    std::array<VkVertexInputAttributeDescription, 3> Model::Vertex::getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);


        return attributeDescriptions;
    }


}