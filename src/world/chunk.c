#include <stdio.h>
#include <stdlib.h>
#include <raymath.h>
#include <rlgl.h>
#include "chunk.h"
#include "../memory.h"

#define STB_PERLIN_IMPLEMENTATION
#include "../thirdparty/stb_perlin.h"


#include "../thirdparty/MarchingCubesTables.h"


// Reference for marching cubes algorithm was taken from:
// https://github.com/islamhaqq/MarchingCubesTables.h



static
void march
(
    struct world_chunk* chunk,
    float* cube,
    Vector3* positions, 
    float isolevel,
    struct triangle* triangles,
    uint32_t* num_triangles
){
    
    int cube_index = 0;

    // Get cube index in the edge table.
    if(cube[0] < isolevel) { cube_index |= 1; }
    if(cube[1] < isolevel) { cube_index |= 2; }
    if(cube[2] < isolevel) { cube_index |= 4; }
    if(cube[3] < isolevel) { cube_index |= 8; }
    if(cube[4] < isolevel) { cube_index |= 16; }
    if(cube[5] < isolevel) { cube_index |= 32; }
    if(cube[6] < isolevel) { cube_index |= 64; }
    if(cube[7] < isolevel) { cube_index |= 128; }

    if(EdgeMasks[cube_index] == 0) {
        return;
    }

    // Find vertices where the surface intersects the cube.
    Vector3 vertices[12] = { 0 };
    int num_vertices = 0;
    for(int i = 0; i < 12; i++) {
        if(EdgeMasks[cube_index] & (1 << i)) {
            
            int i0 = EdgeVertexIndices[i][0];
            int i1 = EdgeVertexIndices[i][1];

            
            float t = (isolevel - cube[i0]) / (cube[i1] - cube[i0]);

            vertices[i] = Vector3Lerp(positions[i0], positions[i1], t);

            /*
            vertices[i].x = (0.5 * (positions[i0].x + positions[i1].x));
            vertices[i].y = (0.5 * (positions[i0].y + positions[i1].y));
            vertices[i].z = (0.5 * (positions[i0].z + positions[i1].z));
            */
        }
    }


    // Create triangles.
    for(int i = 0; i < 16; i += 3) {
        int k = TriangleTable[cube_index][i];
        if(k < 0) {
            break;
        }
        
        struct triangle* tr = &triangles[*num_triangles];

        tr->a = vertices[ TriangleTable[ cube_index ][i] ];
        tr->b = vertices[ TriangleTable[ cube_index ][i+1] ];
        tr->c = vertices[ TriangleTable[ cube_index ][i+2] ];
    
        *num_triangles += 1;
    }
}



static
float perlin_noise(Vector3 p, float freq) {
    return stb_perlin_noise3(
            ((p.x * freq) / (float)CHUNK_SIZE),
            ((p.y * freq) / (float)CHUNK_SIZE),
            ((p.z * freq) / (float)CHUNK_SIZE),
            0,
            0,
            0);
}

static
float get_noise(Vector3 p) {
    return perlin_noise(p, 2.0);
}

