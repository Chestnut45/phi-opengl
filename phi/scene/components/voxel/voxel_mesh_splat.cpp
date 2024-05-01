#include "voxel_mesh_splat.hpp"

#include <phi/core/logging.hpp>
#include <phi/graphics/geometry.hpp>
#include <phi/graphics/indirect.hpp>
#include <phi/scene/node.hpp>

namespace Phi
{
    VoxelMeshSplat::VoxelMeshSplat(const std::vector<VertexVoxel>& voxels)
    {
        if (refCount == 0)
        {
            // Initialize static resources

            shader = new Shader();
            shader->LoadSource(GL_VERTEX_SHADER, "phi://graphics/shaders/voxel_mesh_splat.vs");
            shader->LoadSource(GL_FRAGMENT_SHADER, "phi://graphics/shaders/voxel_mesh_splat.fs");
            shader->Link();

            vertexBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, sizeof(VertexVoxel) * MAX_VOXELS);
            meshDataBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, (sizeof(glm::mat4) + sizeof(glm::mat3) * 2) * MAX_DRAW_CALLS);
            indirectBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, sizeof(DrawArraysCommand) * MAX_DRAW_CALLS);

            // Initialize with base vertex attributes
            vao = new VertexAttributes(VertexFormat::VOXEL, vertexBuffer);

            // Add per-mesh data via instanced array attributes
            vao->Bind();
            meshDataBuffer->Bind(GL_ARRAY_BUFFER);
            
            // Add the attributes for the model matrix
            // Attributes can only hold 4 components so we use 4 consecutive
            // vec4s and then consume them in the vertex shader as a single matrix
            vao->AddAttribute(4, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, 0);
            vao->AddAttribute(4, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4));
            vao->AddAttribute(4, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 2);
            vao->AddAttribute(4, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 3);

            // Add the attributes for the rotation matrix
            vao->AddAttribute(3, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 4);
            vao->AddAttribute(3, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 4 + sizeof(glm::vec3));
            vao->AddAttribute(3, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 4 + sizeof(glm::vec3) * 2);

            // And the inverse rotation matrix
            vao->AddAttribute(3, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 4 + sizeof(glm::vec3) * 3);
            vao->AddAttribute(3, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 4 + sizeof(glm::vec3) * 4);
            vao->AddAttribute(3, GL_FLOAT, 1, sizeof(glm::mat4) + sizeof(glm::mat3) * 2, sizeof(glm::vec4) * 4 + sizeof(glm::vec3) * 5);

            // Debug Logging
            Phi::Log("VoxelMeshSplat resources initialized");
        }
        
        // Copy voxel data
        vertices = voxels;

        refCount++;
    }

    VoxelMeshSplat::~VoxelMeshSplat()
    {
        refCount--;

        if (refCount == 0)
        {
            // Cleanup static resources
            delete shader;
            delete vertexBuffer;
            delete meshDataBuffer;
            delete indirectBuffer;
            delete vao;
        }
    }

    void VoxelMeshSplat::Render(const glm::mat4& transform, const glm::mat3& rotation)
    {
        // Check to ensure the buffers can contain this mesh currently
        if (meshDrawCount >= MAX_DRAW_CALLS || vertexDrawCount + vertices.size() >= MAX_VOXELS)
        {
            FlushRenderQueue();
        }

        // Sync the buffers if this is the first draw call since last render
        // If this has been signaled, the other buffers must also not be being read from
        if (meshDrawCount == 0) indirectBuffer->Sync();

        // Create the indirect draw command
        DrawArraysCommand cmd;
        cmd.count = vertices.size();
        cmd.instanceCount = 1;
        cmd.first = vertexDrawCount + (MAX_VOXELS * vertexBuffer->GetCurrentSection());
        cmd.baseInstance = meshDrawCount + (MAX_DRAW_CALLS * meshDataBuffer->GetCurrentSection());

        // Write the draw command to the buffer
        indirectBuffer->Write(cmd);

        // Write the vertices to the buffer
        vertexBuffer->Write(vertices.data(), vertices.size() * sizeof(VertexVoxel));

        // Write the per-mesh matrices
        meshDataBuffer->Write(transform);
        meshDataBuffer->Write(rotation);
        meshDataBuffer->Write(glm::inverse(rotation));

        // Increase internal counters
        meshDrawCount++;
        vertexDrawCount += vertices.size();
    }

    void VoxelMeshSplat::FlushRenderQueue()
    {
        // Ensure we only flush if necessary
        if (meshDrawCount == 0) return;

        // Bind the relevant resources
        vao->Bind();
        shader->Use();
        indirectBuffer->Bind(GL_DRAW_INDIRECT_BUFFER);

        // Issue the multi draw command
        glMultiDrawArraysIndirect(GL_POINTS, (void*)(indirectBuffer->GetCurrentSection() * indirectBuffer->GetSize()), meshDrawCount, 0);
        vao->Unbind();

        // Set a lock and reset buffers
        indirectBuffer->Lock();
        indirectBuffer->SwapSections();
        meshDataBuffer->SwapSections();
        vertexBuffer->SwapSections();

        // Reset counters
        meshDrawCount = 0;
        vertexDrawCount = 0;
    }
}