#pragma once

#include "imgui.h"

namespace Phi
{
    // Forward declares
    class Node;
    class Scene;
    
    // Base abstract class for all components
    class BaseComponent
    {
        // Interface
        public:

            // Access to the node this component belongs to
            Node* GetNode() const { return node; };

            // Common component interface
            // Must be implemented by any derived component type
            // virtual void Init() = 0;
            // virtual void Update(float delta) = 0;
            virtual void InspectorGUI() = 0;
        
        protected:

            // Pointer to the node that this component belongs to
            // NOTE: Set automatically during Node::AddComponent<T>(...)
            Node* node = nullptr;
            friend class Node;
    };
}
