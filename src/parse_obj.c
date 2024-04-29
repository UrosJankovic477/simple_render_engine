#include "parse_obj.h"

sre_obj_token *token_table_obj[32] = {0};
sre_obj_token *token_table_mtl[32] = {0};

sre_obj_token *undefined_node;
        

sre_material *material_table[SRE_MAX_NUM_OF_MTLS] = {0};



int add_material(sre_material *material)
{
    uint16_t hash = get_string_hash16(material->name);
    sre_material *node = material_table[hash];
    if (node == NULL)
    {
        material->id = hash;
        material->collision = NULL;
        material_table[hash] = material;
        return EXIT_SUCCESS;
    }
    sre_material *prev;
    while (node)
    {
        prev = node;
        node = node->collision;
    }
    prev->collision = material;
    material->collision = NULL;
    material->id = hash;

    return EXIT_SUCCESS;
}

sre_material* get_material(unsigned char *name)
{
    uint16_t hash = get_string_hash16(name);
    sre_material *node = material_table[hash];
    while (node && strcmp(node->name, name) != 0)
    {
        node = node->collision;
    }
    return node;
}

unsigned char get_token_hash(const char *token)
{
    char hash = 0;
    char c;
    while (c = *token++)
    {
        hash = hash ^ c;

    }
        
    return hash & 0x1f; // hash % 32
}



int add_token(sre_obj_token** token_table, const char *token, sre_obj_token_enum type)
{
    unsigned char hash = get_token_hash(token);
    sre_obj_token *node = token_table[hash];
    if (node == NULL)
    {
        sre_obj_token *new_node = (sre_obj_token*)malloc(sizeof(sre_obj_token));
        new_node->key = token;
        new_node->index = hash;
        new_node->collision = NULL;
        new_node->type = type;
        token_table[hash] = new_node;
        return EXIT_SUCCESS;
    }
    
    sre_obj_token *prev;
    while (node)
    {
        prev = node;
        node = node->collision;
    }
    prev->collision = (sre_obj_token*)malloc(sizeof(sre_obj_token));
    prev->collision->key = token;
    prev->collision->index = hash;
    prev->collision->collision = NULL;
    prev->collision->type = type;
    return EXIT_SUCCESS;
    
}

sre_obj_token* get_token(sre_obj_token** token_table, const char *token)
{
    unsigned char hash = get_token_hash(token);
    sre_obj_token *node = token_table[hash];
    while (node && strcmp(node->key, token) != 0)
    {
        node = node->collision;
    }
    if (!node)
    {
        return undefined_node;
    }
    return node;
}

void init_parser() {
    undefined_node = malloc(sizeof(sre_obj_token));
    undefined_node->type = UNDEFINED;

    init_obj_parser();
    init_mtl_parser();
}

void destroy_material_table()
{
    for (size_t i = 0; i < SRE_MAX_NUM_OF_MTLS; i++)
    {
        if (material_table[i])
        {
            delete_material(material_table[i]);
            material_table[i] = NULL;
        }
    }
    
}

void delete_material(sre_material *mtl)
{
    if (mtl->collision)
    {
        delete_material(mtl->collision);
    }
    
    free(mtl->name);
    free(mtl->map_Ka);
    free(mtl->map_Kd);
    free(mtl->map_Ke);
    free(mtl->map_Ka);
    free(mtl);
}

void delete_model_obj(sre_model_obj *model)
{
    for (size_t i = 0; i < model->mesh_count; i++)
    {
        delete_mesh_obj(model->meshes[i]);
    }
}

void delete_mesh_obj(sre_mesh_obj *mesh)
{
    free(mesh->groups);
    free(mesh->indecies[0]);
    free(mesh->indecies[1]);
    free(mesh->indecies[2]);
    free(mesh->normal);
    free(mesh->position);
    for (size_t i = 0; i < 8; i++)
    {
        free(mesh->uv[i]);
    }
    
    free(mesh);
}

void init_obj_parser()
{
    add_token(token_table_obj, "#", COMMENT);
    add_token(token_table_obj, "o", OBJECT);
    add_token(token_table_obj, "v", VERTEX_SPACE_COORDINATES);
    add_token(token_table_obj, "f", FACES);
    add_token(token_table_obj, "vt", VERTEX_TEXTURE_COORDINATES);
    add_token(token_table_obj, "vn", VERTEX_NORMALS);
    add_token(token_table_obj, "usemtl", USE_MATERIAL); 
    add_token(token_table_obj, "mtllib", MATERIAL_LIBRARY); 
    add_token(token_table_obj, "g", GROUP);
    add_token(token_table_obj, "lod", LOD);
}

