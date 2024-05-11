#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <phi/simulation/voxel_chunk.hpp>
#include <phi/simulation/voxel_volume.hpp>
#include <phi/scene/scene.hpp>

namespace Phi
{
    // Represents a voxel world and all of its nodes / components
    // Provides an interface to the internal scene instance
    class VoxelWorld
    {
        // Interface
        public:

            // Creates an empty voxel world with default settings
            VoxelWorld();
            ~VoxelWorld();

            // Delete copy constructor/assignment
            VoxelWorld(const VoxelWorld&) = delete;
            VoxelWorld& operator=(const VoxelWorld&) = delete;

            // Delete move constructor/assignment
            VoxelWorld(VoxelWorld&& other) = delete;
            VoxelWorld& operator=(VoxelWorld&& other) = delete;

            // Generation

            // Loads voxel materials from a YAML file
            // TODO: Should only accept voxel materials!
            void LoadMaterials(const std::string& path);

            // Adds a volume to the world
            void AddVolume(const VoxelVolume& volume);

            // Gets the list of terrain volumes
            std::vector<VoxelVolume>& GetVolumes() { return terrainVolumes; }

            // DEBUG: Regenerates the terrain
            void Regenerate();

            // Simulation

            // Updates the voxel world with the given elapsed time in seconds
            void Update(float delta);

            // Rendering

            // Renders the voxel world to the current framebuffer
            void Render();

            // Full access to the voxel world's internal scene instance
            // TODO: Restrict more? How much access is actually required by the user at this level?
            Scene& GetScene() { return scene; };

        // Data / implementation
        private:

            // Internal scene instance
            Scene scene;

            // Map of loaded chunks
            // TODO: Switch to Phi::HashMap when impl finished (Profile!)
            std::unordered_map<glm::ivec3, VoxelChunk> loadedChunks;

            // Generation

            // List of all volumes that make up the terrain
            std::vector<VoxelVolume> terrainVolumes;

            // TODO: Biomes, features...
    };
}