# Capibara Engine
## Our Project
Capibara Engine is a project developed in the subject 3D Motors for the Game Design and Development Degree in CITM-UPC Univerisity.
## Team
#### Arnau Bonada: https://github.com/arnaubonada
#### Albert Pou: https://github.com/Hydeon-git
#### Pol Pallares: https://github.com/Zeta115
This project was forked from https://github.com/solidajenjo/Engine3D

## Instructions
### Transformation and Game Object hierarchy
- Hierarchy: User can delete, reparent, create empty and create children.
- Transform: User can translate, rotate and scale Game Objects.
- Mesh: User can select or drop any imported mesh.
- Texture: User can select or drop any imported texture.

### Game viewport & editor viewport
- The editor has 2 windows: one with the scene (editor view) and another captured from a Game Object with a Camera component (game view).
- Each window has its own framebuffer and the user can visualize both at the same time.

### Scene serialization
- Scene can be serialized to a file that can be saved.
- 'Street Environment' is automatically loaded at the start.

### Camera
- Component: Camera is a component with settings that can be modified. Its boundaries can be visualized in the editor.
- Frustum culling: All meshes use a bounding volume (AABB or OBB) and can be discarded using Frustum Culling.
- Mouse pick: Game Objects can be picked from the world using the mouse.