void init_mtl_parser()
{
    add_token(token_table_mtl, "#", COMMENT);
    add_token(token_table_mtl, "newmtl", MATERIAL);\
    add_token(token_table_mtl, "Ka", K_AMBIENT);
    add_token(token_table_mtl, "Kd", K_DIFFUSE);
    add_token(token_table_mtl, "Ks", K_SPECULAR);
    add_token(token_table_mtl, "Ke", K_EMISSION);
    add_token(token_table_mtl, "Ns", EXP_SPECULAR);
    add_token(token_table_mtl, "map_Ka", MAP_AMBIENT);
    add_token(token_table_mtl, "map_Kd", MAP_DIFFUSE);
    add_token(token_table_mtl, "map_Ks", MAP_SPECULAR);
    add_token(token_table_mtl, "map_Ke", MAP_EMISSION);
    add_token(token_table_mtl, "map_Ns", MAP_SPECULAR_EXP);
}

int process_obj_file(const char *p_file_path, sre_model_obj *p_model)
{
    FILE *obj_file_p = fopen(p_file_path, "rb");
    if (!obj_file_p)
    {
        fprintf(stderr, "Can't open %s\n", p_file_path);
    }

    p_model->mesh_count = 0;
    int max_mesh_count = 1;
    int max_material_count = 4;

    unsigned int nv = 0;
    unsigned int nvt = 0;
    unsigned int nvn = 0;

    p_model->meshes = malloc(sizeof(sre_mesh_obj));

    char *directory = calloc(256, sizeof(char));
    unsigned long long last_slash = strrchr(p_file_path, '/');
    if (last_slash)
    {
        strncpy_s(directory, 256*sizeof(char), p_file_path, (last_slash - (unsigned long long)p_file_path) / sizeof(char) + 1);
    }
    
    char buffer[1028];
    while (fgets(buffer, sizeof(buffer), obj_file_p))   
    {
        char *token = strtok(buffer, " ");
        sre_obj_token *obj_tok = get_token(token_table_obj, token);

        switch (obj_tok->type)
        {
        case COMMENT:
            continue;
            break;
        case OBJECT:
        {
            if (p_model->mesh_count >= max_mesh_count)
            {
                max_mesh_count = max_mesh_count << 1;
                p_model->meshes = realloc(p_model->meshes, max_mesh_count * sizeof(sre_mesh_obj));
            }
            fpos_t position;
            fgetpos(obj_file_p, &position);
            p_model->meshes[p_model->mesh_count] = malloc(sizeof(sre_mesh_obj));
            int n_meshes;

            int status = process_obj_mesh(obj_file_p, 
                &position, 
                p_model->meshes[p_model->mesh_count++]);

            if (status == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }
            fsetpos(obj_file_p, &position);
            break;
        }
        case MATERIAL_LIBRARY:
        {
            char *mtl_name = strtok(NULL, " \n");
            char mtl_path[256];

            strcpy_s(mtl_path, 256, directory);
            strcat_s(mtl_path, 256, mtl_name);

            int status = procces_mtl_file(mtl_path);
            break;
        }
        default:
            break;
        }
    }
    free(directory);
    return EXIT_SUCCESS;
}

