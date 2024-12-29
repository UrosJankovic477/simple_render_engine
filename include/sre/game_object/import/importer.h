#ifndef SRE_IMPORTER_H
#define SRE_IMPORTER_H

#include <string.h>
#include <stdbool.h>
#include <sre/mem_allocation.h>
#include <sre/logging/errors.h>
#include <sre/hashmap.h>

#define blank_chars " \r\n\t"

//int SRE_Importer_init(char **keywords, sre_keyword *keyword_types, int keywords_count, size_t always_loaded_assets_size, size_t current_zone_assets_size);
int SRE_Importer_init_default();
//int SRE_Get_keyword(const char *token_string, sre_keyword *keyword);
int SRE_Import_asset(const char *filepath);
int SRE_Import_collision(const char *filepath);


#endif