void generate_chunk(struct world_chunk* chunk, int chunk_x, int chunk_y, int chunk_z) {

    size_t num_cubes_max = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

    chunk->mesh.triangleCount = num_cubes_max * 4;
    chunk->mesh.vertexCount   = chunk->mesh.triangleCount * 3;

    chunk->mesh.vertices  = calloc(chunk->mesh.vertexCount, 3 * sizeof(float));
    chunk->mesh.normals   = calloc(chunk->mesh.vertexCount, 3 * sizeof(float));
    chunk->mesh.texcoords = NULL;
    chunk->mesh.indices   = NULL;

    chunk->grid_x = chunk_x;
    chunk->grid_y = chunk_y;
    chunk->grid_z = chunk_z;
    
    chunk->world_pos = (Vector3) {
        (float)chunk->grid_x * CHUNK_SIZE,
        (float)chunk->grid_y * CHUNK_SIZE,
        (float)chunk->grid_z * CHUNK_SIZE
    };

    size_t total_vertices = 0;
    size_t total_normals = 0;

    for(int z = 0; z < CHUNK_SIZE; z++) {
        for(int y = 0; y < CHUNK_SIZE; y++) {
            for(int x = 0; x < CHUNK_SIZE; x++) {

                float cube[8] = { 0 };
                Vector3 positions[8] = { 0 };

                Vector3 p = (Vector3) {
                    (float)x + chunk->world_pos.x,
                    (float)y + chunk->world_pos.y,
                    (float)z + chunk->world_pos.z
                };


                positions[0] = (Vector3){ p.x,     p.y,     p.z     };
                positions[1] = (Vector3){ p.x + 1, p.y,     p.z     };
                positions[2] = (Vector3){ p.x,     p.y + 1, p.z     };
                positions[3] = (Vector3){ p.x + 1, p.y + 1, p.z     };
                positions[4] = (Vector3){ p.x,     p.y,     p.z + 1 };
                positions[5] = (Vector3){ p.x + 1, p.y,     p.z + 1 };
                positions[6] = (Vector3){ p.x,     p.y + 1, p.z + 1 };
                positions[7] = (Vector3){ p.x + 1, p.y + 1, p.z + 1 };

                for(int i = 0; i < 8; i++) {
                    cube[i] = get_noise(positions[i]);
                }


                struct triangle triangles[8] = { 0 };
                uint32_t num_triangles = 0;

                const float isolevel = 0.001f;
                march(chunk, cube, positions, isolevel, triangles, &num_triangles);

                if(num_triangles == 0) {
                    continue;
                }

                for(uint32_t i = 0; i < num_triangles; i++) {
                    struct triangle* tr = &triangles[i];

                    chunk->mesh.vertices[total_vertices+0] = tr->a.x;
                    chunk->mesh.vertices[total_vertices+1] = tr->a.y;
                    chunk->mesh.vertices[total_vertices+2] = tr->a.z;

                    chunk->mesh.vertices[total_vertices+3] = tr->b.x;
                    chunk->mesh.vertices[total_vertices+4] = tr->b.y;
                    chunk->mesh.vertices[total_vertices+5] = tr->b.z;

                    chunk->mesh.vertices[total_vertices+6] = tr->c.x;
                    chunk->mesh.vertices[total_vertices+7] = tr->c.y;
                    chunk->mesh.vertices[total_vertices+8] = tr->c.z;



                    Vector3 vN = Vector3CrossProduct( 
                            Vector3Subtract(tr->b, tr->a), 
                            Vector3Subtract(tr->c, tr->a));

                    vN = Vector3Normalize(vN);

                    chunk->mesh.normals[total_vertices+0] = vN.x;
                    chunk->mesh.normals[total_vertices+1] = vN.y;
                    chunk->mesh.normals[total_vertices+2] = vN.z;

                    chunk->mesh.normals[total_vertices+3] = vN.x;
                    chunk->mesh.normals[total_vertices+4] = vN.y;
                    chunk->mesh.normals[total_vertices+5] = vN.z;

                    chunk->mesh.normals[total_vertices+6] = vN.x;
                    chunk->mesh.normals[total_vertices+7] = vN.y;
                    chunk->mesh.normals[total_vertices+8] = vN.z;

                    total_vertices += 9;
                }
            }
        }
    }

    printf("Chunk total vertices: %li / %i\n", total_vertices, chunk->mesh.vertexCount);

    // TODO: Take a look at marching tetrahedra algorithm.

    chunk->mesh.vertexCount   = total_vertices;
    chunk->mesh.triangleCount = total_vertices / 3;

    UploadMesh(&chunk->mesh, true);
}

void free_chunk(struct world_chunk* chunk) {
    UnloadMesh(chunk->mesh);
}

void render_chunk(struct world_chunk* chunk, Material mat) {

    //rlDisableBackfaceCulling();
    //rlEnableWireMode();
    Matrix translation = MatrixTranslate(
            chunk->world_pos.x,
            chunk->world_pos.y,
            chunk->world_pos.z);

    DrawMesh(chunk->mesh, mat, translation);

    //rlDisableWireMode();
    //rlEnableBackfaceCulling();
   
}