int process_obj_mesh(FILE *p_file, 
    fpos_t *position, 
    sre_mesh_obj *p_mesh)
{
    p_mesh->position_count = 0;
    p_mesh->uv_count = 0;
    p_mesh->normals_count = 0;

    for (size_t i = 0; i < 8; i++)
    {
        p_mesh->uv[i] = NULL;
    }
    
    int max_indecies_num = 0x80;
    int max_group_count = 0x10;
    int material_count = 0;
    p_mesh->indecies_count = 0;
    p_mesh->indecies[0] = malloc(max_indecies_num * sizeof(int));
    p_mesh->indecies[1] = malloc(max_indecies_num * sizeof(int));
    p_mesh->indecies[2] = malloc(max_indecies_num * sizeof(int));
    p_mesh->group_count = 0;
    
    p_mesh->groups = malloc(max_group_count * sizeof(sre_group_obj));

    p_mesh->indecies_count = 0;
    int num_v = 0;
    int num_vt = 0;
    int num_vn = 0;

    char buffer[1028];

    fsetpos(p_file, position);
    int end_of_object = 0;
    
    while (fgets(buffer, sizeof(buffer), p_file) && !end_of_object)
    {
        char *token = strtok(buffer, " ");
        sre_obj_token *obj_tok = get_token(token_table_obj, token);
        switch (obj_tok->type)
        {
        case COMMENT:
            continue;
            break;
        case OBJECT:
            end_of_object = 1;
            break;
        case VERTEX_SPACE_COORDINATES:
            num_v++;
            break;
        case VERTEX_TEXTURE_COORDINATES:
            num_vt++;
            break;
        case VERTEX_NORMALS:
            num_vn++;
            break;
        case FACES:
        {
            if (p_mesh->indecies_count == SRE_MAX_INDECIES_COUNT)
            {
                return EXIT_FAILURE;
            }
            
            if (p_mesh->indecies_count >= max_indecies_num - 2)
            {
                max_indecies_num = max_indecies_num << 1;
                p_mesh->indecies[0] = realloc(p_mesh->indecies[0], max_indecies_num * sizeof(int));
                p_mesh->indecies[1] = realloc(p_mesh->indecies[1], max_indecies_num * sizeof(int));
                p_mesh->indecies[2] = realloc(p_mesh->indecies[2], max_indecies_num * sizeof(int));
            }
            
            const char *indecies_str[3];
            indecies_str[0] = strtok(NULL, " ");
            indecies_str[1] = strtok(NULL, " ");
            indecies_str[2] = strtok(NULL, " ");
            for (size_t i = 0; i < 3; i++)
            {
                int idx = atoi(strtok(indecies_str[i], "/"));
                p_mesh->indecies[0][p_mesh->indecies_count] = idx - p_mesh->position_count;
                p_mesh->indecies[1][p_mesh->indecies_count] = atoi(strtok(NULL, "/")) - p_mesh->uv_count;
                p_mesh->indecies[2][p_mesh->indecies_count++] = atoi(strtok(NULL, "/")) - p_mesh->normals_count;
            }
            break;
        }
        case GROUP:
        {
            
            break;
        }
        case USE_MATERIAL:
        {
            if (p_mesh->group_count == max_group_count)
            {
                max_group_count = max_group_count << 1;
                p_mesh->groups = realloc(p_mesh->groups, max_group_count * sizeof(sre_group_obj));
            }
            
            const char *mtl_name = strtok(NULL, " \n");
            p_mesh->groups[p_mesh->group_count].index = -1;
            p_mesh->groups[(p_mesh->group_count - 1u) & (max_group_count - 1u)].index = p_mesh->indecies_count;
            
            p_mesh->groups[p_mesh->group_count].material = get_material(mtl_name);
            if (!p_mesh->groups[p_mesh->group_count].material)
            {
                p_mesh->groups[p_mesh->group_count].material_name = malloc(sizeof(mtl_name));
                strcpy(p_mesh->groups[p_mesh->group_count].material_name, mtl_name);
            }
            p_mesh->group_count += 1;

            break;
        }
        default:
            break;
        }
    }

    p_mesh->indecies[0] = realloc(p_mesh->indecies[0], p_mesh->indecies_count * sizeof(int));
    p_mesh->indecies[1] = realloc(p_mesh->indecies[1], p_mesh->indecies_count * sizeof(int));
    p_mesh->indecies[2] = realloc(p_mesh->indecies[2], p_mesh->indecies_count * sizeof(int));

    p_mesh->groups = realloc(p_mesh->groups, p_mesh->group_count * sizeof(sre_group_obj));

    p_mesh->position = malloc(num_v * sizeof(sre_float_vec3));
    p_mesh->normal = malloc(num_vn * sizeof(sre_2_10_10_10s));
    p_mesh->uv[0] = malloc(num_vt * sizeof(num_vt));

    p_mesh->position_count += num_v;
    p_mesh->uv_count += num_vt;
    p_mesh->normals_count += num_vn;

    int space_coor_num = 0;
    int normals_num = 0;
    int uv_num = 0;


    fsetpos(p_file, position);

    while (fgets(buffer, sizeof(buffer), p_file))
    {
        char *token = strtok(buffer, " ");
        sre_obj_token *obj_tok = get_token(token_table_obj, token);
        switch (obj_tok->type)
        {
        case COMMENT:
            continue;
            break;
        case OBJECT:
            return ftell(p_file);
            break;
        case VERTEX_SPACE_COORDINATES:
            {
                // TODO: add vertex colors support
                p_mesh->position[space_coor_num].x = atof(strtok(NULL, " "));
                p_mesh->position[space_coor_num].y = atof(strtok(NULL, " "));
                p_mesh->position[space_coor_num++].z = atof(strtok(NULL, " "));
                break;
            }
        case VERTEX_TEXTURE_COORDINATES:
            {
                char *u_str = strtok(NULL, " \n");
                char *v_str = strtok(NULL, " \n");
                p_mesh->uv[0][uv_num].x = float_to_norm_16(atof(u_str));
                p_mesh->uv[0][uv_num++].y = float_to_norm_16(atof(v_str));
                break;
            }
        case VERTEX_NORMALS:
            {
                float nx = atof(strtok(NULL, " "));
                float ny = atof(strtok(NULL, " "));
                float nz = atof(strtok(NULL, " "));
                p_mesh->normal[normals_num++] = SRE_Float_to_2_10_10_10s(nx, ny, nz);
                break;
            }
        case FACES:
            continue;
            break;
        default:
            break;
        }
    }
    fgetpos(p_file, position);
    return EXIT_SUCCESS;
}

