/*
 *
 *    Copyright (c) 2012
 *      maximilian attems <attems@fias.uni-frankfurt.de>
 *
 *    GNU General Public License (GPLv3)
 *
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <list>

#include "include/param-reader.h"
#include "include/Box.h"
#include "include/Laboratory.h"
#include "include/Parameters.h"
#include "include/Sphere.h"
#include "include/outputroutines.h"

/* space separation between items */
const char *sep = " \t\n";

/* process_params - read in all params into list of key value paris */
static void process_params(char *file_path,
  std::list<Parameters> *configuration) {
  char *line = NULL, *saveptr = NULL;
  size_t len = 0;
  ssize_t read;
  char *key, *value;
  FILE *fp;

  /* Looking for parameters in config file
   * If none exists, we'll use default values.
   */
  fp = fopen(file_path, "r");
  if (!fp) {
    fprintf(stderr, "W: No config at %s path.\n", file_path);
    return;
  }

  printf("Processing config %s.\n", file_path);

  while ((read = getline(&line, &len, fp)) != -1) {
    printd("Retrieved %s line of length %li :\n", file_path, read);
    printd("%s", line);
    /* Skip comments and blank lines */
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\t' || line[0] == '/')
      continue;

    key = strtok_r(line, sep, &saveptr);
    if (key == NULL)
      continue;
    value = strtok_r(NULL, sep, &saveptr);
    if (value == NULL)
      continue;

    Parameters parameter(key, value);
    configuration->push_back(parameter);
    printd("%s %s\n", configuration->back().key(),
           configuration->back().value());
  }
  free(line);
  fclose(fp);
  printd("Read all %s.\n", file_path);
}

/* process_config - configuration handling */
void process_config(Parameters, char *path) {
    std::list<Parameters> configuration;
    size_t len = strlen("./config_*.txt") + strlen(path) + 1;
    char *config_path = reinterpret_cast<char *>(malloc(len));
    snprintf(config_path, len, "%s/config*.txt", path);
    process_params(config_path, &configuration);
    warn_wrong_params(&configuration);
    free(config_path);
}