int procces_mtl_file(const char *p_file_path)
{
    FILE *mtl_file_p = fopen(p_file_path, "r");
    if (!mtl_file_p)
    {
        fprintf(stderr, "Can't open %s\n", p_file_path);
    }
    sre_material *current_mtl = NULL;
    char buffer[1028];
    while (fgets(buffer, sizeof(buffer), mtl_file_p))
    {
        char *token = strtok(buffer, " ");
        sre_obj_token *mtl_tok = get_token(token_table_mtl, token);
        
        switch (mtl_tok->type)
        {
        case COMMENT:
            continue;
            break;
        case MATERIAL:
        {
            sre_material *material = calloc(1, sizeof(sre_material));
            char *mtl_name = strtok(NULL, " \n");
            size_t size = sizeof(char)*strlen(mtl_name) + 1;
            material->name = malloc(sizeof(char)*(size));
            strcpy_s(material->name, size, mtl_name);
            add_material(material);
            current_mtl = material;
            break;
        }
        case EXP_SPECULAR:
        {
            current_mtl->Ns = atof(strtok(NULL, " "));
            break;
        }
        case K_AMBIENT:
        {
            float r = atof(strtok(NULL, " "));
            float g = atof(strtok(NULL, " "));
            float b = atof(strtok(NULL, " "));

            current_mtl->Ka = SRE_Vec3_to_rgb(r, g, b);
            break;
        }
        case K_DIFFUSE:
        {
            float r = atof(strtok(NULL, " "));
            float g = atof(strtok(NULL, " "));
            float b = atof(strtok(NULL, " "));

            current_mtl->Kd = SRE_Vec3_to_rgb(r, g, b);
            break;
        }
        case K_SPECULAR:
        {
            float r = atof(strtok(NULL, " "));
            float g = atof(strtok(NULL, " "));
            float b = atof(strtok(NULL, " "));

            current_mtl->Ks = SRE_Vec3_to_rgb(r, g, b);
            break;
        }
        case K_EMISSION:
        {
            float r = atof(strtok(NULL, " "));
            float g = atof(strtok(NULL, " "));
            float b = atof(strtok(NULL, " "));

            current_mtl->Ke = SRE_Vec3_to_rgb(r, g, b);
            break;
        }
        case MAP_AMBIENT:
        {
            char *map_fname = strtok(NULL, " \n");
            char map_path[256] = "../resources/textures/";
            strcat_s(map_path, 256, map_fname);
            current_mtl->map_Ka = malloc(sizeof(sre_texture));
            SRE_Load_texture(map_path, current_mtl->map_Ka);

            break;
        }
        case MAP_DIFFUSE:
        {
            char *map_fname = strtok(NULL, " \n");
            char map_path[256] = "../resources/textures/";
            strcat_s(map_path, 256, map_fname);
            current_mtl->map_Kd = malloc(sizeof(sre_texture));
            SRE_Load_texture(map_path, current_mtl->map_Kd);

            break;
        }
        case MAP_SPECULAR:
        {
            char *map_fname = strtok(NULL, " \n");
            char map_path[256] = "../resources/textures/";
            strcat_s(map_path, 256, map_fname);
            current_mtl->map_Ks = malloc(sizeof(sre_texture));
            SRE_Load_texture(map_path, current_mtl->map_Ks);

            break;
        }
        case MAP_SPECULAR_EXP:
        {
            char *map_fname = strtok(NULL, " \n");
            char map_path[256] = "../resources/textures/";
            strcat_s(map_path, 256, map_fname);
            current_mtl->map_Ns = malloc(sizeof(sre_texture));
            SRE_Load_texture(map_path, current_mtl->map_Ns);

            break;
        }
        case MAP_EMISSION:
        {
            char *map_fname = strtok(NULL, " \n");
            char map_path[256] = "../resources/textures/";
            strcat_s(map_path, 256, map_fname);
            current_mtl->map_Ke = malloc(sizeof(sre_texture));
            SRE_Load_texture(map_path, current_mtl->map_Ke);

            break;
        }
        default:
            continue;
            break;
        }
    }
    

    return EXIT_SUCCESS;
}

// int main(int argc, char const *argv[])
// {
//     sre_model model;
//     init_obj_parser();
//     process_obj_file("../models/test.obj", &model);

//     return 0;
// }

